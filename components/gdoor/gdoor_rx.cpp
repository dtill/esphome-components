/*
 * This file is part of the GDoor distribution (https://github.com/gdoor-org).
 * Copyright (c) 2024 GDoor authors.
 * ... (license header)
 *
 * This version has been fundamentally rewritten to align with the Arduino Core v3.x
 * timer handling, as seen in ESPHome's native components. It uses a single,
 * free-running timer as a stopwatch and processes time deltas in the main loop
 * to avoid issues with non-ISR-safe function calls.
 */
#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"

using esphome::esp_log_printf_;

static const char *TAG = "gdoor_esphome.gdoor_rx";

namespace GDOOR_RX {

    // --- Data Collection ---
    // Buffer to store time deltas between interrupts (in microseconds)
    static volatile uint32_t delta_buffer[MAX_WORDLEN * 20];
    static volatile int buffer_index = 0;
    // Flag to signal the main loop that a message is ready for processing
    static volatile bool message_ready = false;

    // --- State Variables ---
    GDOOR_DATA retval;
    uint8_t pin_rx = 0;

    // --- Timer and ISR Variables ---
    hw_timer_t *stopwatch = NULL;
    // Stores the timestamp of the last interrupt
    static volatile uint64_t last_interrupt_micros = 0;

    // The GPIO ISR - now extremely simple and fast
    void ARDUINO_ISR_ATTR isr_extint_rx() {
        // Read the current time from our free-running "stopwatch" timer
        const uint64_t now = timerRead(stopwatch);

        // Calculate the time since the last interrupt
        const uint32_t delta = now - last_interrupt_micros;
        last_interrupt_micros = now;

        // Store the delta if there is space in the buffer
        if (buffer_index < (MAX_WORDLEN * 20)) {
            delta_buffer[buffer_index++] = delta;
        }
    }

    void reset() {
        buffer_index = 0;
        message_ready = false;
        // The last_interrupt_micros is reset in the loop
    }

    void enable() {
        reset();
        attachInterrupt(pin_rx, isr_extint_rx, FALLING);
    }

    void disable() {
        detachInterrupt(pin_rx);
        reset();
    }

    void setup(uint8_t rxpin) {
        reset();
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT_PULLUP);

        ESP_LOGD(TAG, "GDoor RX setup on pin: %d", rxpin);
        retval.len = 0;
        retval.valid = 0;

        // Setup the stopwatch: 1MHz frequency means each tick is 1 microsecond.
        stopwatch = timerBegin(1000000); // 1MHz = 1 tick per µs
        // Start the stopwatch immediately. It will run forever.
        timerStart(stopwatch);

        enable();
        ESP_LOGI(TAG, "GDoor RX setup complete using robust delta-time method.");
    }

    // This function reconstructs the original `counts` array from the time deltas
    void process_deltas() {
        uint16_t counts[MAX_WORDLEN*9] = {0};
        uint8_t bit_idx = 0;
        uint16_t pulse_count = 0;

        // Approx. time for one 60kHz pulse cycle is 16.6µs.
        // We allow a generous window (e.g., up to 30µs) to count as a pulse.
        const uint32_t PULSE_MAX_DELTA_US = 30;

        for (int i = 0; i < buffer_index; i++) {
            if (delta_buffer[i] < PULSE_MAX_DELTA_US) {
                // This is a short delta, part of a 60kHz pulse train
                pulse_count++;
            } else {
                // This is a longer delta, representing a gap between bits.
                // Store the collected pulse count for the previous bit.
                if (pulse_count > 0 && bit_idx < MAX_WORDLEN*9) {
                    counts[bit_idx++] = pulse_count;
                }
                // Reset for the next bit
                pulse_count = 1; // The current pulse starts a new bit
            }
        }
        // Store the last collected pulse count
        if (pulse_count > 0 && bit_idx < MAX_WORDLEN*9) {
            counts[bit_idx++] = pulse_count;
        }

        // --- DEBUG OUTPUT ---
        char buffer[256];
        int offset = 0;
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "Reconstructed Counts. Bit count: %d. Counts: [", bit_idx);
        for (int i = 0; i < bit_idx; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%d", counts[i]);
            if (i < bit_idx - 1) offset += snprintf(buffer + offset, sizeof(buffer) - offset, ", ");
        }
        snprintf(buffer + offset, sizeof(buffer) - offset, "]");
        ESP_LOGD(TAG, "%s", buffer);
        // --- END DEBUG ---

        if (retval.parse(counts, bit_idx)) {
            ESP_LOGI(TAG, "Parse SUCCESS!");
            // This flag is currently not used, but kept for API compatibility
            // rx_state |= FLAG_DATA_READY;
        } else {
            ESP_LOGW(TAG, "Parse FAILED!");
        }
    }

    void loop() {
        // Check if there has been any bus activity
        if (buffer_index > 0) {
            // Check for a message timeout. If the last interrupt was too long ago,
            // we assume the message is complete and ready for processing.
            // A value like 5000µs (5ms) is usually a safe bet.
            if ((timerRead(stopwatch) - last_interrupt_micros) > 5000) {
                ESP_LOGD(TAG, "Message timeout detected. Processing %d deltas.", buffer_index);

                // Disable interrupts while we process the buffer to prevent race conditions
                noInterrupts();
                process_deltas();
                reset();
                last_interrupt_micros = timerRead(stopwatch); // Reset timeout timer
                interrupts(); // Re-enable interrupts
            }
        }
    }

    GDOOR_DATA* read() {
        // This function needs to be adapted if you use it, as the FLAG_DATA_READY
        // is not the primary mechanism anymore. For now, it returns NULL.
        // The parsing result is currently only visible in the logs.
        return NULL;
    }
}