import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor as sensor_comp, text_sensor as text_comp
from esphome.const import CONF_ID

AUTO_LOAD = ["text_sensor"]  # raw HEX bleibt verfügbar
CODEOWNERS = ["@dtill"]
DEPENDENCIES = ["uart", "sensor", "text_sensor"]
MULTI_CONF = True

systa_ns = cg.esphome_ns.namespace("systa_reader")
SystaReader = systa_ns.class_("SystaReader", uart.UARTDevice, cg.Component)

# Device types
SYSTA_DEVICE = cv.one_of("aqua", "modula", "espresso", "solar", lower=True)

CONF_UART_ID = "uart_id"
CONF_LOG_INVALID = "log_invalid"
CONF_DEVICE = "systa_device"

# AQUA sensor keys
CONF_TSA = "tsa"
CONF_TSE = "tse"
CONF_TWU = "twu"
CONF_TW2 = "tw2"
CONF_SOL = "sol"
CONF_TAG = "tag"
CONF_GES = "gesamt"
CONF_STATUS_CODE = "status_code"
CONF_STATUS_TEXT = "status_text"
CONF_TIMESTAMP = "timestamp"

AQUA_SCHEMA = cv.Schema({
    cv.Optional(CONF_TSA): sensor_comp.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1, device_class="temperature", state_class="measurement"),
    cv.Optional(CONF_TSE): sensor_comp.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1, device_class="temperature", state_class="measurement"),
    cv.Optional(CONF_TWU): sensor_comp.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1, device_class="temperature", state_class="measurement"),
    cv.Optional(CONF_TW2): sensor_comp.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1, device_class="temperature", state_class="measurement"),
    cv.Optional(CONF_SOL): sensor_comp.sensor_schema(unit_of_measurement="kW", accuracy_decimals=0, device_class="energy", state_class="measurement"),
    cv.Optional(CONF_TAG): sensor_comp.sensor_schema(unit_of_measurement="kWh", accuracy_decimals=0, device_class="energy", state_class="total_increasing"),
    cv.Optional(CONF_GES): sensor_comp.sensor_schema(unit_of_measurement="kWh", accuracy_decimals=0, device_class="energy", state_class="total"),
    cv.Optional(CONF_STATUS_CODE): sensor_comp.sensor_schema(accuracy_decimals=0),
    cv.Optional(CONF_STATUS_TEXT): text_comp.text_sensor_schema(),
    cv.Optional(CONF_TIMESTAMP): text_comp.text_sensor_schema(),
})

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SystaReader),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_LOG_INVALID, default=True): cv.boolean,
    cv.Optional(CONF_DEVICE, default="aqua"): SYSTA_DEVICE,
    cv.Optional("aqua"): AQUA_SCHEMA,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    cg.add(var.set_log_invalid(config[CONF_LOG_INVALID]))
    cg.add(var.set_device_type(config.get(CONF_DEVICE)))

    if "aqua" in config:
        aqua = config["aqua"]
        if CONF_TSA in aqua:
            s = await sensor_comp.new_sensor(aqua[CONF_TSA]); cg.add(var.set_aqua_tsa_sensor(s))
        if CONF_TSE in aqua:
            s = await sensor_comp.new_sensor(aqua[CONF_TSE]); cg.add(var.set_aqua_tse_sensor(s))
        if CONF_TWU in aqua:
            s = await sensor_comp.new_sensor(aqua[CONF_TWU]); cg.add(var.set_aqua_twu_sensor(s))
        if CONF_TW2 in aqua:
            s = await sensor_comp.new_sensor(aqua[CONF_TW2]); cg.add(var.set_aqua_tw2_sensor(s))
        if CONF_SOL in aqua:
            s = await sensor_comp.new_sensor(aqua[CONF_SOL]); cg.add(var.set_aqua_sol_sensor(s))
        if CONF_TAG in aqua:
            s = await sensor_comp.new_sensor(aqua[CONF_TAG]); cg.add(var.set_aqua_tag_sensor(s))
        if CONF_GES in aqua:
            s = await sensor_comp.new_sensor(aqua[CONF_GES]); cg.add(var.set_aqua_ges_sensor(s))
        if CONF_STATUS_CODE in aqua:
            s = await sensor_comp.new_sensor(aqua[CONF_STATUS_CODE]); cg.add(var.set_aqua_status_code_sensor(s))
        if CONF_STATUS_TEXT in aqua:
            t = await text_comp.new_text_sensor(aqua[CONF_STATUS_TEXT]); cg.add(var.set_aqua_status_text_sensor(t))
        if CONF_TIMESTAMP in aqua:
            t = await text_comp.new_text_sensor(aqua[CONF_TIMESTAMP]); cg.add(var.set_aqua_timestamp_text_sensor(t))
