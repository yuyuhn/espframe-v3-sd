import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor

from . import CONF_SD_MMC_CARD_ID, SdMmcCard, sd_mmc_card_ns

DEPENDENCIES = ["sd_mmc_card"]

SdMmcCardStatusTextSensor = sd_mmc_card_ns.class_(
    "SdMmcCardStatusTextSensor", text_sensor.TextSensor
)

CONFIG_SCHEMA = text_sensor.text_sensor_schema().extend(
    {
        cv.GenerateID(): cv.declare_id(SdMmcCardStatusTextSensor),
        cv.GenerateID(CONF_SD_MMC_CARD_ID): cv.use_id(SdMmcCard),
    }
)


async def to_code(config):
    var = await text_sensor.new_text_sensor(config)
    parent = await cg.get_variable(config[CONF_SD_MMC_CARD_ID])
    cg.add(parent.set_status_sensor(var))
    return var
