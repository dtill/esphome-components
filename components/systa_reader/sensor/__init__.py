import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from .. import systa_ns, SystaReader

CONF_PARENT_ID = "systa_reader_id"
CONF_KIND = "kind"

KIND = cv.one_of(
    # AQUA
    "aqua_tsa","aqua_tse","aqua_twu","aqua_tw2","aqua_sol","aqua_tag","aqua_gesamt","aqua_status_code",
    # MODULA
    "modula_two","modula_tbv","modula_tbr","modula_tv","modula_tv2",
    "modula_tr","modula_tr2","modula_tpo","modula_tpu","modula_tzr",
    # ESPRESSO
    "espresso_t01","espresso_t02","espresso_t03","espresso_t04","espresso_t05",
    "espresso_pk","espresso_t06","espresso_t07","espresso_t08","espresso_t09",
    "espresso_t10","espresso_t11","espresso_t12",
    lower=True
)

CONFIG_SCHEMA = sensor.sensor_schema().extend({
    cv.Required(CONF_PARENT_ID): cv.use_id(SystaReader),
    cv.Required(CONF_KIND): KIND,
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_PARENT_ID])
    s = await sensor.new_sensor(config)
    k = config[CONF_KIND]
    if   k == "aqua_tsa":           cg.add(parent.set_aqua_tsa_sensor(s))
    elif k == "aqua_tse":           cg.add(parent.set_aqua_tse_sensor(s))
    elif k == "aqua_twu":           cg.add(parent.set_aqua_twu_sensor(s))
    elif k == "aqua_tw2":           cg.add(parent.set_aqua_tw2_sensor(s))
    elif k == "aqua_sol":           cg.add(parent.set_aqua_sol_sensor(s))
    elif k == "aqua_tag":           cg.add(parent.set_aqua_tag_sensor(s))
    elif k == "aqua_gesamt":        cg.add(parent.set_aqua_ges_sensor(s))
    elif k == "aqua_status_code":   cg.add(parent.set_aqua_status_code_sensor(s))
    elif k == "modula_two":         cg.add(parent.set_modula_two_sensor(s))
    elif k == "modula_tbv":         cg.add(parent.set_modula_tbv_sensor(s))
    elif k == "modula_tbr":         cg.add(parent.set_modula_tbr_sensor(s))
    elif k == "modula_tv":          cg.add(parent.set_modula_tv_sensor(s))
    elif k == "modula_tv2":         cg.add(parent.set_modula_tv2_sensor(s))
    elif k == "modula_tr":          cg.add(parent.set_modula_tr_sensor(s))
    elif k == "modula_tr2":         cg.add(parent.set_modula_tr2_sensor(s))
    elif k == "modula_tpo":         cg.add(parent.set_modula_tpo_sensor(s))
    elif k == "modula_tpu":         cg.add(parent.set_modula_tpu_sensor(s))
    elif k == "modula_tzr":         cg.add(parent.set_modula_tzr_sensor(s))
    elif k == "espresso_t01":       cg.add(parent.set_espresso_t01_sensor(s))
    elif k == "espresso_t02":       cg.add(parent.set_espresso_t02_sensor(s))
    elif k == "espresso_t03":       cg.add(parent.set_espresso_t03_sensor(s))
    elif k == "espresso_t04":       cg.add(parent.set_espresso_t04_sensor(s))
    elif k == "espresso_t05":       cg.add(parent.set_espresso_t05_sensor(s))
    elif k == "espresso_pk":        cg.add(parent.set_espresso_pk_sensor(s))
    elif k == "espresso_t06":       cg.add(parent.set_espresso_t06_sensor(s))
    elif k == "espresso_t07":       cg.add(parent.set_espresso_t07_sensor(s))
    elif k == "espresso_t08":       cg.add(parent.set_espresso_t08_sensor(s))
    elif k == "espresso_t09":       cg.add(parent.set_espresso_t09_sensor(s))
    elif k == "espresso_t10":       cg.add(parent.set_espresso_t10_sensor(s))
    elif k == "espresso_t11":       cg.add(parent.set_espresso_t11_sensor(s))
    elif k == "espresso_t12":       cg.add(parent.set_espresso_t12_sensor(s))
