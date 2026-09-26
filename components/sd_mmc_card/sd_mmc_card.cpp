#include "sd_mmc_card.h"

#include "esphome/core/log.h"

#include "soc/soc_caps.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#if SOC_SDMMC_IO_POWER_EXTERNAL
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#endif

namespace esphome {
namespace sd_mmc_card {

static const char *const TAG = "sd_mmc_card";

void SdMmcCard::setup() {
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  host.slot = this->slot_ == 1 ? SDMMC_HOST_SLOT_1 : SDMMC_HOST_SLOT_0;
  // Run at SD High Speed (40 MHz). Bring-up negotiated 20 MHz reliably; 40 MHz
  // roughly doubles write throughput for photo uploads. Drop back to
  // SDMMC_FREQ_DEFAULT (20 MHz) if a marginal board shows CRC/timing errors.
  host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;

#if SOC_SDMMC_IO_POWER_EXTERNAL
  // ESP32-P4: the SDMMC I/O power (VDDPST) is supplied by an on-chip LDO.
  // Channel 4 (LDO_VO4) feeds the SD card slot VCC. Without enabling it the
  // card never answers ACMD41, so sdmmc_init_ocr times out (ESP_ERR_TIMEOUT).
  sd_pwr_ctrl_ldo_config_t ldo_config = {
      .ldo_chan_id = this->ldo_channel_,
  };
  sd_pwr_ctrl_handle_t pwr_ctrl_handle = nullptr;
  esp_err_t ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to enable on-chip LDO channel %d for SDMMC I/O power: %s",
             this->ldo_channel_, esp_err_to_name(ret));
    this->publish_status("LDO power error");
    this->mark_failed();
    return;
  }
  host.pwr_ctrl_handle = pwr_ctrl_handle;
#endif

  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  slot_config.width = this->mode_1bit_ ? 1 : 4;
  slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = this->max_files_,
      .allocation_unit_size = 16 * 1024,
      .disk_status_check_enable = false,
  };

  sdmmc_card_t *card = nullptr;
  esp_err_t err = esp_vfs_fat_sdmmc_mount(this->mount_point_.c_str(), &host, &slot_config,
                                          &mount_config, &card);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to mount SD card at %s: %s", this->mount_point_.c_str(),
             esp_err_to_name(err));
    this->publish_status("Mount failed");
    this->mark_failed();
#if SOC_SDMMC_IO_POWER_EXTERNAL
    sd_pwr_ctrl_del_on_chip_ldo(host.pwr_ctrl_handle);
#endif
    return;
  }

  this->mounted_ = true;
  sdmmc_card_print_info(stdout, card);
  ESP_LOGI(TAG, "SD card mounted successfully at %s", this->mount_point_.c_str());
  this->publish_status("Mounted");
}

void SdMmcCard::publish_status(const std::string &status) {
  if (this->status_sensor_ != nullptr) {
    this->status_sensor_->publish_state(status);
  }
}

}  // namespace sd_mmc_card
}  // namespace esphome
