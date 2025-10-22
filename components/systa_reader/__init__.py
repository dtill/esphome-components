import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID


AUTO_LOAD = ["text_sensor"]
CODEOWNERS = ["@dtill"]
DEPENDENCIES = ["uart"]


systa_ns = cg.esphome_ns.namespace("systa_reader")
SystaReader = systa_ns.class_("SystaReader", uart.UARTDevice, cg.Component)


CONF_UART_ID = "uart_id"
CONF_LOG_INVALID = "log_invalid"


CONFIG_SCHEMA = cv.Schema({
cv.GenerateID(): cv.declare_id(SystaReader),
cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
cv.Optional(CONF_LOG_INVALID, default=True): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
var = cg.new_Pvariable(config[CONF_ID])
await cg.register_component(var, config)
# Properly register as UART device (no set_parent call)
await uart.register_uart_device(var, config)
cg.add(var.set_log_invalid(config[CONF_LOG_INVALID]))