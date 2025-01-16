import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_NAME
from .. import DOMAIN, gdoor_esphome_ns

DEPENDENCIES = [DOMAIN]

# Define the text sensor class for gdoor
GdoorTextSensor = gdoor_esphome_ns.class_("GdoorTextSensor", text_sensor.TextSensor, cg.Component)

PLATFORM_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(GdoorTextSensor),
    cv.Required(CONF_NAME): cv.string,
    cv.Required("gdoor_id"): cv.use_id(None),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config["gdoor_id"])
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await text_sensor.register_text_sensor(var, config)
    cg.add(var.set_parent(parent))
