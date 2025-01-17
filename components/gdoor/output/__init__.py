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


def validate_hex_string(value):
    """Validate that the string is a valid hexadecimal."""
    if not HEX_STRING_REGEX.match(value):
        raise cv.Invalid("Payload must be a valid hex string (e.g., '011041A286FD0000A18FA7')")
    return value

def validate_crc(value):
    """Validate the CRC checksum of the hex string."""
    def calculate_crc(data: str) -> str:
        checksum = 0
        for i in range(0, len(data) - 2, 2):    # Exclude the last 2 characters (assumed checksum)
            checksum ^= int(data[i:i+2], 16)
        return f"{checksum:02X}"
    provided_crc = value[-2:]                   # Assume last 2 characters are the CRC
    data_without_crc = value[:-2]
    expected_crc = calculate_crc(data_without_crc)
    if provided_crc != expected_crc:
        cg.esphome_ns.global_log().warn(
            f"Payload CRC mismatch: expected {expected_crc}, got {provided_crc} (Payload: {value})"
        )
    return value

CONFIG_SCHEMA = output.BINARY_OUTPUT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(GDoorBusWrite),
    cv.Required(CONF_NAME): cv.string,
    cv.Required("gdoor_id"): cv.use_id(GdoorComponent),
    cv.Required(CONF_PAYLOAD): cv.All(
        cv.string,              # Ensure it's a string
        validate_hex_string,    # Validate hex format
        validate_crc            # Validate CRC and log warning
    ),
    cv.Optional(CONF_REQUIRE_RESPONSE, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config["gdoor_id"])
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await output.register_output(var, config)
    cg.add(var.set_parent(parent))
    cg.add(var.set_payload(config[CONF_PAYLOAD]))
    cg.add(var.set_require_response(config[CONF_REQUIRE_RESPONSE]))