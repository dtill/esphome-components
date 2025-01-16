# gdoor ESPHome Component
An esphome component for the [gdoor](https://gdoor-org.github.io/) GIRA TKS-Bus-Adapter.
Based on @nholloh's [gdoor esphome-component](https://github.com/nholloh/gdoor-esphome) who was the first to make the [gdoor firmware](https://github.com/gdoor-org/gdoor) work in ESPHome
(see more details in : [gdoor issue #25](https://github.com/gdoor-org/gdoor/issues/25)).

Supported Features: Read Bus State

TODO: Write to TKS-Bus, add parameters and IO PINs in yaml config.

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
    
text_sensor:
  - platform: gdoor
    name: 'BUS State'
    id: gdoor_bus_state

```