import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from .. import SystaReader, CONF_UART_ID, systa_ns

CONF_PARENT_ID = "systa_reader_id"
CONF_KIND = "kind"

KIND = cv.one_of(
    # COMFORT Status Bits
    "comfort_stat_phk", "comfort_stat_phk2", "comfort_stat_pk",
    "comfort_stat_m1_open", "comfort_stat_m1_close",
    "comfort_stat_m2_open", "comfort_stat_m2_close",
    "comfort_stat_ulv", "comfort_stat_pzi", "comfort_stat_b1",
    "comfort_stat_taster", "comfort_stat_lon", "comfort_stat_ot",
    lower=True
)

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema().extend({
    cv.GenerateID(CONF_PARENT_ID): cv.use_id(SystaReader),
    cv.Required("kind"): KIND,
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    s = await binary_sensor.new_binary_sensor(config)
    
    k = config["kind"]
    if k == "comfort_stat_phk":      cg.add(parent.set_comfort_stat_phk_binary_sensor(s))
    elif k == "comfort_stat_phk2":     cg.add(parent.set_comfort_stat_phk2_binary_sensor(s))
    elif k == "comfort_stat_pk":       cg.add(parent.set_comfort_stat_pk_binary_sensor(s))
    elif k == "comfort_stat_m1_open":  cg.add(parent.set_comfort_stat_m1_open_binary_sensor(s))
    elif k == "comfort_stat_m1_close": cg.add(parent.set_comfort_stat_m1_close_binary_sensor(s))
    elif k == "comfort_stat_m2_open":  cg.add(parent.set_comfort_stat_m2_open_binary_sensor(s))
    elif k == "comfort_stat_m2_close": cg.add(parent.set_comfort_stat_m2_close_binary_sensor(s))
    elif k == "comfort_stat_ulv":      cg.add(parent.set_comfort_stat_ulv_binary_sensor(s))
    elif k == "comfort_stat_pzi":      cg.add(parent.set_comfort_stat_pzi_binary_sensor(s))
    elif k == "comfort_stat_b1":       cg.add(parent.set_comfort_stat_b1_binary_sensor(s))
    elif k == "comfort_stat_taster":   cg.add(parent.set_comfort_stat_taster_binary_sensor(s))
    elif k == "comfort_stat_lon":      cg.add(parent.set_comfort_stat_lon_binary_sensor(s))
    elif k == "comfort_stat_ot":       cg.add(parent.set_comfort_stat_ot_binary_sensor(s))
