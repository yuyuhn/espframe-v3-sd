"""ESPHome component that cycles through local photos on an SD card.

Enumerates a directory (e.g. "/sdcard/PHOTOS") for image files and drives a
single OnlineImage via "file://" URLs, reusing the remote_image decode/scale/
LVGL pipeline. Runs below the SD card (BUS priority) so the FAT mount is ready
before the first enumeration.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import remote_image
from esphome.components.sd_mmc_card import SdMmcCard, CONF_SD_MMC_CARD_ID
from esphome.const import CONF_ID

DEPENDENCIES = ["sd_mmc_card", "remote_image"]

CODEOWNERS = ["@clydebarrow"]

local_photo_frame_ns = cg.esphome_ns.namespace("local_photo_frame")
LocalPhotoFrame = local_photo_frame_ns.class_("LocalPhotoFrame", cg.PollingComponent)

CONF_LOCAL_PHOTO_FRAME_ID = "local_photo_frame_id"
CONF_IMAGE_ID = "image_id"
CONF_PHOTOS_DIR = "photos_dir"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LocalPhotoFrame),
        cv.GenerateID(CONF_IMAGE_ID): cv.use_id(remote_image.OnlineImage),
        cv.GenerateID(CONF_SD_MMC_CARD_ID): cv.use_id(SdMmcCard),
        cv.Optional(CONF_PHOTOS_DIR, default="/sdcard/PHOTOS"): cv.string,
    }
).extend(cv.polling_component_schema("15s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    image = await cg.get_variable(config[CONF_IMAGE_ID])
    cg.add(var.set_image(image))

    sd_card = await cg.get_variable(config[CONF_SD_MMC_CARD_ID])
    cg.add(var.set_sd_card(sd_card))

    cg.add(var.set_photos_dir(config[CONF_PHOTOS_DIR]))
