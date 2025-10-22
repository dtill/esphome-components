import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from .. import systa_ns, SystaReader

SystaReaderText = systa_ns.class_("SystaReaderTextSensor", text_sensor.TextSensor, cg.Component)
Kind = systa_ns.enum("Kind")

CONF_PARENT_ID = "systa_reader_id"
CONF_MODE = "mode"
CONF_FILTER = "filter"
CONF_KIND = "kind"

MODE = cv.one_of("raw", "field", lower=True)
FILTER = cv.one_of("all", "aqua", lower=True)
FIELD_KIND = cv.one_of("aqua_status_text", "aqua_timestamp", lower=True)

def field_kind_to_enum(v):
    mapping = {"aqua_status_text":"AQUA_STATUS_TEXT", "aqua_timestamp":"AQUA_TIMESTAMP"}
    return getattr(Kind, mapping[v])

CONFIG_SCHEMA = (
    text_sensor.text_sensor_schema(SystaReaderText)
    .extend({
        cv.Required(CONF_PARENT_ID): cv.use_id(SystaReader),
        cv.Required(CONF_MODE): MODE,
        cv.Optional(CONF_FILTER, default="all"): FILTER,     # nur für mode: raw
        cv.Optional(CONF_KIND): FIELD_KIND,                  # nur für mode: field
    })
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)

    if config[CONF_MODE] == "raw":
        if config[CONF_FILTER] == "aqua":
            cg.add(parent.add_sink_aqua(var))
        else:
            cg.add(parent.add_sink_all(var))
    else:
        k = field_kind_to_enum(config[CONF_KIND])
        cg.add(parent.set_text_sensor(k, var))
