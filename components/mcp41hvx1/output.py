import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import spi, output
from esphome.const import (
    CONF_ID,
    CONF_INITIAL_VALUE,
    UNIT_OHM,
)

CODEOWNERS = ["@dtill"]
mcp41hvx1_ns = cg.esphome_ns.namespace("mcp41hvx1")

Mcp41hvx1Output = mcp41hvx1_ns.class_("Mcp41hvx1Output", output.FloatOutput, cg.Component, spi.SPIDevice)

CONF_SHDN_PIN = "shdn_pin"
CONF_WLAT_PIN = "wlat_pin"

CONFIG_SCHEMA = cv.All(
    output.FLOAT_OUTPUT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(Mcp41hvx1Output),
            cv.Optional(CONF_SHDN_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_WLAT_PIN): pins.internal_gpio_output_pin_schema,
            cv.Optional(CONF_INITIAL_VALUE, default=0.5): cv.float_range(
                # default refers to the wiper POR (Power-On Reset) and BOR (Brown-Out Reset) value 0x7F.
                # Means wiper is in "middle" position between Terminal-A and Terminal-B for example on startup.
                min=0.01, max=1.0
            ),
        }
    )
    .extend(spi.spi_device_schema())
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await output.register_output(var, config)
    await spi.register_spi_device(var, config)

    if CONF_SHDN_PIN in config:
        shdn_pin = await cg.gpio_pin_expression(config[CONF_SHDN_PIN])
        cg.add(var.set_shdn_pin(shdn_pin))
    if CONF_WLAT_PIN in config:
        wlat_pin = await cg.gpio_pin_expression(config[CONF_WLAT_PIN])
        cg.add(var.set_wlat_pin(wlat_pin))

    cg.add(var.set_initial_value(config[CONF_INITIAL_VALUE]))
