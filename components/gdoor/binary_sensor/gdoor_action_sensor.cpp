#include "esphome/core/log.h"
#include "gdoor_action_sensor.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.action_sensor";

void GDoorActionSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up GDoorActionSensor...");
  if (this->parent_ != nullptr) {
    this->parent_->setup();
  }
  this->publish_state(false);
}

void GDoorActionSensor::loop() {
  if (this->parent_ != nullptr) {
    uint32_t parent_timestamp = this->parent_->get_last_bus_update();
    if (parent_timestamp != this->last_bus_update_) {
      std::string current_message = this->parent_->get_last_rx_data_str();
      this->last_bus_update_ = parent_timestamp;
      for (const auto &busdata : this->busdata_list_) {
        if (current_message.find(busdata) != std::string::npos) {
          ESP_LOGVV(TAG, "Matched busdata: %s", busdata.c_str());
          this->publish_state(true);
          this->publish_state(false);
          return;
        }
      }
    }
  } else {
    ESP_LOGW(TAG, "Parent component not set!");
  }
}

void GDoorActionSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "GDoor Action Sensor binary_sensor");
  for (const auto &busdata : this->busdata_list_) {
    ESP_LOGCONFIG(TAG, "  Busdata filter: %s", busdata.c_str());
  }
}

}  // namespace gdoor_esphome
}  // namespace esphome
