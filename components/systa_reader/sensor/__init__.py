import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from .. import systa_ns, SystaReader

Kind = systa_ns.enum("Kind")

CONF_PARENT_ID = "systa_reader_id"
CONF_KIND = "kind"

# nur gültige Kinds anbieten (Device-präfix!)
KIND = cv.one_of(
    # AQUA
    "aqua_tsa", "aqua_tse", "aqua_twu", "aqua_tw2",
    "aqua_sol", "aqua_tag", "aqua_gesamt", "aqua_status_code",
    lower=True
)

def kind_to_enum(v):
    mapping = {
        "aqua_tsa":"AQUA_TSA", "aqua_tse":"AQUA_TSE", "aqua_twu":"AQUA_TWU", "aqua_tw2":"AQUA_TW2",
        "aqua_sol":"AQUA_SOL", "aqua_tag":"AQUA_TAG", "aqua_gesamt":"AQUA_GESAMT", "aqua_status_code":"AQUA_STATUS_CODE",
    }
    return getattr(Kind, mapping[v])

CONFIG_SCHEMA = sensor.sensor_schema().extend({
    cv.Required(CONF_PARENT_ID): cv.use_id(SystaReader),
    cv.Required(CONF_KIND): KIND,
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = await sensor.new_sensor(config)
    k = kind_to_enum(config[CONF_KIND])
    cg.add(parent.set_numeric_sensor(k, var))
