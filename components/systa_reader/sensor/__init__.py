import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from .. import systa_ns, SystaReader

SystaReaderClass = SystaReader
Kind = systa_ns.enum("Kind")  # nutzt das C++-Enum

CONF_PARENT_ID = "systa_reader_id"
CONF_KIND = "kind"

KIND = cv.one_of(
    "tsa", "tse", "twu", "tw2", "sol", "tag", "gesamt", "status_code",
    lower=True
)

# Map Python -> C++ Enum
def kind_to_enum(v):
    mapping = {
        "tsa": "TSA", "tse": "TSE", "twu": "TWU", "tw2": "TW2",
        "sol": "SOL", "tag": "TAG", "gesamt": "GESAMT", "status_code": "STATUS_CODE",
    }
    return getattr(Kind, mapping[v])

CONFIG_SCHEMA = sensor.sensor_schema().extend({
    cv.Required(CONF_PARENT_ID): cv.use_id(SystaReaderClass),
    cv.Required(CONF_KIND): KIND,
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    var = await sensor.new_sensor(config)

    k = kind_to_enum(config[CONF_KIND])
    cg.add(parent.set_numeric_sensor(k, var))
