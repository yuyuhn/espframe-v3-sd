import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import CONF_LOCAL_PHOTO_FRAME_ID, LocalPhotoFrame, local_photo_frame_ns

DEPENDENCIES = ["local_photo_frame"]

LocalPhotoFrameStatusTextSensor = local_photo_frame_ns.class_(
    "LocalPhotoFrameStatusTextSensor", text_sensor.TextSensor
)

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend(
    {
        cv.GenerateID(): cv.declare_id(LocalPhotoFrameStatusTextSensor),
        cv.GenerateID(CONF_LOCAL_PHOTO_FRAME_ID): cv.use_id(LocalPhotoFrame),
    }
)


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    parent = await cg.get_variable(config[CONF_LOCAL_PHOTO_FRAME_ID])
    cg.add(parent.set_status_sensor(var))
    return var
