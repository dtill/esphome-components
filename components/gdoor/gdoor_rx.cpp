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

    // ISR remains simple: it just records the timestamp of every falling edge.
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

        // Message timeout logic: if a long time has passed since the last pulse, the message is over.
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

            // Define the pause that signifies the end of a bit's pulse train.
            // Original timer was 166µs. We'll use a value in that range.
            const uint32_t PAUSE_BETWEEN_BITS_US = 150;

            for (int i = 0; i <= local_pos; i++) {
                current_pulse_count++; // Count the current pulse

                // Check if we are at the last pulse or if the time to the next pulse is a long pause
                bool is_last_pulse = (i == local_pos);
                if (!is_last_pulse) {
                    uint32_t delta_to_next = local_timings[i+1] - local_timings[i];
                    if (delta_to_next > PAUSE_BETWEEN_BITS_US) {
                        // A long pause was detected, this bit is over.
                        if (bit_idx < (MAX_WORDLEN * 9)) {
                            counts[bit_idx++] = current_pulse_count;
                        }
                        current_pulse_count = 0; // Reset for the next bit.
                    }
                } else {
                    // This is the last pulse of the message, store its count.
                    if (bit_idx < (MAX_WORDLEN * 9)) {
                        counts[bit_idx++] = current_pulse_count;
                    }
                }
            }

            // --- DEBUG OUTPUT and PARSING ---
            char buffer[256];
            int offset = 0;
            offset += snprintf(buffer, sizeof(buffer), "Reconstructed Counts: [");
            for(int i=0; i<bit_idx; i++) {
                if(offset < 240) offset += snprintf(buffer+offset, sizeof(buffer)-offset, "%d, ", counts[i]);
            }
            snprintf(buffer+offset, sizeof(buffer)-offset, "]");
            ESP_LOGD(TAG, "%s", buffer);

            if (retval.parse(counts, bit_idx)) {
                GDOOR_DATA_PROTOCOL busmessage(&retval);
                ESP_LOGI(TAG, "Parse SUCCESS! -> HEX: %s", busmessage.busdata_str().c_str());
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