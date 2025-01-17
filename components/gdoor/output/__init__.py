import re
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import output
from esphome.const import CONF_NAME
from .. import DOMAIN, GdoorComponent, gdoor_esphome_ns

CODEOWNERS = ["@dtill"]
DEPENDENCIES = [DOMAIN]

CONF_REQUIRE_RESPONSE = "require_response"
CONF_PAYLOAD = "payload"
HEX_STRING_REGEX = re.compile(r"^[0-9A-Fa-f]+$")  # Regex to validate hex string


# Define the text sensor class for gdoor
GDoorBusWrite = gdoor_esphome_ns.class_("GDoorBusWrite", output.BinaryOutput, cg.Component)

CONFIG_SCHEMA = output.BINARY_OUTPUT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(GDoorBusWrite),
    cv.Required(CONF_NAME): cv.string,
    cv.Required("gdoor_id"): cv.use_id(GdoorComponent),
    cv.Required(CONF_PAYLOAD): cv.string_strict,
    cv.Optional(CONF_REQUIRE_RESPONSE, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA).extend({
    cv.Required(CONF_PAYLOAD): cv.All(
        cv.string_strict,
        cv.matches_regex(HEX_STRING_REGEX, msg="must be a valid hex string")
    ),
})

async def to_code(config):
    parent = await cg.get_variable(config["gdoor_id"])
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await output.register_output(var, config)
    cg.add(var.set_parent(parent))
    cg.add(var.set_payload(config[CONF_PAYLOAD]))
    cg.add(var.set_require_response(config[CONF_REQUIRE_RESPONSE]))