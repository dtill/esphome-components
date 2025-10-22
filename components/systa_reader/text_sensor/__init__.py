import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ICON, CONF_NAME
from .. import systa_ns, SystaReader

SystaReaderTextSensorAll  = systa_ns.class_("SystaReaderTextSensorAll",  text_sensor.TextSensor, cg.Component)
SystaReaderTextSensorAqua = systa_ns.class_("SystaReaderTextSensorAqua", text_sensor.TextSensor, cg.Component)

CONF_PARENT_ID = "systa_reader_id"
CONF_FILTER = "filter"
FILTER = cv.one_of("all", "aqua", lower=True)

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(SystaReaderTextSensorAll)  # Basis-Schema
    .extend(
        {
            cv.Required(CONF_NAME): cv.string,
            cv.Required(CONF_PARENT_ID): cv.use_id(SystaReader),
            cv.Optional(CONF_ICON, default="mdi:code-hex"): cv.icon,
            cv.Optional(CONF_FILTER, default="all"): FILTER,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    flt = config.get(CONF_FILTER, "all")

    if flt == "aqua":
        var = cg.new_Pvariable(config[cv.GenerateID()], SystaReaderTextSensorAqua)
        await cg.register_component(var, config)
        await text_sensor.register_text_sensor(var, config)
        cg.add(parent.add_sink_aqua(var))
    else:
        var = cg.new_Pvariable(config[cv.GenerateID()], SystaReaderTextSensorAll)
        await cg.register_component(var, config)
        await text_sensor.register_text_sensor(var, config)
        cg.add(parent.add_sink_all(var))
