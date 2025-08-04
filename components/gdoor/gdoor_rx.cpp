#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"

static const char *TAG = "gdoor_esphome.gdoor_rx";

namespace GDOOR_RX {

    static volatile uint32_t edge_timings[MAX_WORDLEN * 20] = {0};
    static volatile int32_t edge_pos = -1;
    uint8_t pin_rx = 0;
    GDOOR_DATA retval;
    uint16_t rx_state = 0;

    void ARDUINO_ISR_ATTR isr_extint_rx() {
        if (edge_pos < ((MAX_WORDLEN * 20) - 1)) {
            edge_pos++;
            edge_timings[edge_pos] = micros();
        }
    }

    void enable() {
        attachInterrupt(pin_rx, isr_extint_rx, FALLING);
        ESP_LOGD(TAG, "RX interrupt enabled on FALLING edge.");
    }
    void disable() {
        detachInterrupt(pin_rx);
        ESP_LOGD(TAG, "RX interrupt disabled.");
    }

    void setup(uint8_t rxpin) {
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT_PULLUP);
        edge_pos = -1;
        ESP_LOGI(TAG, "FINAL RX: Setup on pin %d, using safe emulation of original pulse counting logic.", rxpin);
        enable();
    }

    void loop() {
        if (edge_pos < 0) return;
        if ((micros() - edge_timings[edge_pos]) > 5000) {
            noInterrupts();
            int32_t local_pos = edge_pos;
            uint32_t local_timings[MAX_WORDLEN * 20];
            if (local_pos >= 0) memcpy(local_timings, (void *)edge_timings, (local_pos + 1) * sizeof(uint32_t));
            edge_pos = -1;
            interrupts();

            if (local_pos < 1) return;

            ESP_LOGD(TAG, "Message complete with %d pulses. Reconstructing counts...", local_pos + 1);

            uint16_t counts[MAX_WORDLEN * 9] = {0};
            uint8_t bit_idx = 0;
            uint16_t current_pulse_count = 0;
            const uint32_t PAUSE_BETWEEN_BITS_US = 150;

            for (int i = 0; i <= local_pos; i++) {
                current_pulse_count++;
                bool is_last_pulse = (i == local_pos);
                if (!is_last_pulse) {
                    uint32_t delta_to_next = local_timings[i+1] - local_timings[i];
                    if (delta_to_next > PAUSE_BETWEEN_BITS_US) {
                        if (bit_idx < (MAX_WORDLEN * 9)) counts[bit_idx++] = current_pulse_count;
                        current_pulse_count = 0;
                    }
                } else {
                    if (bit_idx < (MAX_WORDLEN * 9)) counts[bit_idx++] = current_pulse_count;
                }
            }

            char debug_buffer[256];
            int offset = 0;
            offset += snprintf(debug_buffer, sizeof(debug_buffer), "Reconstructed Counts: [");
            for(int i=0; i<bit_idx; i++) {
                if(offset < 240) offset += snprintf(debug_buffer+offset, sizeof(debug_buffer)-offset, "%d, ", counts[i]);
            }
            snprintf(debug_buffer+offset, sizeof(debug_buffer)-offset, "]");
            ESP_LOGD(TAG, "%s", debug_buffer);

            if (retval.parse(counts, bit_idx)) {
                // #######################################################
                // ## HIER IST DIE TATSÄCHLICHE, FUNKTIONIERENDE KORREKTUR ##
                // #######################################################
                char hex_buffer[MAX_WORDLEN * 2 + 1];
                retval.to_hex(hex_buffer);
                ESP_LOGI(TAG, "Parse SUCCESS! -> HEX: %s", hex_buffer);
                // #######################################################

                rx_state |= FLAG_DATA_READY;
            } else {
                ESP_LOGW(TAG, "Parse FAILED! (bit_idx=%d)", bit_idx);
            }
        }
    }

    GDOOR_DATA* read() {
        if (rx_state & FLAG_DATA_READY) {
            rx_state &= (uint16_t)~FLAG_DATA_READY;
            return &retval;
        }
        return NULL;
    }
}