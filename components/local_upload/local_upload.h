#pragma once

#include "esphome/core/component.h"
#include "esphome/components/web_server_base/web_server_base.h"
#include "esphome/components/local_photo_frame/local_photo_frame.h"

#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace esphome {
namespace local_upload {

// Streams raw-binary photo uploads (POST /upload with an X-Filename header) to
// the local photo directory, and serves a small HTML form on GET /local.
//
// Uses AsyncWebHandler::handleBody() instead of multipart parsing so an image
// body is written straight to the SD card without buffering it in RAM. State is
// kept per-request so two in-flight uploads cannot clobber each other's file.
class LocalUploadHandler : public AsyncWebHandler {
 public:
  void set_photos_dir(const std::string &dir) { this->photos_dir_ = dir; }
  void set_frame(local_photo_frame::LocalPhotoFrame *frame) { this->frame_ = frame; }

  bool canHandle(AsyncWebServerRequest *request) const override;
  void handleRequest(AsyncWebServerRequest *request) override;
  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) override;

 protected:
  static std::string sanitize_filename_(const std::string &name);

  struct UploadState {
    FILE *file{nullptr};
    std::string name;
    std::vector<uint8_t> buffer;  // large write buffer, lives until fclose()
  };

  std::string photos_dir_{"/sdcard/PHOTOS"};
  local_photo_frame::LocalPhotoFrame *frame_{nullptr};
  std::map<AsyncWebServerRequest *, UploadState> uploads_;
};

// Registers LocalUploadHandler on the shared web server (web_server_base) so the
// endpoints coexist with ESPHome's web_server. Runs at BUS priority so the
// handler is queued before the server begins serving.
class LocalUpload : public Component {
 public:
  void setup() override;
  float get_setup_priority() const override { return 1000.0f; }

  void set_photos_dir(const std::string &dir) { this->handler_.set_photos_dir(dir); }
  void set_frame(local_photo_frame::LocalPhotoFrame *frame) { this->handler_.set_frame(frame); }

 protected:
  LocalUploadHandler handler_{};
};

}  // namespace local_upload
}  // namespace esphome
