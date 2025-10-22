import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from .. import systa_ns, SystaReader

SystaReaderTextSensor = systa_ns.class_("SystaReaderTextSensor", text_sensor.TextSensor, cg.Component)
CONF_PARENT_ID = "systa_reader_id"

CONFIG_SCHEMA = text_sensor.text_sensor_schema(SystaReaderTextSensor).extend({
    cv.Required(CONF_PARENT_ID): cv.use_id(SystaReader),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = cg.new_Pvariable(config[CONF_ID])  # <-- Wichtig: CONF_ID statt cv.GenerateID()
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
    cg.add(parent.add_sink(var))
