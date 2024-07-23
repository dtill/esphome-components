# MCP45HVX1 Digital POT component

For MCP45HVX1 7/8-Bit Single, +36V (±18V) Digital POT with I2C Serial Interface and Volatile Memory

This component provides support for the [MCP45HVX1](https://ww1.microchip.com/downloads/en/DeviceDoc/20005304A.pdf) series.

You need an `i2c:` component configured.

Example:
```yaml
esp8266:
  board: nodemcuv2

i2c:
  sda: GPIO4
  scl: GPIO5
  frequency: 400kHz
  scan: true
  id: i2c0

mcp45hvx1:
  id: digipot1
  i2c_id: i2c0
  address: 0x3c # (default is 0x3c but can be set by address-PIN A0 (5) and A1 (3) to 0x3d, 0x3e and 0x3f)
```