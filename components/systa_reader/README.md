# Systa-Reader ESPHome Component

An esphome component for the DIY "Systa-BUS-Reader" Bus-Adapter, 
based and inspired by this [ringwelt.de](https://ringwelt.de/homeautomation/heizungsanlage/einfuehrung.html)-blog

Supported Features: read and interpret Paradigma SystaBus messages

Tested Paradigma Systa-hardware: SystaSolar/Aqua/Modulo II/Expresso/SystaComfort I.

[Example YAML](../../example_systa_reader.yaml) configuration:
```yaml
# for updates and code examples + more systa devices supported please check
# https://github.com/dtill/esphome-components/tree/main/components/systa_reader

external_components:
  - source:
      type: git
      url: https://github.com/dtill/esphome-components
    components: [systa_reader]
    refresh: 0s

time:
  - platform: homeassistant
    id: homeassistant_time
    on_time:
      # Every Day at specified hour
      - seconds: 0
        minutes: 0
        hours: 0
        then:
          - sensor.integration.reset: aqua_ii_tagesgewinn_calculated #Reset power integrator

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
    systa_device: [aqua, modula, espresso]  # call one or more devices [aqua, aqua_ii, modula, espresso, compact]
    log_invalid: true                     # logs invalid frames for debugging purpose

sensor:
    # AQUA sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_tsa
    name: "AQUA TSA (Kollektor)"
    unit_of_measurement: "°C"
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

    # AQUA-II sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_tsa
    name: "AQUA-II TSA1 (Kollektor)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_tsa2
    name: "AQUA-II TSA2 (Kollektor2)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_twu
    name: "AQUA-II TWU (Speicher Unten)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_tsv
    name: "AQUA-II TSV (Solarvorlauf)"
    id: aqua_ii_tsv
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_tam
    name: "AQUA-II TAM (Außen)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_tam2
    name: "AQUA-II TAM2 (Außentemp2)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_tse
    name: "AQUA-II TSE (Solarrücklauf)"
    id: aqua_ii_tse
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_dfl
    name: "AQUA-II Durchfluss"
    id: aqua_ii_dfl
    unit_of_measurement: "l/min"
    accuracy_decimals: 1
    device_class: volume_flow_rate
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_pwm
    name: "AQUA-II PWM (Pumpe)"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: template
    name: "AQUA-II Kollektorleistung (berechnet)"
    id: aqua_ii_kollektorleistung_calculated
    unit_of_measurement: "kW"
    accuracy_decimals: 2
    device_class: energy
    state_class: measurement
    update_interval: 10s
    lambda: |-
      const float dfl = id(aqua_durchfluss).state;  // l/min
      const float tsv = id(aqua_tsv).state;         // °C
      const float tse = id(aqua_tse).state;         // °C
      if (isnan(dfl) || isnan(tsv) || isnan(tse)) return 0.0f;
      const float deltaT = tsv - tse;               // K
      const float power_kW = (dfl / 60.0f) * 1.0f * 4.18f * deltaT;
      return power_kW < 0.0f ? 0.0f : power_kW;
  - platform: integration
    name: "AQUA-II Tagesgewinn (berechnet)"
    id: aqua_ii_tagesgewinn_calculated
    sensor: aqua_ii_kollektorleistung_calculated
    integration_method: left
    time_unit: h
    unit_of_measurement: "kWh"
    accuracy_decimals: 2
    device_class: energy
    state_class: total_increasing
    restore: false
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_tag
    name: "AQUA-II Tagesgewinn"
    unit_of_measurement: "kWh"
    accuracy_decimals: 0
    device_class: energy
    state_class: total_increasing
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_gesamt
    name: "AQUA-II Solargewinn gesamt"
    unit_of_measurement: "kWh"
    accuracy_decimals: 0
    device_class: energy
    state_class: total_increasing
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_status_code
    name: "AQUA-II Status Code"
    accuracy_decimals: 0

    # MODULA sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: modula_ta
    name: "MODULA TA"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
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
    kind: espresso_ta
    name: "ESPRESSO TA"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_two
    name: "ESPRESSO TWO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_fa_tv
    name: "ESPRESSO FA TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_fa_tr
    name: "ESPRESSO FA TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk1_ti
    name: "ESPRESSO HK1 TI"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk2_ti2
    name: "ESPRESSO HK2 TI2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk1_tv
    name: "ESPRESSO HK1 TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk2_tv2
    name: "ESPRESSO HK2 TV2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk1_tr
    name: "ESPRESSO HK1 TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk2_tr2
    name: "ESPRESSO HK2 TR2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_tpo
    name: "ESPRESSO TPO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_tpu
    name: "ESPRESSO TPU"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_tzr
    name: "ESPRESSO TZR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_pk
    name: "ESPRESSO PK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk1_phk
    name: "ESPRESSO HK1 PHK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk2_phk2
    name: "ESPRESSO HK2 PHK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement

  # COPMACT sensors
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_ta
    name: "COMPACT TA"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_two
    name: "COMPACT TWO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_fa_tv
    name: "COMPACT FA TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_fa_tr
    name: "COMPACT FA TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_ti
    name: "COMPACT TI"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_ti_s
    name: "COMPACT TI SOLL"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_tv_s
    name: "COMPACT TV SOLL"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_two_s
    name: "COMPACT TWO SOLL"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: compact_status_code
    name: "COMPACT Status Code"
    accuracy_decimals: 0

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

  # AQUA-II text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: aqua_ii_status_text
    name: "AQUA-II Status Text"
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: aqua_ii_timestamp
    name: "AQUA-II Zeitstempel"

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

  # COMPACT text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: compact_timestamp
    name: "Compact Zeitstempel"

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