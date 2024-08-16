# MCP41HVX1 SPI Potentiometer Output

For MCP41HVX1 7/8-Bit Single, +36V (±18V) Digital POT with SPI Serial Interface and Volatile Memory

This output component provides support for the [MCP41HVX1](https://ww1.microchip.com/downloads/en/DeviceDoc/20005207B.pdf) chip.

Supported Features: Wiper Read/Write. 

TODO: Incriment/Decremnt Wiper command, control resistor network via SHDN and WLAT PINs

You need an `spi:` component configured.

Tested with MCP41HV51-503E/ST (50kOhms 8bit) 

[Example YAML](../../example_mcp41hvx1.yaml) configuration:
```yaml
esp8266:
  board: nodemcuv2

external_components:
  - source:
      type: git
      url: https://github.com/dtill/esphome-components
    components: [mcp41hvx1]
    
spi:
  - id: spi_bus0
    clk_pin: GPIO14
    mosi_pin: GPIO13
    miso_pin: GPIO12
    interface: any

output:
  - platform: mcp41hvx1  
    id: digipot_spi_1
    cs_pin: GPIO15      # optional parameter
    data_rate: 1MHz     # optional 80MHz, 40MHz, 20MHz, 10MHz, 5MHz, 4MHz, 2MHz, 1MHz (default), 200kHz, 75kHz or 1kHz
    mode: 3             # optional mode0, mode1, mode2, mode3 (default)

```