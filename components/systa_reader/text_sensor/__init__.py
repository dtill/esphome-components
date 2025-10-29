import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from .. import systa_ns, SystaReader

SystaReaderText = systa_ns.class_("SystaReaderTextSensor", text_sensor.TextSensor, cg.Component)

CONF_PARENT_ID = "systa_reader_id"
CONF_MODE  = "mode"    # raw | field
CONF_FILTER= "filter"  # all | aqua | aqua_ii #  (nur bei raw)
CONF_KIND  = "kind"    # aqua_status_text | aqua_timestamp (nur bei field)

MODE   = cv.one_of("raw", "field", lower=True)
FILTER = cv.one_of("all", "aqua", "aqua_ii", lower=True)
FIELD_KIND = cv.one_of("aqua_status_text", "aqua_timestamp", "aqua_display_text",
                       "aqua_ii_status_text", "aqua_ii_timestamp",
                       "modula_timestamp",
                       "espresso_timestamp",
                       lower=True)
def _validate(cfg):
    if cfg[CONF_MODE] == "raw":
        if CONF_KIND in cfg:
            raise cv.Invalid("kind not allowed when mode: raw")
    else:
        if CONF_KIND not in cfg:
            raise cv.Invalid("kind required when mode: field")
    return cfg

CONFIG_SCHEMA = cv.All(
    text_sensor.text_sensor_schema(SystaReaderText).extend({
        cv.Required(CONF_PARENT_ID): cv.use_id(SystaReader),
        cv.Required(CONF_MODE): MODE,
        cv.Optional(CONF_FILTER, default="all"): FILTER,
        cv.Optional(CONF_KIND): FIELD_KIND,
    }),
    _validate
)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)

    if config[CONF_MODE] == "raw":
        if config[CONF_FILTER] == "aqua":
            cg.add(parent.add_sink_aqua(var))
        elif config[CONF_FILTER] == "aqua_ii":
            cg.add(parent.add_sink_aqua_ii(var))
        else:
            cg.add(parent.add_sink_all(var))
    else:
        # field mode → mappe auf konkrete Text-Slots
        k = config[CONF_KIND]
        if   k == "aqua_status_text":       cg.add(parent.set_aqua_status_text_sensor(var))
        elif k == "aqua_timestamp":         cg.add(parent.set_aqua_timestamp_text_sensor(var))
        elif k == "aqua_display_text":      cg.add(parent.set_aqua_display_text_sensor(var))
        elif k == "aqua_ii_status_text":    cg.add(parent.set_aqua_ii_status_text_sensor(var))
        elif k == "aqua_ii_timestamp":      cg.add(parent.set_aqua_ii_timestamp_text_sensor(var))
        elif k == "modula_timestamp":       cg.add(parent.set_modula_timestamp_text_sensor(var))
        elif k == "espresso_timestamp":     cg.add(parent.set_espresso_timestamp_text_sensor(var))
