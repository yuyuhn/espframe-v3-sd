"""ESPHome component that adds a browser upload endpoint for local photos.

Registers two routes on the shared web server (web_server_base):
  - POST /upload  stream a raw image body (X-Filename header) to the SD card
  - GET  /local   serve a minimal HTML form that uploads selected images

After a successful upload it tells the local_photo_frame to rescan the directory.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import local_photo_frame
from esphome.const import CONF_ID

DEPENDENCIES = ["web_server_base", "local_photo_frame"]

local_upload_ns = cg.esphome_ns.namespace("local_upload")
LocalUpload = local_upload_ns.class_("LocalUpload", cg.Component)

CONF_LOCAL_UPLOAD_ID = "local_upload_id"
CONF_LOCAL_PHOTO_FRAME_ID = "local_photo_frame_id"
CONF_PHOTOS_DIR = "photos_dir"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LocalUpload),
        cv.GenerateID(CONF_LOCAL_PHOTO_FRAME_ID): cv.use_id(local_photo_frame.LocalPhotoFrame),
        cv.Optional(CONF_PHOTOS_DIR, default="/sdcard/PHOTOS"): cv.string,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    frame = await cg.get_variable(config[CONF_LOCAL_PHOTO_FRAME_ID])
    cg.add(var.set_frame(frame))

    cg.add(var.set_photos_dir(config[CONF_PHOTOS_DIR]))
