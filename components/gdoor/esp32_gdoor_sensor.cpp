#include "esphome/core/log.h"
#include "esp32_gdoor_sensor.h"
#include "gdoor.h"

namespace esphome {
namespace gdoor_esphome {

static const char *TAG = "gdoor_esphome.gdoor";
static const GDOOR_DATA_PROTOCOL gdoor_data_idle(NULL, true);

void GDoorSensor::setup() {
    ESP_LOGI(TAG, "Setting up GDoorSensor");
    
    // TODO: Make sensitivity configurable.
    GDOOR::setRxThreshold(PIN_RX_THRESH, RX_SENS_MED_NUM);

    // TODO: Make RX PIN configurable.
    GDOOR::setup(PIN_TX, PIN_TX_EN, RX_PIN_22_NUM);

    // Publish initial idle state.
    publish_state(gdoor_data_idle.action);
}

void GDoorSensor::dump_config() { 
    ESP_LOGCONFIG(TAG, "GDoor text sensor");
}

void GDoorSensor::loop() {
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