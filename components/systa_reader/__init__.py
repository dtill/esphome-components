import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

AUTO_LOAD = ["sensor", "text_sensor"]
CODEOWNERS = ["@dtill"]
DEPENDENCIES = ["uart", "sensor", "text_sensor"]

systa_ns = cg.esphome_ns.namespace("systa_reader")
SystaReader = systa_ns.class_("SystaReader", uart.UARTDevice, cg.Component)

SYSTA_DEVICE = cv.one_of("aqua", "modula", "espresso", "solar", lower=True)
SYSTA_DEVICES_LIST = cv.All(cv.ensure_list(SYSTA_DEVICE), cv.Length(min=1))

# ---- device flags for bitmasking
DEV_FLAGS = {"aqua": 1, "modula": 2, "espresso": 4, "solar": 8}

CONF_UART_ID = "uart_id"
CONF_LOG_INVALID = "log_invalid"
CONF_DEVICES = "systa_devices"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SystaReader),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_LOG_INVALID, default=True): cv.boolean,
    cv.Required(CONF_DEVICES): SYSTA_DEVICES_LIST,
}).extend(cv.COMPONENT_SCHEMA)

# build mask
def _devices_to_mask(names) -> int:
    mask = 0
    for n in names:
        mask |= DEV_FLAGS[n]   # extend DEV_FLAGS as you add devices; use next power of two
    return mask

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_log_invalid(config[CONF_LOG_INVALID]))
    # enable multiple decoders
    names = config[CONF_DEVICES]
    mask = _devices_to_mask(names)
    cg.add(var.set_enabled_mask(mask))
