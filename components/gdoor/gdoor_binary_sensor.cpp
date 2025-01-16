#include "esphome/core/log.h"
#include "gdoor_binary_sensor.h"
#include "gdoor.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.gdoor";
static const GDOOR_DATA_PROTOCOL gdoor_data_idle(NULL, true);

void GDoorBinarySensor::setup() {
    ESP_LOGI(TAG, "Setting up GDoorBinarySensor");
    GDOOR::setRxThreshold(PIN_RX_THRESH, RX_SENS_MED_NUM);
    GDOOR::setup(PIN_TX, PIN_TX_EN, RX_PIN_22_NUM);

    // Publish initial idle state.
    publish_state(gdoor_data_idle.action);
}

void GDoorBinarySensor::dump_config() {
    ESP_LOGCONFIG(TAG, "GDoor binary sensor");
}

void GDoorBinarySensor::loop() {
    GDOOR::loop();

    GDOOR_DATA* rx_data = GDOOR::read();
    if(rx_data != NULL) {
        ESP_LOGI(TAG, "Received data from bus");
        GDOOR_DATA_PROTOCOL busmessage = GDOOR_DATA_PROTOCOL(rx_data);

        ESP_LOGD(TAG, "Data: ", busmessage);

        // Set sensor state.
        publish_state(busmessage.action);

        ESP_LOGD(TAG, "Sensor state set. Back to idle.");

        // TODO: Set sensor state back to idle.
        publish_state(gdoor_data_idle.action);
    }
}

}  // namespace gdoor_esphome
}  // namespace esphome