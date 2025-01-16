#pragma once
#include "esphome/core/component.h"
#include "esphome/core/pins.h"
#include "gdoor.h"

namespace esphome {
namespace gdoor_esphome {

class GdoorComponent : public Component {
 public:
  // Methods for setting the pins and sensitivity.
  void set_tx_pin(GPIOPin *tx_pin);
  void set_tx_en_pin(GPIOPin *tx_en_pin);
  void set_rx_pin(GPIOPin *rx_pin);
  void set_rx_sens(float rx_sens);

  void setup() override;
  void loop() override;

  void set_last_rx_data(GDOOR_DATA* &data) { this->last_rx_data_ = data; }
  GDOOR_DATA* get_last_rx_data() { return this->last_rx_data_; }

  GPIOPin* tx_pin() const { return tx_pin_; }
  GPIOPin* tx_en_pin() const { return tx_en_pin_; }
  GPIOPin* rx_pin() const { return rx_pin_; }

 protected:
  GPIOPin *tx_pin_{nullptr};
  GPIOPin *tx_en_pin_{nullptr};
  GPIOPin *rx_pin_{nullptr};
  float rx_sens_{-1};
  GDOOR_DATA* last_rx_data_{nullptr};
};

}  // namespace gdoor_esphome
}  // namespace esphome
