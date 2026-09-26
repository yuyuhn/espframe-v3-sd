"""ESPHome component that mounts an SD card over the ESP32-P4 SDMMC host.

The upstream sd_mmc_card does not support ESP32-P4, and the youkorr fork does
not power the SDMMC I/O domain. On ESP32-P4 the SD card slot VCC is fed from
the on-chip LDO channel 4 (LDO_VO4, net "VDDPST"), so this component enables
that LDO via sd_pwr_ctrl_new_on_chip_ldo() before mounting FAT.

Slot 0 uses the IO_MUX-fixed pins (CLK=43 CMD=44 D0-3=39-42), which is what
the SD card socket on the Guition JC8012P4A1 is wired to; slot 1 is reserved
for the ESP32-C6 WiFi/BT over SDIO.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@clydebarrow"]

sd_mmc_card_ns = cg.esphome_ns.namespace("sd_mmc_card")
SdMmcCard = sd_mmc_card_ns.class_("SdMmcCard", cg.Component)

CONF_SLOT = "slot"
CONF_MODE_1BIT = "mode_1bit"
CONF_LDO_CHANNEL = "ldo_channel"
CONF_MOUNT_POINT = "mount_point"
CONF_MAX_FILES = "max_files"
CONF_SD_MMC_CARD_ID = "sd_mmc_card_id"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SdMmcCard),
        cv.Optional(CONF_SLOT, default=0): cv.int_range(0, 1),
        cv.Optional(CONF_MODE_1BIT, default=False): cv.boolean,
        cv.Optional(CONF_LDO_CHANNEL, default=4): cv.int_range(0, 7),
        cv.Optional(CONF_MOUNT_POINT, default="/sdcard"): cv.string,
        cv.Optional(CONF_MAX_FILES, default=5): cv.int_range(1, 16),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_slot(config[CONF_SLOT]))
    cg.add(var.set_mode_1bit(config[CONF_MODE_1BIT]))
    cg.add(var.set_ldo_channel(config[CONF_LDO_CHANNEL]))
    cg.add(var.set_mount_point(config[CONF_MOUNT_POINT]))
    cg.add(var.set_max_files(config[CONF_MAX_FILES]))
