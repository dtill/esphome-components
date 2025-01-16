#include "esphome/core/log.h"
#include "gdoor_bus_message.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.bus_message";

void GDoorBusMessage::setup() {
  ESP_LOGI(TAG, "Setting up GDoorBusMessage text_sensor");
  if (this->parent_ != nullptr) {
    this->parent_->setup();
  }
  publish_state("BUS_IDLE");
}

void GDoorBusMessage::loop() {
  if (this->parent_ != nullptr) {
    uint32_t parent_timestamp = this->parent_->get_last_bus_update();
    if (parent_timestamp != this->last_bus_update_) {
      std::string current_message = this->parent_->get_last_rx_data_str();
      publish_state(current_message.c_str());
      ESP_LOGVV("GDoorBusMessage", "Published bus message: %s", current_message.c_str());
      publish_state("BUS_IDLE");
      ESP_LOGVV("GDoorBusMessage", "Switched to BUS_IDLE.");
      this->last_bus_update_ = parent_timestamp;
    }
  } else {
    ESP_LOGW("GDoorBusMessage", "Parent component is null!");
  }
}

void GDoorBusMessage::dump_config() {
  ESP_LOGCONFIG(TAG, "GDoor Bus Message text sensor");
}

}  // namespace gdoor_esphome
}  // namespace esphome
