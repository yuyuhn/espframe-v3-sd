#include "local_photo_frame.h"

#include "esphome/core/log.h"

#include <dirent.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <strings.h>
#include <sys/stat.h>
#include <errno.h>

namespace esphome {
namespace local_photo_frame {

static const char *const TAG = "local_photo_frame";

static const char *const PHOTO_EXTENSIONS[] = {".jpg", ".jpeg", ".png", ".webp", ".bmp"};

static bool has_photo_extension(const char *name) {
  const char *dot = strrchr(name, '.');
  if (dot == nullptr)
    return false;
  for (const char *ext : PHOTO_EXTENSIONS) {
    if (strcasecmp(dot, ext) == 0)
      return true;
  }
  return false;
}

void LocalPhotoFrame::setup() {
  // A freshly formatted card has no photos directory yet — create it so the
  // upload endpoint and the enumeration below have somewhere to write/read.
  int rc = mkdir(this->photos_dir_.c_str(), 0755);
  if (rc != 0 && errno != EEXIST) {
    ESP_LOGW(TAG, "mkdir %s failed: errno=%d", this->photos_dir_.c_str(), errno);
  }
  // Enumerate once so the status sensor reports the photo count, but do not
  // auto-load: local mode stays idle (paused) until enabled via the switch, so
  // it never fights the Immich screen for display.
  this->enumerate_();
}

void LocalPhotoFrame::update() {
  if (this->paused_)
    return;
  if (!this->started_) {
    this->rescan();
    return;
  }
  // Don't skip ahead while the current photo is still decoding.
  if (this->image_ != nullptr && this->image_->is_downloading())
    return;
  this->next();
}

void LocalPhotoFrame::next() {
  if (this->files_.empty()) {
    this->enumerate_();
    if (this->files_.empty()) {
      this->publish_status_();
      return;
    }
    this->index_ = 0;
  } else {
    this->index_ = (this->index_ + 1) % this->files_.size();
  }
  this->load_current_();
}

void LocalPhotoFrame::previous() {
  if (this->files_.empty()) {
    this->enumerate_();
    if (this->files_.empty()) {
      this->publish_status_();
      return;
    }
    this->index_ = 0;
  } else {
    this->index_ = (this->index_ + this->files_.size() - 1) % this->files_.size();
  }
  this->load_current_();
}

void LocalPhotoFrame::rescan() {
  this->enumerate_();
  if (this->files_.empty()) {
    this->started_ = false;
    this->publish_status_();
    return;
  }
  if (this->paused_) {
    // Local mode is off: refresh the count only and leave the display alone.
    this->publish_status_();
    return;
  }
  this->index_ = 0;
  this->load_current_();
  this->started_ = true;
}

void LocalPhotoFrame::set_paused(bool paused) {
  if (this->paused_ == paused)
    return;
  this->paused_ = paused;
  if (!paused && !this->started_) {
    // First time the local mode is enabled: load the first photo now.
    this->rescan();
  }
  ESP_LOGI(TAG, "Slideshow %s", paused ? "paused" : "resumed");
}

void LocalPhotoFrame::load_current_() {
  if (this->files_.empty() || this->index_ >= this->files_.size())
    return;
  const std::string &path = this->files_[this->index_];
  if (this->image_ != nullptr) {
    this->image_->set_url("file://" + path);
    this->image_->update();
  }
  ESP_LOGI(TAG, "Showing photo %u/%u: %s", (unsigned) (this->index_ + 1),
           (unsigned) this->files_.size(), path.c_str());
  this->publish_status_();
}

void LocalPhotoFrame::enumerate_() {
  this->files_.clear();
  DIR *dir = opendir(this->photos_dir_.c_str());
  if (dir == nullptr) {
    ESP_LOGD(TAG, "Cannot open photos directory %s", this->photos_dir_.c_str());
    this->publish_status_();
    return;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (entry->d_type == DT_DIR)
      continue;
    if (!has_photo_extension(entry->d_name))
      continue;
    std::string path = this->photos_dir_;
    if (!path.empty() && path.back() != '/')
      path += '/';
    path += entry->d_name;
    this->files_.push_back(std::move(path));
  }
  closedir(dir);
  std::sort(this->files_.begin(), this->files_.end());
  ESP_LOGI(TAG, "Found %u photos in %s", (unsigned) this->files_.size(),
           this->photos_dir_.c_str());
  this->publish_status_();
}

void LocalPhotoFrame::publish_status_() {
  if (this->status_sensor_ == nullptr)
    return;
  if (this->files_.empty()) {
    this->status_sensor_->publish_state("No photos");
    return;
  }
  char buf[64];
  snprintf(buf, sizeof(buf), "%u/%u", (unsigned) (this->index_ + 1),
           (unsigned) this->files_.size());
  this->status_sensor_->publish_state(buf);
}

}  // namespace local_photo_frame
}  // namespace esphome
