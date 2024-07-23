import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import output
from esphome.components import i2c
from esphome.const import (
    CONF_ID,
    CONF_INITIAL_VALUE,
    CONF_STEP_DELAY,  # Add if needed
)

CODEOWNERS = ["@dtill"]

mcp45hvx1_ns = cg.esphome_ns.namespace("mcp45hvx1")

Mcp45hvx1Output = mcp45hvx1_ns.class_("Mcp45hvx1Output", output.FloatOutput, cg.Component, i2c.I2CDevice)

CONF_SHDN_PIN = "shdn_pin"
CONF_WLAT_PIN = "wlat_pin"

CONFIG_SCHEMA = cv.All(
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(Mcp45hvx1Output),
            cv.Optional(CONF_SHDN_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_WLAT_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_INITIAL_VALUE, default=1.0): cv.float_range(
                min=0.01, max=1.0
            ),
        }
    )
    .extend(i2c.i2c_device_schema(0x3c))
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_SHDN_PIN in config:
        shdn_pin = await cg.gpio_pin_expression(config[CONF_SHDN_PIN])
        cg.add(var.set_shdn_pin(shdn_pin))
    if CONF_WLAT_PIN in config:
        wlat_pin = await cg.gpio_pin_expression(config[CONF_WLAT_PIN])
        cg.add(var.set_wlat_pin(wlat_pin))

    cg.add(var.set_initial_value(config[CONF_INITIAL_VALUE]))
