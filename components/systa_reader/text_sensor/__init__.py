import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from .. import systa_ns, SystaReader

SystaReaderText = systa_ns.class_("SystaReaderTextSensor", text_sensor.TextSensor, cg.Component)

CONF_PARENT_ID = "systa_reader_id"
CONF_MODE  = "mode"     # raw | field
CONF_FILTER= "filter"   # "all" | device | [devices...]
CONF_KIND  = "kind"     # field-only

MODE = cv.one_of("raw", "field", lower=True)

# allow single value or list of values
_FILTER_ONE = cv.one_of("all", "aqua", "aqua_ii", "modula", "espresso", "palletti_ii", "compact", "comfort", lower=True)
FILTER = cv.Any(_FILTER_ONE, cv.All(cv.ensure_list(_FILTER_ONE), cv.Length(min=1)))

FIELD_KIND = cv.one_of(
    "aqua_status_text", "aqua_timestamp", "aqua_display_text", "aqua_fw_version",
    "aqua_ii_status_text", "aqua_ii_timestamp",
    "modula_timestamp",
    "espresso_timestamp",
    "palletti_ii_timestamp", "palletti_ii_display_text",
    "compact_timestamp", "comfort_boiler_err", "comfort_fw_version",
    lower=True
)

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
        f = config[CONF_FILTER]
        if f == "aqua":
            cg.add(parent.add_sink_aqua(var))
        elif f == "aqua_ii":
            cg.add(parent.add_sink_aqua_ii(var))
        elif f == "all":
            cg.add(parent.add_sink_all(var))
        else:
            # Fallback, sollte nie erreicht werden
            cg.add(parent.add_sink_all(var))
    else:
        # field mode → konkrete Text-Slots
        k = config[CONF_KIND]
        if k == "aqua_status_text":
            cg.add(parent.set_aqua_status_text_sensor(var))
        elif k == "aqua_timestamp":
            cg.add(parent.set_aqua_timestamp_text_sensor(var))
        elif k == "aqua_display_text":
            cg.add(parent.set_aqua_display_text_sensor(var))
        elif k == "aqua_fw_version":
            cg.add(parent.set_aqua_fw_version_text_sensor(var))
        elif k == "aqua_ii_status_text":
            cg.add(parent.set_aqua_ii_status_text_sensor(var))
        elif k == "aqua_ii_timestamp":
            cg.add(parent.set_aqua_ii_timestamp_text_sensor(var))
        elif k == "modula_timestamp":
            cg.add(parent.set_modula_timestamp_text_sensor(var))
        elif k == "espresso_timestamp":
            cg.add(parent.set_espresso_timestamp_text_sensor(var))
        elif k == "palletti_ii_timestamp":
            cg.add(parent.set_palletti_ii_timestamp_text_sensor(var))
        elif k == "palletti_ii_display_text":
            cg.add(parent.set_palletti_ii_display_text_sensor(var))
        elif k == "compact_timestamp":
            cg.add(parent.set_compact_timestamp_text_sensor(var))
        elif k == "comfort_boiler_err":
            cg.add(parent.set_comfort_boiler_err_text_sensor(var))
        elif k == "comfort_fw_version":
            cg.add(parent.set_comfort_fw_version_text_sensor(var))

