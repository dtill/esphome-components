/* 
 * This file is part of the GDoor distribution (https://github.com/gdoor-org).
 * Copyright (c) 2024 GDoor authors.
 * ... (rest of the license header)
 */
#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"

using esphome::esp_log_printf_;

static const char *TAG = "gdoor_esphome.gdoor_rx";

namespace GDOOR_RX {

    // --- DEBUGGING FLAG ---
    // This flag is set to true within the ISR and checked in the main loop.
    // 'volatile' is crucial to prevent the compiler from optimizing away access to this variable.
    static volatile bool isr_triggered_flag = false;

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
        // --- Set the flag here ---
        // This is a safe operation within an ISR.
        isr_triggered_flag = true;

        // The original logic remains
        rx_state |= (uint16_t)FLAG_RX_ACTIVE;
        isr_cnt = isr_cnt + 1;
        timerRestart(timer_bit_received);
        timerRestart(timer_bitstream_received);
    }

    // ... (rest of the ISRs: isr_timer_bit_received, isr_timer_bitstream_received)
    void ARDUINO_ISR_ATTR isr_timer_bit_received() {
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
        pinMode(pin_rx, INPUT);

        retval.len = 0;
        retval.valid = 0;

        constexpr uint32_t ALARM_US_RX = (20 * 1000000) / TIMER_FREQ_RX;
        constexpr uint32_t ALARM_US_STREAM = (6 * STARTBIT_MIN_LEN * 1000000) / TIMER_FREQ_RX;

        timer_bit_received = timerBegin(TIMER_FREQ_RX);
        timerAttachInterrupt(timer_bit_received, &isr_timer_bit_received);
        timerAlarm(timer_bit_received, ALARM_US_RX, /*autoreload=*/false, 0);

        timer_bitstream_received = timerBegin(TIMER_FREQ_RX);
        timerAttachInterrupt(timer_bitstream_received, &isr_timer_bitstream_received);
        timerAlarm(timer_bitstream_received, ALARM_US_STREAM, /*autoreload=*/false, 0);

        enable();

        timerStop(timer_bit_received);
        timerStop(timer_bitstream_received);
    }

    void loop() {
        // --- DEBUGGING CHECK ---
        // Check if the ISR has been triggered.
        if (isr_triggered_flag) {
            // Print a debug message from the safe context of the main loop.
            ESP_LOGD(TAG, "*** External interrupt triggered! (isr_extint_rx fired) ***");
            // Reset the flag so we can detect the next event.
            isr_triggered_flag = false;
        }

        // The original logic remains
        if (rx_state & FLAG_BITSTREAM_RECEIVED) {
            rx_state &= (uint16_t)~FLAG_BITSTREAM_RECEIVED;
            ESP_LOGVV(TAG, "Gira RX done");
            if (retval.parse(counts, bitcounter)) {
                ESP_LOGVV(TAG, "Gira RX was successfully parsed");
                rx_state |= FLAG_DATA_READY;
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