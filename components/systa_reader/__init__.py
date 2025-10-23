import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

AUTO_LOAD = ["sensor", "text_sensor"]
CODEOWNERS = ["@dtill"]
DEPENDENCIES = ["uart", "sensor", "text_sensor"]
MULTI_CONF = True  # allow multiple systa_reader blocks, but we'll enforce 1 per UART

systa_ns = cg.esphome_ns.namespace("systa_reader")
SystaReader = systa_ns.class_("SystaReader", uart.UARTDevice, cg.Component)

# ----- devices (list)
SYSTA_DEVICE = cv.one_of("aqua", "modula", "espresso", "solar", lower=True)
SYSTA_DEVICES_LIST = cv.All(cv.ensure_list(SYSTA_DEVICE), cv.Length(min=1))

# bit flags (room to grow—switch to 64-bit in C++ if you like)
DEV_FLAGS = {
    "aqua":     1 << 0,
    "modula":   1 << 1,
    "espresso": 1 << 2,
    "solar":    1 << 3,
}

CONF_UART_ID = "uart_id"
CONF_LOG_INVALID = "log_invalid"
CONF_DEVICES = "systa_device"  # list, e.g. systa_device: [aqua, espresso]

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SystaReader),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_LOG_INVALID, default=True): cv.boolean,
    cv.Required(CONF_DEVICES): SYSTA_DEVICES_LIST,
}).extend(cv.COMPONENT_SCHEMA)

# keep a module-level set of claimed UART ids to forbid duplicates
_uart_claims = set()

def _devices_to_mask(names) -> int:
    mask = 0
    for n in names:
        mask |= DEV_FLAGS[n]
    return mask

async def to_code(config):
    # enforce one systa_reader per UART
    uart_id_obj = config[CONF_UART_ID]
    uart_key = str(uart_id_obj.id)
    if uart_key in _uart_claims:
        raise cv.Invalid(
            f"Only one systa_reader is allowed per UART. The UART '{uart_key}' "
            f"is already used by another systa_reader."
        )
    _uart_claims.add(uart_key)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_log_invalid(config[CONF_LOG_INVALID]))

    mask = _devices_to_mask(config[CONF_DEVICES])
    cg.add(var.set_enabled_mask(mask))
