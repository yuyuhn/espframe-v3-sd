#include "local_upload.h"

#include "esphome/core/log.h"

#include <cstring>
#include <strings.h>
#include <sys/stat.h>
#include <errno.h>

namespace esphome {
namespace local_upload {

static const char *const TAG = "local_upload";

// The web server streams the body in ~1460-byte chunks; the default FILE buffer
// is only ~1KB, so flushing that often forces the SD card into tiny
// read-modify-write cycles that make large uploads crawl. A 64KB buffer turns
// ~18k small writes into ~400 full-cluster writes.
static constexpr size_t WRITE_BUFFER_SIZE = 64 * 1024;

static const char *const UPLOAD_PAGE = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Upload photos</title>
<style>
  :root { color-scheme: dark; }
  body { font-family: system-ui, sans-serif; background: #121212; color: #eee;
         max-width: 560px; margin: 40px auto; padding: 0 20px; }
  h1 { font-size: 1.4em; font-weight: 600; }
  p { color: #999; line-height: 1.5; }
  code { color: #ffd166; }
  input[type=file] { display: block; margin: 16px 0; color: #999; }
  button { background: #ffd166; color: #1a1a1a; border: 0; padding: 12px 24px;
           border-radius: 8px; font-size: 16px; cursor: pointer; }
  #status { margin-top: 16px; min-height: 1.3em; }
  #status.ok { color: #7ee787; }
  #status.err { color: #ff7b72; }
</style>
</head>
<body>
<h1>Upload photos to the SD card</h1>
<p>Select one or more images. They are saved to <code>/sdcard/PHOTOS</code> and
appear in the local photo slideshow.</p>
<input type="file" id="file" multiple accept=".jpg,.jpeg,.png,.webp,.bmp">
<button onclick="upload()">Upload</button>
<div id="status"></div>
<script>
async function upload() {
  const input = document.getElementById('file');
  const status = document.getElementById('status');
  const files = Array.from(input.files || []);
  if (!files.length) { status.className = 'err'; status.textContent = 'Choose at least one file.'; return; }
  status.className = '';
  status.textContent = 'Uploading...';
  for (const file of files) {
    try {
      const res = await fetch('/upload', { method: 'POST', body: file, headers: { 'X-Filename': file.name, 'Content-Type': 'application/octet-stream' } });
      if (!res.ok) throw new Error('HTTP ' + res.status);
      status.textContent = 'Uploaded: ' + file.name;
    } catch (e) {
      status.className = 'err';
      status.textContent = 'Failed: ' + file.name + ' (' + e.message + ')';
      return;
    }
  }
  status.className = 'ok';
  status.textContent = 'Done - ' + files.length + ' file(s) uploaded.';
  input.value = '';
}
</script>
</body>
</html>
)HTML";

static const char *const IMAGE_EXTENSIONS[] = {".jpg", ".jpeg", ".png", ".webp", ".bmp"};

static bool has_image_extension(const char *name) {
  const char *dot = strrchr(name, '.');
  if (dot == nullptr)
    return false;
  for (const char *ext : IMAGE_EXTENSIONS) {
    if (strcasecmp(dot, ext) == 0)
      return true;
  }
  return false;
}

std::string LocalUploadHandler::sanitize_filename_(const std::string &name) {
  // Keep only the basename so a crafted X-Filename header cannot escape the
  // photos directory (reject '/', '\\' and ".."), and require an image
  // extension so only decodable files land on the card.
  size_t slash = name.find_last_of("/\\");
  std::string base = (slash == std::string::npos) ? name : name.substr(slash + 1);
  if (base.empty() || base.size() > 128)
    return "";
  if (base.find("..") != std::string::npos)
    return "";
  if (!has_image_extension(base.c_str()))
    return "";
  return base;
}

bool LocalUploadHandler::canHandle(AsyncWebServerRequest *request) const {
  char path[AsyncWebServerRequest::URL_BUF_SIZE];
  const StringRef url = request->url_to(path);
  if (url == "/local")
    return request->method() == HTTP_GET;
  if (url == "/upload")
    return request->method() == HTTP_POST;
  return false;
}

void LocalUploadHandler::handleRequest(AsyncWebServerRequest *request) {
  char path[AsyncWebServerRequest::URL_BUF_SIZE];
  const StringRef url = request->url_to(path);

  if (request->method() == HTTP_GET && url == "/local") {
    request->send(200, "text/html", UPLOAD_PAGE);
    return;
  }

  if (request->method() == HTTP_POST && url == "/upload") {
    auto it = this->uploads_.find(request);
    const bool saved = it != this->uploads_.end() && it->second.file != nullptr;
    if (saved) {
      std::string name = it->second.name;
      fclose(it->second.file);
      this->uploads_.erase(it);
      ESP_LOGI(TAG, "Saved %s", name.c_str());
      std::string body = "OK: " + name;
      request->send(200, "text/plain", body.c_str());
      if (this->frame_ != nullptr)
        this->frame_->rescan();
    } else {
      if (it != this->uploads_.end())
        this->uploads_.erase(it);
      request->send(400, "text/plain", "Upload failed: invalid filename or SD card not writable");
    }
    return;
  }

  request->send(404, "text/plain", "Not found");
}

void LocalUploadHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index,
                                    size_t total) {
  if (request->method() != HTTP_POST)
    return;
  char path[AsyncWebServerRequest::URL_BUF_SIZE];
  if (!(request->url_to(path) == "/upload"))
    return;

  UploadState &state = this->uploads_[request];
  if (index == 0) {
    // First chunk: resolve and open the destination file.
    std::string raw_name = request->get_header("X-Filename").value_or("");
    std::string name = sanitize_filename_(raw_name);
    if (name.empty()) {
      ESP_LOGW(TAG, "Rejected upload: invalid filename '%s'", raw_name.c_str());
      return;  // state.file stays null; handleRequest() reports the error
    }
    state.name = name;
    std::string full_path = this->photos_dir_;
    if (!full_path.empty() && full_path.back() != '/')
      full_path += '/';
    full_path += name;
    // A freshly formatted SD card has no photos directory yet — create it first.
    int rc = mkdir(this->photos_dir_.c_str(), 0755);
    if (rc != 0 && errno != EEXIST) {
      ESP_LOGW(TAG, "mkdir %s failed: errno=%d", this->photos_dir_.c_str(), errno);
    }
    state.file = fopen(full_path.c_str(), "wb");
    if (state.file == nullptr) {
      ESP_LOGE(TAG, "Failed to open %s for writing: errno=%d", full_path.c_str(), errno);
      state.name.clear();
      return;
    }
    // Batch writes into big SD-card writes instead of flushing ~1KB at a time.
    state.buffer.resize(WRITE_BUFFER_SIZE);
    if (state.buffer.size() >= BUFSIZ)
      setvbuf(state.file, reinterpret_cast<char *>(state.buffer.data()), _IOFBF, state.buffer.size());
    ESP_LOGI(TAG, "Receiving upload %s (%u bytes)", name.c_str(), (unsigned) total);
  }
  if (state.file != nullptr && len > 0)
    fwrite(data, 1, len, state.file);
}

void LocalUpload::setup() {
  auto *base = web_server_base::global_web_server_base;
  if (base == nullptr) {
    ESP_LOGW(TAG, "web_server_base unavailable; upload endpoints disabled");
    return;
  }
  base->add_handler(&this->handler_);
}

}  // namespace local_upload
}  // namespace esphome
