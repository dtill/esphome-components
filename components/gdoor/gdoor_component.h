#pragma once
#include "esphome/core/component.h"
#include "gdoor.h"

namespace esphome {
namespace gdoor_esphome {

class GdoorComponent : public Component {
 public:
  // Methods for setting the pins and sensitivity.
  void set_tx_pin(int tx_pin);
  void set_tx_en_pin(int tx_en_pin);
  void set_rx_pin(int rx_pin);
  void set_rx_sens(float rx_sens);

  void setup() override;
  void loop() override;

  void set_last_rx_data(const GDOOR_DATA* &data) { this->last_rx_data_ = data; }
  GDOOR_DATA* get_last_rx_data() const { return this->last_rx_data_; }

  int tx_pin() const { return tx_pin_; }
  int tx_en_pin() const { return tx_en_pin_; }
  int rx_pin() const { return rx_pin_; }

 protected:
  int tx_pin_{-1};
  int tx_en_pin_{-1};
  int rx_pin_{-1};
  float rx_sens_{-1};
  GDOOR_DATA* last_rx_data_{""};
};

}  // namespace gdoor_esphome
}  // namespace esphome
