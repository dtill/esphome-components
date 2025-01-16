# gdoor ESPHome Component
An esphome component for the [gdoor](https://gdoor-org.github.io/) GIRA TKS-Bus-Adapter.
Based on @nholloh's [gdoor esphome-component](https://github.com/nholloh/gdoor-esphome) who was the first to make the [gdoor firmware](https://github.com/gdoor-org/gdoor) work in ESPHome
(see more details in : [gdoor issue #25](https://github.com/gdoor-org/gdoor/issues/25)).

Supported Features: Read Bus State

TODO: Write to TKS-Bus.

Tested hardware combination: Gira Wohnungsstation AP (1250 015) + gdoor Adapter 3.1-1 + ESP32 D1 Mini.

[Example YAML](../../example_gdoor.yaml) configuration:
```yaml
esp32:
  board: esp32dev

external_components:
  - source:
      type: git
      url: https://github.com/dtill/esphome-components
    components: [gdoor]
    refresh: 15m

gdoor:
  id: my_gdoor    # optional set your own id here
  tx_pin: 25      # optional (default 25)
  tx_en_pin: 27   # optional (default 27)
  rx_pin: 22      # optional (default 22)
  rx_sens: 'med'  # optional if rx_pin is 22: 'low', 'med' or 'high' (default 'high')

text_sensor:      # atm returns gdoor formatted string: "action": "BUTTON_RING", "parameters": "0360", "source": "A286FD", "destination": "000000", "type": "OUTDOOR", "busdata": "011011A286FD0360A04A"
  - platform: gdoor
    name: "GDoor Bus Message"
    gdoor_id: my_gdoor
```