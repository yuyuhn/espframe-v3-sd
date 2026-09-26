#pragma once

#include "esphome/core/component.h"
#include "esphome/components/remote_image/remote_image.h"
#include "esphome/components/sd_mmc_card/sd_mmc_card.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include <string>
#include <vector>

namespace esphome {
namespace local_photo_frame {

// Cycles through photos found in a local directory (e.g. "/sdcard/PHOTOS")
// and drives a single OnlineImage via "file://" URLs so the existing
// decode/scale/LVGL pipeline renders them. Runs below the SD card (BUS) so the
// FAT mount is ready before the first directory enumeration.
class LocalPhotoFrame : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_image(remote_image::OnlineImage *image) { this->image_ = image; }
  void set_sd_card(sd_mmc_card::SdMmcCard *sd) { this->sd_card_ = sd; }
  void set_photos_dir(const std::string &dir) { this->photos_dir_ = dir; }
  void set_status_sensor(text_sensor::TextSensor *sensor) { this->status_sensor_ = sensor; }

  // Navigation / control, callable from YAML lambdas.
  void next();
  void previous();
  void set_paused(bool paused);
  void toggle_pause() { this->set_paused(!this->paused_); }
  // Re-enumerate the directory (used after a browser upload adds files).
  void rescan();

  size_t photo_count() const { return this->files_.size(); }

 protected:
  void enumerate_();
  void load_current_();
  void publish_status_();

  remote_image::OnlineImage *image_{nullptr};
  sd_mmc_card::SdMmcCard *sd_card_{nullptr};
  std::string photos_dir_{"/sdcard/PHOTOS"};
  std::vector<std::string> files_;
  size_t index_{0};
  bool paused_{false};
  bool started_{false};

  text_sensor::TextSensor *status_sensor_{nullptr};
};

// Trivial text sensor subclass: the parent LocalPhotoFrame publishes its state.
class LocalPhotoFrameStatusTextSensor : public text_sensor::TextSensor {};

}  // namespace local_photo_frame
}  // namespace esphome
