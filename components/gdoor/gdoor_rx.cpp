/* 
 * This file is part of the GDoor distribution (https://github.com/gdoor-org).
 * ... (license header)
 */
#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"

using esphome::esp_log_printf_;

static const char *TAG = "gdoor_esphome.gdoor_rx";

namespace GDOOR_RX {

    // --- DEBUGGING FLAGS ---
    static volatile bool ext_interrupt_fired_flag = false;
    static volatile bool bit_timer_fired_flag = false;

    uint16_t counts[MAX_WORDLEN*9];
    uint16_t isr_cnt = 0;

    uint8_t words[MAX_WORDLEN];
    uint16_t raw[MAX_WORDLEN*9];

    uint16_t rx_state = 0;
    uint8_t bitcounter = 0;

    GDOOR_DATA retval;

    hw_timer_t * timer_bit_received = NULL;
    hw_timer_t * timer_bitstream_received = NULL;

    uint8_t pin_rx = 0;

    void ARDUINO_ISR_ATTR isr_extint_rx() {
        ext_interrupt_fired_flag = true; // Set flag for debugging

        rx_state |= (uint16_t)FLAG_RX_ACTIVE;
        isr_cnt = isr_cnt + 1;

        // NEW STRATEGY: Reset counter and enable the pre-configured alarm.
        // This is often more reliable inside an ISR than timerStart/Restart.
        timerWrite(timer_bit_received, 0);
        timerAlarmEnable(timer_bit_received);

        timerWrite(timer_bitstream_received, 0);
        timerAlarmEnable(timer_bitstream_received);
    }

    void ARDUINO_ISR_ATTR isr_timer_bit_received() {
        bit_timer_fired_flag = true; // Set flag for debugging

        if (bitcounter >= MAX_WORDLEN*9) {
            bitcounter = 0;
        }
        counts[bitcounter] = isr_cnt;

        isr_cnt = 0;
        bitcounter = bitcounter + 1;
        // The alarm is one-shot, so it disables itself. No need to call timerStop().
    }

    void ARDUINO_ISR_ATTR isr_timer_bitstream_received() {
        rx_state &= (uint16_t)~FLAG_RX_ACTIVE;
        rx_state |= (uint16_t)FLAG_BITSTREAM_RECEIVED;
        // The alarm is one-shot. Let's also disable the other timer to be safe.
        timerAlarmDisable(timer_bit_received);
    }

    void reset() {
        bitcounter = 0;
        isr_cnt = 0;
    }

    void enable() {
        reset();
        attachInterrupt(pin_rx, isr_extint_rx, FALLING);
    }

    void disable() {
        reset();
        detachInterrupt(pin_rx);
    }

    void setup(uint8_t rxpin) {
        reset();
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT_PULLUP); // Using INPUT_PULLUP is generally robust.

        ESP_LOGD(TAG, "GDoor RX setup on pin: %d", rxpin);

        retval.len = 0;
        retval.valid = 0;

        constexpr uint32_t ALARM_US_RX = (20 * 1000000) / TIMER_FREQ_RX;
        constexpr uint32_t ALARM_US_STREAM = (6 * STARTBIT_MIN_LEN * 1000000) / TIMER_FREQ_RX;

        // Configure timer for bit-end detection
        timer_bit_received = timerBegin(TIMER_FREQ_RX);
        timerAttachInterrupt(timer_bit_received, &isr_timer_bit_received);
        timerAlarm(timer_bit_received, ALARM_US_RX, false, 0); // false = one-shot alarm
        timerAlarmDisable(timer_bit_received); // Keep it disabled until needed

        // Configure timer for bitstream-end detection
        timer_bitstream_received = timerBegin(TIMER_FREQ_RX);
        timerAttachInterrupt(timer_bitstream_received, &isr_timer_bitstream_received);
        timerAlarm(timer_bitstream_received, ALARM_US_STREAM, false, 0); // false = one-shot alarm
        timerAlarmDisable(timer_bitstream_received); // Keep it disabled until needed

        enable();

        ESP_LOGI(TAG, "GDoor RX setup complete. Timers configured and disabled. Waiting for bus activity.");
    }

    void loop() {
        // Check flags in a safe order
        if (ext_interrupt_fired_flag) {
            ESP_LOGD(TAG, "> Ext Interrupt");
            ext_interrupt_fired_flag = false;
        }

        if (bit_timer_fired_flag) {
            ESP_LOGD(TAG, "--> Bit Timer Fired");
            bit_timer_fired_flag = false;
        }

        if (rx_state & FLAG_BITSTREAM_RECEIVED) {
            char buffer[256];
            int offset = 0;
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, ">>>> Stream Timer Fired! Bit count: %d. Counts: [", bitcounter);
            for (int i = 0; i < bitcounter && i < MAX_WORDLEN*9; i++) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d", counts[i]);
                if (i < bitcounter - 1) {
                    offset += snprintf(buffer + offset, sizeof(buffer) - offset, ", ");
                }
            }
            snprintf(buffer + offset, sizeof(buffer) - offset, "]");
            ESP_LOGI(TAG, "%s", buffer);

            rx_state &= (uint16_t)~FLAG_BITSTREAM_RECEIVED;
            if (retval.parse(counts, bitcounter)) {
                ESP_LOGI(TAG, "Parse SUCCESS!");
                rx_state |= FLAG_DATA_READY;
            } else {
                ESP_LOGW(TAG, "Parse FAILED!");
            }
            reset();
        }
    }

    GDOOR_DATA* read() {
        if(rx_state & FLAG_DATA_READY) {
            rx_state &= (uint16_t)~FLAG_DATA_READY;
            return &retval;
        }
        return NULL;
    }
}