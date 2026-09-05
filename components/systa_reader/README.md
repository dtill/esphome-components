# Systa-Reader ESPHome Component

An esphome component for the DIY "Systa-BUS-Reader" Bus-Adapter, 
based and inspired by 
    
- [ringwelt.de](https://ringwelt.de/homeautomation/heizungsanlage/einfuehrung.html) (Hardware circuit)

Supported Features: read and interpret Paradigma SystaBus messages

Tested Paradigma Systa-hardware: 
 - Solar
 - Aqua / Aqua II
 - Modula II
 - Expresso
 - Expresso II
 - Pelletti II
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
    systa_device: [aqua_ii]     # call one or more devices [aqua, aqua_ii, modula, espresso, expresso_ii, palletti_ii, compact, comfort]
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
    state_class: measurement

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

  # EXPRESSO sensors
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_ta
    name: "EXPRESSO TA"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_two
    name: "EXPRESSO TWO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_fa_tv
    name: "EXPRESSO FA TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_fa_tr
    name: "EXPRESSO FA TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk1_ti
    name: "EXPRESSO HK1 TI"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk2_ti2
    name: "EXPRESSO HK2 TI2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk1_tv
    name: "EXPRESSO HK1 TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk2_tv2
    name: "EXPRESSO HK2 TV2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk1_tr
    name: "EXPRESSO HK1 TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk2_tr2
    name: "EXPRESSO HK2 TR2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_tpo
    name: "EXPRESSO TPO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_tpu
    name: "EXPRESSO TPU"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_tzr
    name: "EXPRESSO TZR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_pk
    name: "EXPRESSO PK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk1_phk
    name: "EXPRESSO HK1 PHK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: espresso_hk2_phk2
    name: "EXPRESSO HK2 PHK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement

  # PELLETTII-II sensors
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_ta
    name: "PELLETTII-II TA"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_two
    name: "PELLETTII-II TWO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_fa_tv
    name: "PELLETTII-II FA TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_fa_tr
    name: "PELLETTII-II FA TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk1_ti
    name: "PELLETTII-II HK1 TI"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk2_ti2
    name: "PELLETTII-II HK2 TI2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk1_tv
    name: "PELLETTII-II HK1 TV"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk2_tv2
    name: "PELLETTII-II HK2 TV2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk1_tr
    name: "PELLETTII-II HK1 TR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk2_tr2
    name: "PELLETTII-II HK2 TR2"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_tpo
    name: "PELLETTII-II TPO"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_tpu
    name: "PELLETTII-II TPU"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_tzr
    name: "PELLETTII-II TZR"
    unit_of_measurement: "°C"
    accuracy_decimals: 1
    device_class: temperature
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_pk
    name: "PELLETTII-II PK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk1_phk
    name: "PELLETTII-II HK1 PHK"
    unit_of_measurement: "%"
    accuracy_decimals: 0
    device_class: power_factor
    state_class: measurement
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    kind: palletti_ii_hk2_phk2
    name: "PELLETTII-II HK2 PHK"
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
    kind: aqua_fw_version            # decoded from FD 05 AA 0B <Maj> <min> <patch>
    entity_category: diagnostic
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

  # EXPRESSO text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: espresso_timestamp
    name: "EXPRESSO Zeitstempel"

  # PELLETTII-II text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: palletti_ii_timestamp
    name: "PELLETTII-II Zeitstempel"
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: palletti_ii_display_text
    name: "PELLETTII-II Displaytext"

  # COMPACT text sensors:
  - platform: systa_reader
    systa_reader_id: systa_bus_01
    mode: field
    kind: compact_timestamp
    name: "Compact Zeitstempel"

button:
  - platform: restart
    id: urgrow_firmware_restart
    name:  "Neustarten"
```

## Expresso II (`expresso_ii`)

Status frame `FC 37 14 01 ...` (58 bytes), firmware announce `FD 05 AA 14 ...`,
device address `0x14`.

Expresso II is close enough to Espresso I to be tempting to share a decoder,
but different enough that it has its own. The frame is 58 bytes instead of
~38, the register block past offset 16 is remapped, and the timestamp uses a
different encoding. Espresso I drives two heating circuits and a buffer;
Expresso II drives a fresh water station, so the same byte positions carry
flow rates and setpoints instead of temperatures.

### Timestamp

The timestamp sits at `payload[0..3]` like on Espresso I, but is **not** BCD.
It uses the Comfort keypad's `UHR` encoding: two big-endian u16, minutes since
midnight followed by days since 2000-01-01.

### Identified registers

| Kind | Frame offset | Type | Unit |
|------|--------------|------|------|
| `expresso_ii_ta`      | `[8]`  | s16 / 10 | °C |
| `expresso_ii_two`     | `[10]` | u16 / 10 | °C |
| `expresso_ii_tkw`     | `[12]` | u16 / 10 | °C |
| `expresso_ii_dfl_tw`  | `[14]` | u16 / 10 | l/min |
| `expresso_ii_tsp`     | `[16]` | u16 / 10 | °C |
| `expresso_ii_two_s`   | `[20]` | u16 / 10 | °C |
| `expresso_ii_dfl_hz1` | `[22]` | u16 / 10 | l/min |
| `expresso_ii_dfl_hz2` | `[24]` | u16 / 10 | l/min |
| `expresso_ii_tsp_s`   | `[26]` | u16 / 10 | °C |
| `expresso_ii_pk`      | `[34]` | u8       | % |
| `expresso_ii_phk`     | `[35]` | u8       | — |
| `expresso_ii_p_sp`    | `[41]` | u8       | % |
| `expresso_ii_timestamp`  | `payload[0..3]` | text | — |
| `expresso_ii_fw_version` | `FD 05 AA 14`   | text | — |

Notes on individual registers:

- **`ta`** looks like the damped outside temperature the controller uses for
  its heating curve rather than the raw sensor. It can sit unchanged for
  hours. Name it accordingly in your config so the flat line is not mistaken
  for a broken sensor.
- **`dfl_hz1` / `dfl_hz2`** are two measuring points on the same heating water
  circuit and track each other almost exactly. Which one is flow and which is
  return has not been established, so they are numbered rather than named.
- **`pk`** is the boiler pump, not the boiler's modulation level. Comfort
  names the same group of three bytes `p_hk1` / `p_hk2` / `p_kes` and labels
  status bit 2 `PK`; Espresso and Palletti carry the same triple at
  `[34]`..`[36]`.
- **`pk`, `phk`, `p_sp`** are plain u8 percentages, **not** scaled by 1/10.
  `p_sp` drives the heat exchanger and closely follows `dfl_hz1`/`dfl_hz2`.

### Raw registers

Everything else is published as `expresso_ii_raw_<offset>`, deliberately
without a unit:

`[18]` `[28]` `[30]` `[32]` `[36]` `[38]` `[42]` `[44]` `[46]` `[48]`

All are read as u16 / 10 **except `[32]`, which is signed** — read as u16 it
shows values around 6550 instead of small negatives.

To map one of these, compare it against the controller display and use the
`expresso_ii_timestamp` text sensor as the time reference: controller clocks
drift, and each device on the bus keeps its own. Once a register is confirmed,
move it in `expresso_ii.cpp` from the raw loop up to the named values and add
a `kind` in `sensor/__init__.py`.

### Not available on the bus

Some values shown on the controller display are computed inside the device and
never transmitted. Heat meter readings (domestic hot water, circulation) fall
into this category — no register in the frame moves when hot water is drawn,
so they cannot be recovered by listening. The same applies to the Aqua's
maximum collector temperature.

Note that the Aqua `sol` register (instantaneous solar power) reads 0 on all
known installations.
