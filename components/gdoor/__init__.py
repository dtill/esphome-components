import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID

CODEOWNERS = ["@dtill"]
DOMAIN = "gdoor"
DEPENDENCIES = []  # No platform dependency here. The platforms will list their own dependency.
MULTI_CONF = True

# Create a namespace for your custom integration.
gdoor_esphome_ns = cg.esphome_ns.namespace("gdoor_esphome")

# Define your base component. This is not a sensor or an output on its own.
GdoorComponent = gdoor_esphome_ns.class_("GdoorComponent", cg.Component)

# Define your own configuration options.
CONF_TX_PIN = "tx_pin"
CONF_TX_EN_PIN = "tx_en_pin"
CONF_RX_PIN = "rx_pin"
CONF_RX_SENS = "rx_sens"

DEFAULT_TX_PIN = 25
DEFAULT_TX_EN_PIN = 27
DEFAULT_RX_PIN = 22

RX_SENS_MODES = {
    "low": 1.3,
    "med": 1.45,
    "high": 1.65,
}

def validate_rx_sens_and_pin(cfg):
    """
    Enforces:
      - If RX pin is not 22, then either rx_sens is not provided or must be "high".
    """
    rx_pin = cfg.get(CONF_RX_PIN, DEFAULT_RX_PIN)
    if rx_pin != DEFAULT_RX_PIN:
        # If rx_sens is given and is not 'high', raise an error.
        if CONF_RX_SENS in cfg and cfg[CONF_RX_SENS] != "high":
            raise cv.Invalid("If rx_pin is not 22, rx_sens must be 'high'.")
    return cfg

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.Optional(CONF_TX_PIN, default=DEFAULT_TX_PIN): pins.internal_gpio_output_pin_schema,
        cv.Optional(CONF_TX_EN_PIN, default=DEFAULT_TX_EN_PIN): pins.internal_gpio_output_pin_schema,
        cv.Optional(CONF_RX_PIN, default=DEFAULT_RX_PIN): pins.internal_gpio_input_pin_schema,
        cv.Optional(CONF_RX_SENS): cv.enum(RX_SENS_MODES, upper=False),
    }).extend(cv.COMPONENT_SCHEMA),
    validate_rx_sens_and_pin
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    tx_pin = await cg.gpio_pin_expression(config[CONF_TX_PIN])
    cg.add(var.set_tx_pin(tx_pin))

    tx_en_pin = await cg.gpio_pin_expression(config[CONF_TX_EN_PIN])
    cg.add(var.set_tx_en_pin(tx_en_pin))

    rx_pin = await cg.gpio_pin_expression(config[CONF_RX_PIN])
    cg.add(var.set_rx_pin(rx_pin))

    # If rx_sens is specified, set it.
    if CONF_RX_SENS in config:
        cg.add(var.set_rx_sens(config[CONF_RX_SENS]))
