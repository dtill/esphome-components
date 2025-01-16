#pragma once
#include "esphome/core/component.h"

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

  void set_last_rx_data(const std::string &data) { this->last_rx_data_ = data; }
  std::string get_last_rx_data() const { return this->last_rx_data_; }

  void set_id(const std::string &id) { this->id_ = id; }
  std::string get_id() const { return this->id_; }

  int tx_pin() const { return tx_pin_; }
  int tx_en_pin() const { return tx_en_pin_; }
  int rx_pin() const { return rx_pin_; }

 protected:
  int tx_pin_{-1};
  int tx_en_pin_{-1};
  int rx_pin_{-1};
  float rx_sens_{-1};
};

}  // namespace gdoor_esphome
}  // namespace esphome
