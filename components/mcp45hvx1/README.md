# MCP45HVX1 Digital Potentiometer output component

For MCP45HVX1 7/8-Bit Single, +36V (±18V) Digital POT with I2C Serial Interface and Volatile Memory

This output component provides support for the [MCP45HVX1](https://ww1.microchip.com/downloads/en/DeviceDoc/20005304A.pdf) chip.

Supported Features: Wiper Read/Write. 

TODO: Incriment/Decremnt Wiper command, TCON Read/Write to connect/disconnect Terminal- A,B,W from resistor network, General Call Commands


ATTENTION: There is an issue with i2c Interface on these chips. Read [MCP45HVX1 Rev. A1 Silicon/Data Sheet Errata](https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/Errata/80000649B.pdf).
In short: General Call commands do not properly work and can lead to failure states.
Workaround: "Each MCP45HVX1 device would need to be on a unique I2C bus with no other I2C devices."

You need an `i2c:` component configured.

Tested with MCP45HV51-503E/ST (50kOhms 8bit) 

[Example YAML](../../example_mcp45hvx1.yaml) configuration:
```yaml
esp8266:
  board: nodemcuv2

external_components:
  - source:
      type: git
      url: https://github.com/dtill/esphome-mcp45hvx1
    components: [mcp45hvx1]
    
i2c:
  sda: GPIO4
  scl: GPIO5
  frequency: 400kHz
  scan: true
  id: i2c0

output:
  - platform: mcp45hvx1  
    id: digipot1
    i2c_id: i2c0
    address: 0x3c
    # Attention: There is an issue with i2c Interface on these chips. Rever to https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/Errata/80000649B.pdf for more details.
    # Workaround: "Each MCP45HVX1 device would need to be on a unique I2C bus with no other I2C devices."
    # device i2c-address default is 0x3c but can be set by address-PIN A0 (5) and A1 (3) to 0x3d, 0x3e and 0x3f
```