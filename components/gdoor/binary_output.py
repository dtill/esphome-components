import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_NAME
from . import DOMAIN, gdoor_esphome_ns

DEPENDENCIES = [DOMAIN]

GdoorOutput = gdoor_esphome_ns.class_("GdoorOutput", output.BinaryOutput, cg.Component)

PLATFORM_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(GdoorOutput),
    cv.Required(CONF_NAME): cv.string,
    cv.Required("gdoor_id"): cv.use_id(None),
    cv.Optional("payload"): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config["gdoor_id"])
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await output.register_output(var, config)
    cg.add(var.set_parent(parent))
