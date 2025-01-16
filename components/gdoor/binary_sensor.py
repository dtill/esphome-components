import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_NAME
from . import DOMAIN, gdoor_esphome_ns

DEPENDENCIES = [DOMAIN]

GdoorBinarySensor = gdoor_esphome_ns.class_("GdoorBinarySensor", binary_sensor.BinarySensor, cg.Component)

PLATFORM_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(GdoorBinarySensor),
    cv.Required(CONF_NAME): cv.string,
    cv.Required("gdoor_id"): cv.use_id(None),
    cv.Optional("payload"): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config["gdoor_id"])
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await binary_sensor.register_binary_sensor(var, config)
    cg.add(var.set_parent(parent))
