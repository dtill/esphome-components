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
    std::string current_message = this->parent_->get_last_rx_data_str();
    if (!current_message.empty() && current_message != this->last_message_) {
      this->new_data_available_ = true;
      this->last_message_ = current_message;
      ESP_LOGD("GDoorBusMessage", "New bus message: %s", current_message.c_str());
    }
    if (this->new_data_available_) {
      publish_state(this->last_message_.c_str());  // Publish the new data
      ESP_LOGD("GDoorBusMessage", "Published updated bus message: %s", this->last_message_.c_str());
      this->new_data_available_ = false;
      publish_state("BUS_IDLE");
      ESP_LOGD("GDoorBusMessage", "Bus message sensor switched to BUS_IDLE.");
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
