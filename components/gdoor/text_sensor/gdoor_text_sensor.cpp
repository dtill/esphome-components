#include "esphome/core/log.h"
#include "gdoor_text_sensor.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.text_sensor";

void GDoorBusMessage::setup() {
  ESP_LOGI(TAG, "Setting up GDoorBusMessage text sensor");
  if (this->parent_ != nullptr) {
    this->parent_->setup();
  }
  publish_state("BUS_IDLE");
}

void GDoorBusMessage::loop() {
  if (this->parent_ != nullptr) {
    if (this->parent_->has_new_data()) {
      std::string current_message = this->parent_->get_last_rx_data_str();
      publish_state(current_message.c_str());
      ESP_LOGVV("GDoorBusMessage", "Published bus message: %s", current_message.c_str());
      //delay(10); // Optional delay
      publish_state("BUS_IDLE");
      ESP_LOGVV("GDoorBusMessage", "Switched to BUS_IDLE.");
      this->parent_->mark_data_as_read();
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
