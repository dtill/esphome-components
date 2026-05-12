# Systa-Reader ESPHome Component

An esphome component for the DIY "Systa-BUS-Reader" Bus-Adapter, 
based and inspired by 
    
- [ringwelt.de](https://ringwelt.de/homeautomation/heizungsanlage/einfuehrung.html) (Hardware circuit)
- [SystaBridge](https://github.com/marvinGitHub/systa-bridge) by [marvinGitHub](https://github.com/marvinGitHub) (Decoding Systa Comfort II Protocol )

Supported Features: read and interpret Paradigma SystaBus messages

Tested Paradigma Systa-hardware: 
 - Solar
 - Aqua / Aqua II
 - Modula II
 - Expresso
 - Palletti II
 - Compact
 - SystaComfort II
 - Bedienteil Display

[Comfort II Example YAML](../../example_systa_reader_comfort.yaml)

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

uart:
  id: uart_bus
  tx_pin: GPIO1
  rx_pin: GPIO13
  baud_rate: 9600
  data_bits: 8
  parity: NONE
  stop_bits: 1

systa_reader:                       # can be multiple systa_reader but only one per uart.
  - id: systa_bus_01
    uart_id: uart_bus
    systa_device: [aqua_ii]     # call one or more devices [aqua, aqua_ii, modula, espresso, palletti_ii, compact, comfort]
    log_invalid: true               # logs invalid frames for debugging purpose

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
    kind: aqua_pwm
    name: "AQUA PWM (Pumpe)"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_var1
    name: "AQUA VAR1" # VAR1: 3 = Betrieb 8 = Aus/Standby 0 = Abschalten wegen T-Max
    accuracy_decimals: 0
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_stat
    name: "AQUA STAT" # This sensor is on some systems with aqua_ii the "STATUS_SOLAR"
    accuracy_decimals: 0
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
    name: "AQUA-II TSA (Kollektor)"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: aqua_ii_tsa1
    name: "AQUA-II TSA1 (Kollektor1)"
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

  # PALLETTI-II sensors
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_ta
    name: "PALLETTI-II TA"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_two
    name: "PALLETTI-II TWO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_fa_tv
    name: "PALLETTI-II FA TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_fa_tr
    name: "PALLETTI-II FA TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk1_ti
    name: "PALLETTI-II HK1 TI"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk2_ti2
    name: "PALLETTI-II HK2 TI2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk1_tv
    name: "PALLETTI-II HK1 TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk2_tv2
    name: "PALLETTI-II HK2 TV2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk1_tr
    name: "PALLETTI-II HK1 TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk2_tr2
    name: "PALLETTI-II HK2 TR2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_tpo
    name: "PALLETTI-II TPO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_tpu
    name: "PALLETTI-II TPU"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_tzr
    name: "PALLETTI-II TZR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_pk
    name: "PALLETTI-II PK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk1_phk
    name: "PALLETTI-II HK1 PHK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk2_phk2
    name: "PALLETTI-II HK2 PHK"
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
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: aqua_fw_version
    name: "AQUA FW Version"

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
    name: "ESPRESSO Zeitstempel"

  # PALLETTI-II text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: palletti_ii_timestamp
    name: "PALLETTI-II Zeitstempel"
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: palletti_ii_display_text
    name: "PALLETTI-II Displaytext"

  # COMPACT text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: compact_timestamp
    name: "Compact Zeitstempel"
```