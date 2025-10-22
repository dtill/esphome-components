import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ICON, CONF_NAME
from .. import systa_ns, SystaReader


# Text sensor that receives raw HEX frames from the reader
SystaReaderTextSensor = systa_ns.class_(
"SystaReaderTextSensor", text_sensor.TextSensor, cg.Component
)


CONF_PARENT_ID = "systa_reader_id"


CONFIG_SCHEMA = (
text_sensor.text_sensor_schema(SystaReaderTextSensor)
.extend(
{
cv.Required(CONF_NAME): cv.string,
cv.Required(CONF_PARENT_ID): cv.use_id(SystaReader),
cv.Optional(CONF_ICON, default="mdi:code-hex"): cv.icon,
}
)
.extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
parent = await cg.get_variable(config[CONF_PARENT_ID])
var = cg.new_Pvariable(config[cv.GenerateID()])
await cg.register_component(var, config)
await text_sensor.register_text_sensor(var, config)
cg.add(parent.add_sink(var))