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
        ext_interrupt_fired_flag = true; // Set flag for debug output

        rx_state |= (uint16_t)FLAG_RX_ACTIVE;
        isr_cnt = isr_cnt + 1;

        // Let's try to start the timers here explicitly
        timerStart(timer_bit_received);
        timerStart(timer_bitstream_received);
        // And also restart the count to 0
        timerWrite(timer_bit_received, 0);
        timerWrite(timer_bitstream_received, 0);
    }

    void ARDUINO_ISR_ATTR isr_timer_bit_received() {
        bit_timer_fired_flag = true; // Set flag for debug output

        if (bitcounter >= MAX_WORDLEN*9) {
            bitcounter = 0;
        }
        counts[bitcounter] = isr_cnt;

        isr_cnt = 0;
        bitcounter = bitcounter + 1;
        timerStop(timer_bit_received);
    }

    void ARDUINO_ISR_ATTR isr_timer_bitstream_received() {
        rx_state &= (uint16_t)~FLAG_RX_ACTIVE;
        rx_state |= (uint16_t)FLAG_BITSTREAM_RECEIVED;
        timerStop(timer_bitstream_received);
        timerStop(timer_bit_received);
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
        pinMode(pin_rx, INPUT_PULLUP); // Using INPUT_PULLUP is often safer

        ESP_LOGD(TAG, "GDoor RX setup on pin: %d", rxpin);

        retval.len = 0;
        retval.valid = 0;

        constexpr uint32_t ALARM_US_RX = (20 * 1000000) / TIMER_FREQ_RX;
        constexpr uint32_t ALARM_US_STREAM = (6 * STARTBIT_MIN_LEN * 1000000) / TIMER_FREQ_RX;
        ESP_LOGD(TAG, "Timer RX Alarm value: %d us", ALARM_US_RX);
        ESP_LOGD(TAG, "Timer Stream Alarm value: %d us", ALARM_US_STREAM);

        timer_bit_received = timerBegin(TIMER_FREQ_RX);
        timerAttachInterrupt(timer_bit_received, &isr_timer_bit_received);
        timerAlarm(timer_bit_received, ALARM_US_RX, false, 0);

        timer_bitstream_received = timerBegin(TIMER_FREQ_RX);
        timerAttachInterrupt(timer_bitstream_received, &isr_timer_bitstream_received);
        timerAlarm(timer_bitstream_received, ALARM_US_STREAM, false, 0);

        enable();

        // Timers are configured but will be started by the first external interrupt.
        ESP_LOGI(TAG, "GDoor RX setup complete. Waiting for bus activity.");
    }

    void loop() {
        if (ext_interrupt_fired_flag) {
            ESP_LOGD(TAG, ">>> Ext Interrupt Fired!");
            ext_interrupt_fired_flag = false;
        }

        if (bit_timer_fired_flag) {
            ESP_LOGD(TAG, "======> Bit Timer Fired! (isr_timer_bit_received was called)");
            bit_timer_fired_flag = false;
        }

        if (rx_state & FLAG_BITSTREAM_RECEIVED) {
            ESP_LOGI(TAG, ">>>>>>>>> Stream Timer Fired! Bit count: %d", bitcounter);

            rx_state &= (uint16_t)~FLAG_BITSTREAM_RECEIVED;
            if (retval.parse(counts, bitcounter)) {
                ESP_LOGI(TAG, "Gira RX was successfully parsed!");
                rx_state |= FLAG_DATA_READY;
            } else {
                ESP_LOGW(TAG, "Gira RX parse failed!");
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