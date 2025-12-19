import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from .. import systa_ns, SystaReader

CONF_PARENT_ID = "systa_reader_id"
CONF_KIND = "kind"

KIND = cv.one_of(
    # AQUA
    "aqua_tsa","aqua_tse","aqua_twu","aqua_tw2","aqua_sol","aqua_tag","aqua_gesamt","aqua_status_code",
    # AQUA_II
    "aqua_ii_tsa","aqua_ii_twu","aqua_ii_tsv","aqua_ii_tam","aqua_ii_tse","aqua_ii_dfl","aqua_ii_pwm",
    "aqua_ii_koll_lstg", "aqua_ii_tag","aqua_ii_gesamt", "aqua_ii_tsa2", "aqua_ii_tam2", "aqua_ii_status_code",
    # MODULA
    "modula_ta","modula_two","modula_tbv","modula_tbr","modula_tv","modula_tv2",
    "modula_tr","modula_tr2","modula_tpo","modula_tpu","modula_tzr",
    # ESPRESSO
    "espresso_ta","espresso_two","espresso_fa_tv","espresso_fa_tr","espresso_hk1_ti","espresso_hk2_ti2",
    "espresso_hk1_tv","espresso_hk2_tv2","espresso_hk1_tr","espresso_hk2_tr2","espresso_tpo",
    "espresso_tpu","espresso_tzr","espresso_pk","espresso_hk1_phk","espresso_hk2_phk2",
    # PALLETTI-II
    "palletti_ii_ta","palletti_ii_two","palletti_ii_fa_tv","palletti_ii_fa_tr","palletti_ii_hk1_ti","palletti_ii_hk2_ti2",
    "palletti_ii_hk1_tv","palletti_ii_hk2_tv2","palletti_ii_hk1_tr","palletti_ii_hk2_tr2","palletti_ii_tpo",
    "palletti_ii_tpu","palletti_ii_tzr","palletti_ii_pk","palletti_ii_hk1_phk","palletti_ii_hk2_phk2",
    # COMPACT
    "compact_ta","compact_two","compact_fa_tv","compact_fa_tr","compact_ti","compact_ti_s",
    "compact_tv_s","compact_two_s","compact_status_code",
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
    elif k == "aqua_ii_tsa":        cg.add(parent.set_aqua_ii_tsa_sensor(s))
    elif k == "aqua_ii_twu":        cg.add(parent.set_aqua_ii_twu_sensor(s))
    elif k == "aqua_ii_tsv":        cg.add(parent.set_aqua_ii_tsv_sensor(s))
    elif k == "aqua_ii_tam":        cg.add(parent.set_aqua_ii_tam_sensor(s))
    elif k == "aqua_ii_tse":        cg.add(parent.set_aqua_ii_tse_sensor(s))
    elif k == "aqua_ii_dfl":        cg.add(parent.set_aqua_ii_dfl_sensor(s))
    elif k == "aqua_ii_pwm":        cg.add(parent.set_aqua_ii_pwm_sensor(s))
    elif k == "aqua_ii_koll_lstg":  cg.add(parent.set_aqua_ii_koll_lstg_sensor(s))
    elif k == "aqua_ii_tag":        cg.add(parent.set_aqua_ii_tag_sensor(s))
    elif k == "aqua_ii_gesamt":     cg.add(parent.set_aqua_ii_ges_sensor(s))
    elif k == "aqua_ii_tsa2":       cg.add(parent.set_aqua_ii_tsa2_sensor(s))
    elif k == "aqua_ii_tam2":       cg.add(parent.set_aqua_ii_tam2_sensor(s))
    elif k == "aqua_ii_status_code":cg.add(parent.set_aqua_ii_status_code_sensor(s))
    elif k == "modula_ta":          cg.add(parent.set_modula_ta_sensor(s))
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
    elif k == "espresso_ta":        cg.add(parent.set_espresso_ta_sensor(s))
    elif k == "espresso_two":       cg.add(parent.set_espresso_two_sensor(s))
    elif k == "espresso_fa_tv":     cg.add(parent.set_espresso_fa_tv_sensor(s))
    elif k == "espresso_fa_tr":     cg.add(parent.set_espresso_fa_tr_sensor(s))
    elif k == "espresso_hk1_ti":    cg.add(parent.set_espresso_hk1_ti_sensor(s))
    elif k == "espresso_hk2_ti2":   cg.add(parent.set_espresso_hk2_ti2_sensor(s))
    elif k == "espresso_hk1_tv":    cg.add(parent.set_espresso_hk1_tv_sensor(s))
    elif k == "espresso_hk2_tv2":   cg.add(parent.set_espresso_hk2_tv2_sensor(s))
    elif k == "espresso_hk1_tr":    cg.add(parent.set_espresso_hk1_tr_sensor(s))
    elif k == "espresso_hk2_tr2":   cg.add(parent.set_espresso_hk2_tr2_sensor(s))
    elif k == "espresso_tpo":       cg.add(parent.set_espresso_tpo_sensor(s))
    elif k == "espresso_tpu":       cg.add(parent.set_espresso_tpu_sensor(s))
    elif k == "espresso_tzr":       cg.add(parent.set_espresso_tzr_sensor(s))
    elif k == "espresso_pk":        cg.add(parent.set_espresso_pk_sensor(s))
    elif k == "espresso_hk1_phk":   cg.add(parent.set_espresso_hk1_phk_sensor(s))
    elif k == "espresso_hk2_phk2":  cg.add(parent.set_espresso_hk2_phk2_sensor(s))
    elif k == "palletti_ii_ta":        cg.add(parent.set_palletti_ii_ta_sensor(s))
    elif k == "palletti_ii_two":       cg.add(parent.set_palletti_ii_two_sensor(s))
    elif k == "palletti_ii_fa_tv":     cg.add(parent.set_palletti_ii_fa_tv_sensor(s))
    elif k == "palletti_ii_fa_tr":     cg.add(parent.set_palletti_ii_fa_tr_sensor(s))
    elif k == "palletti_ii_hk1_ti":    cg.add(parent.set_palletti_ii_hk1_ti_sensor(s))
    elif k == "palletti_ii_hk2_ti2":   cg.add(parent.set_palletti_ii_hk2_ti2_sensor(s))
    elif k == "palletti_ii_hk1_tv":    cg.add(parent.set_palletti_ii_hk1_tv_sensor(s))
    elif k == "palletti_ii_hk2_tv2":   cg.add(parent.set_palletti_ii_hk2_tv2_sensor(s))
    elif k == "palletti_ii_hk1_tr":    cg.add(parent.set_palletti_ii_hk1_tr_sensor(s))
    elif k == "palletti_ii_hk2_tr2":   cg.add(parent.set_palletti_ii_hk2_tr2_sensor(s))
    elif k == "palletti_ii_tpo":       cg.add(parent.set_palletti_ii_tpo_sensor(s))
    elif k == "palletti_ii_tpu":       cg.add(parent.set_palletti_ii_tpu_sensor(s))
    elif k == "palletti_ii_tzr":       cg.add(parent.set_palletti_ii_tzr_sensor(s))
    elif k == "palletti_ii_pk":        cg.add(parent.set_palletti_ii_pk_sensor(s))
    elif k == "palletti_ii_hk1_phk":   cg.add(parent.set_palletti_ii_hk1_phk_sensor(s))
    elif k == "palletti_ii_hk2_phk2":  cg.add(parent.set_palletti_ii_hk2_phk2_sensor(s))
    elif k == "compact_ta":        cg.add(parent.set_compact_ta_sensor(s))
    elif k == "compact_two":       cg.add(parent.set_compact_two_sensor(s))
    elif k == "compact_fa_tv":     cg.add(parent.set_compact_fa_tv_sensor(s))
    elif k == "compact_fa_tr":     cg.add(parent.set_compact_fa_tr_sensor(s))
    elif k == "compact_ti":        cg.add(parent.set_compact_ti_sensor(s))
    elif k == "compact_ti_s":      cg.add(parent.set_compact_ti_s_sensor(s))
    elif k == "compact_tv_s":      cg.add(parent.set_compact_tv_s_sensor(s))
    elif k == "compact_two_s":     cg.add(parent.set_compact_two_s_sensor(s))
    elif k == "compact_status_code":cg.add(parent.set_compact_status_code_sensor(s))