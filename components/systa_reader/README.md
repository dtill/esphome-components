# Systa-Reader ESPHome Component
An esphome component for the SystaReader-v2 Bus-Adapter.


Supported Features: read Paradigma SystaBus messages

Tested Paradigma Systa-hardware: SystaSolar/Aqua/Modulo II/Espresso.

[Example YAML](../../example_systa_reader.yaml) configuration:
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/dtill/esphome-components
    components: [systa_reader]
    refresh: 0s

uart:
  id: uart_bus
  tx_pin: GPIO1
  rx_pin: GPIO13
  baud_rate: 9600
  data_bits: 8
  parity: NONE
  stop_bits: 1

systa_reader:                             # can be multiple systa_reader but only one per uart.
  - id: systa_bus_01
    uart_id: uart_bus
    systa_device: [aqua, modula, espresso]  # call one or more devices [aqua,modula,espresso]
    log_invalid: true                     # logs invalid frames for debugging purpose

sensor:
    # AQUA sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_tsa
    name: "AQUA TSA (Kollektor)"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_tse
    name: "AQUA TSE (Eintritt)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_twu
    name: "AQUA TWU (unten)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_tw2
    name: "AQUA TW2 (oben)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_sol
    name: "AQUA Solare Leistung"
    unit_of_measurement: "kW"
    accuracy_decimals: 0
    device_class: energy
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_tag
    name: "AQUA Tagesgewinn"
    unit_of_measurement: "kWh"
    accuracy_decimals: 0
    device_class: energy
    state_class: total_increasing
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_gesamt
    name: "AQUA Solargewinn gesamt"
    unit_of_measurement: "kWh"
    accuracy_decimals: 0
    device_class: energy
    state_class: total
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_status_code
    name: "AQUA Status Code"
    accuracy_decimals: 0

    # MODULA sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_two
    name: "MODULA TWO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tbv
    name: "MODULA TBV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tbr
    name: "MODULA TBR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tv
    name: "MODULA TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tv2
    name: "MODULA TV2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tr
    name: "MODULA TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tr2
    name: "MODULA TR2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tpo
    name: "MODULA TPO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tpu
    name: "MODULA TPU"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_tzr
    name: "MODULA TZR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement

  # ESPRESSO sensors
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_t01
    name: "Espresso T01"
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_t02
    name: "Espresso T02"
  # … bis espresso_t12 analog …
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_pk
    name: "Espresso PK"

text_sensor:
  # AQUA text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: aqua_status_text
    name: "AQUA Status Text"
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: aqua_timestamp
    name: "AQUA Zeitstempel"
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: aqua_display_text
    name: "AQUA Displaytext"

  # MODULA text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: modula_timestamp
    name: "MODULA Zeitstempel"

  # ESPRESSO text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: espresso_timestamp
    name: "Espresso Zeitstempel"

  # General Systa BUS Frames in RAW HEX format
  # (use with care! only for debug purpose, because big text data chunks filling
  # homeassistant history data space over longer periods):

  #- platform: systa_reader
  #  systa_reader_id: systa_bus_01
  #  mode: raw
  #  filter: all
  #  name: "Systa Raw HEX (ALL)"
  #- platform: systa_reader
  #  systa_reader_id: systa_bus_01
  #  mode: raw
  #  filter: aqua
  #  name: "Systa Raw HEX (AQUA)"
```