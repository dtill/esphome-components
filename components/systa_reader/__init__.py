import re
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

AUTO_LOAD = ["sensor", "text_sensor"]
CODEOWNERS = ["@dtill"]
DEPENDENCIES = ["uart", "sensor", "text_sensor"]
MULTI_CONF = True  # allow multiple systa_reader blocks, but we'll enforce 1 per UART

systa_ns = cg.esphome_ns.namespace("systa_reader")
SystaReader = systa_ns.class_("SystaReader", uart.UARTDevice, cg.Component)

# ----- devices (list)
SYSTA_DEVICE = cv.one_of("aqua", "aqua_ii", "modula", "espresso", "solar", lower=True)
SYSTA_DEVICES_LIST = cv.All(cv.ensure_list(SYSTA_DEVICE), cv.Length(min=1))

# bit flags (room to grow—switch to 64-bit in C++ if you like)
DEV_FLAGS = {
    "aqua":     1 << 0,
    "aqua_ii":  1 << 1,
    "modula":   1 << 2,
    "espresso": 1 << 3,
    "solar":    1 << 4,
}

CONF_UART_ID = "uart_id"
CONF_LOG_INVALID = "log_invalid"
CONF_DEVICES = "systa_device"  # list, e.g. systa_device: [aqua, espresso]
CONF_TEST_DATA = "test_data"

HEX_RE = re.compile(r"^[0-9A-Fa-f]+$")

def _hex_to_bytes(s: str) -> bytes:
    if not HEX_RE.match(s):
        raise cv.Invalid("test_data must be a hex string (e.g. 'FC1F...').")
    if len(s) % 2 != 0:
        raise cv.Invalid("test_data must have an even number of hex digits.")
    return bytes.fromhex(s)

def _twos_complement_checksum(b: bytes) -> int:
    # sum(all bytes except last) -> two's complement on 8-bit
    total = sum(b[:-1]) & 0xFF
    return (-total) & 0xFF

def _validate_test_data(value: str) -> str:
    raw = _hex_to_bytes(value)
    if len(raw) < 3:
        raise cv.Invalid("test_data: too short to be a valid frame.")
    # FC frame?
    if raw[0] == 0xFC:
        if len(raw) < 3:
            raise cv.Invalid("test_data (FC): too short.")
        declared_len = raw[1]
        total = 2 + declared_len + 1
        if len(raw) != total:
            raise cv.Invalid(
                f"test_data (FC): length mismatch, got {len(raw)} bytes, "
                f"expected {total} (len={declared_len})."
            )
        exp = _twos_complement_checksum(raw)
        got = raw[-1]
        if got != exp:
            raise cv.Invalid(
                f"test_data (FC): checksum mismatch, provided {got:02X}, expected {exp:02X}."
            )
        return value.upper()
    # Display frame 0F 22 04 00 (fixed 37 bytes)
    if len(raw) >= 4 and raw[0] == 0x0F and raw[1] == 0x22 and raw[2] == 0x04 and raw[3] == 0x00:
        if len(raw) != 37:
            raise cv.Invalid(f"test_data (Display): expected 37 bytes, got {len(raw)}.")
        exp = _twos_complement_checksum(raw)
        got = raw[-1]
        if got != exp:
            raise cv.Invalid(
                f"test_data (Display): checksum mismatch, provided {got:02X}, expected {exp:02X}."
            )
        return value.upper()

    raise cv.Invalid("test_data: unsupported frame header (expected FC... or 0F220400...).")


CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SystaReader),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_LOG_INVALID, default=True): cv.boolean,
    cv.Required(CONF_DEVICES): SYSTA_DEVICES_LIST,
    cv.Optional(CONF_TEST_DATA): cv.ensure_list(cv.All(cv.string, _validate_test_data)),
}).extend(cv.COMPONENT_SCHEMA)

# keep a module-level set of claimed UART ids to forbid duplicates
_uart_claims = set()

def _devices_to_mask(names) -> int:
    mask = 0
    for n in names:
        mask |= DEV_FLAGS[n]
    return mask

async def to_code(config):
    # enforce one systa_reader per UART
    uart_id_obj = config[CONF_UART_ID]
    uart_key = str(uart_id_obj.id)
    if uart_key in _uart_claims:
        raise cv.Invalid(
            f"Only one systa_reader is allowed per UART. The UART '{uart_key}' "
            f"is already used by another systa_reader."
        )
    _uart_claims.add(uart_key)

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_log_invalid(config[CONF_LOG_INVALID]))

    mask = _devices_to_mask(config[CONF_DEVICES])
    cg.add(var.set_enabled_mask(mask))

    if CONF_TEST_DATA in config:
        cg.add(var.set_test_data_frames(config[CONF_TEST_DATA]))