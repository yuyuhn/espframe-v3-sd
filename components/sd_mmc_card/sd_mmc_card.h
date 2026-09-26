#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include <string>

namespace esphome {
namespace sd_mmc_card {

// Mounts a FAT-formatted SD card over the ESP32-P4 SDMMC host. On ESP32-P4 the
// SDMMC I/O power (VDDPST) must be enabled first via the on-chip LDO, which
// this component does using sd_pwr_ctrl_new_on_chip_ldo().
class SdMmcCard : public Component {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::BUS; }

  void set_slot(uint8_t slot) { this->slot_ = slot; }
  void set_mode_1bit(bool mode_1bit) { this->mode_1bit_ = mode_1bit; }
  void set_ldo_channel(int ldo_channel) { this->ldo_channel_ = ldo_channel; }
  void set_mount_point(const std::string &mount_point) { this->mount_point_ = mount_point; }
  void set_max_files(int max_files) { this->max_files_ = max_files; }

  void set_status_sensor(text_sensor::TextSensor *sensor) { this->status_sensor_ = sensor; }

  bool is_mounted() const { return this->mounted_; }

 protected:
  void publish_status(const std::string &status);

  uint8_t slot_{0};
  bool mode_1bit_{false};
  int ldo_channel_{4};
  std::string mount_point_{"/sdcard"};
  int max_files_{5};
  bool mounted_{false};

  text_sensor::TextSensor *status_sensor_{nullptr};
};

// Trivial text sensor subclass: the parent SdMmcCard publishes its state.
class SdMmcCardStatusTextSensor : public text_sensor::TextSensor {};

}  // namespace sd_mmc_card
}  // namespace esphome
