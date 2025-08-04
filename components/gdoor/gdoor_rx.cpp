/* 
 * This file is part of the GDoor distribution (https://github.com/gdoor-org).
 * Copyright (c) 2024 GDoor authors.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "defines.h"
#include "gdoor_rx.h"
#include "gdoor_data.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"

using esphome::esp_log_printf_;

static const char *TAG = "gdoor_esphome.gdoor_rx";

namespace GDOOR_RX {

    uint16_t counts[MAX_WORDLEN*9]; // Received counter values of bitstream, buffer
    uint16_t isr_cnt = 0; // Interrupt Counter (Counting RX edges)

    uint8_t words[MAX_WORDLEN]; //Received words buffer
    uint16_t raw[MAX_WORDLEN*9]; // Received raw counter values of bitstream

    uint16_t rx_state = 0; // State Machine

    uint8_t bitcounter = 0; //Current bit index, in currently active bitstream

    GDOOR_DATA retval;

    hw_timer_t * timer_bit_received = NULL;
    hw_timer_t * timer_bitstream_received = NULL;

    uint8_t pin_rx = 0;

    /*
    * We received a 60kHz pulse, so start timeout timer (for bit and whole bitstream) and increment bit pulse count,
    * so that logic knows how much pulses were in this bit pulse-train.
    */
    void ARDUINO_ISR_ATTR isr_extint_rx() {
        rx_state |= (uint16_t)FLAG_RX_ACTIVE;
        isr_cnt = isr_cnt + 1;
        timerRestart(timer_bit_received); //restart timer to detect bit is over
        timerRestart(timer_bitstream_received); //restart timer to detect bit is over
    }

    /*
    * If this timer fires, the rx 60kHz pulse-train stopped,
    * so we should read out how many pulses we got for this bit (to decide 1 or 0)
    */
    void ARDUINO_ISR_ATTR isr_timer_bit_received() {
        if (bitcounter >= MAX_WORDLEN*9) {
            bitcounter = 0;
        }
        counts[bitcounter] = isr_cnt;

        isr_cnt = 0;
        bitcounter = bitcounter + 1;
        timerStop(timer_bit_received);
    }

    /*
    * If this timer fires, rx bit stream is over
    */
    void ARDUINO_ISR_ATTR isr_timer_bitstream_received() {
        rx_state &= (uint16_t)~FLAG_RX_ACTIVE;
        rx_state |= (uint16_t)FLAG_BITSTREAM_RECEIVED;
        timerStop(timer_bitstream_received);
        timerStop(timer_bit_received);
    }

    /*
    * Internal function set reset all internal values.
    */
    void reset() {
        bitcounter = 0;
        isr_cnt = 0;
    }

    /*
    * Function to enable/disable RX, so that during TX we can disable RX to not get our own message
    */
    void enable() {
        reset();
        attachInterrupt(pin_rx, isr_extint_rx, FALLING);
    }

     /*
    * Function to enable/disable RX, so that during TX we can disable RX to not get our own message
    */
    void disable() {
        reset();
        detachInterrupt(pin_rx);
    }


    /*
    * Function called by user to setup everything needed for GDoor.
    * @param int rxpin Pin number where pulses from bus are received
    */
    void setup(uint8_t rxpin) {
        reset();
        pin_rx = rxpin;
        pinMode(pin_rx, INPUT);

        retval.len = 0;
        retval.valid = 0;

        // Timeout für ein einzelnes Bit: 20 Zyklen bei TIMER_FREQ_RX (z.B. 120kHz)
        // timerAlarm erwartet den Wert in Mikrosekunden.
        constexpr uint32_t ALARM_US_RX = (20 * 1000000) / TIMER_FREQ_RX;

        // Timeout für den gesamten Bitstream: 6 * STARTBIT_MIN_LEN Zyklen bei TIMER_FREQ_RX
        constexpr uint32_t ALARM_US_STREAM = (6 * STARTBIT_MIN_LEN * 1000000) / TIMER_FREQ_RX;

        // Timer zur Erkennung des Bit-Endes konfigurieren
        timer_bit_received = timerBegin(TIMER_FREQ_RX);
        timerAttachInterrupt(timer_bit_received, &isr_timer_bit_received);
        timerAlarm(timer_bit_received, ALARM_US_RX, /*autoreload=*/false, 0);

        // Timer zur Erkennung des Bitstream-Endes konfigurieren
        timer_bitstream_received = timerBegin(TIMER_FREQ_RX);
        timerAttachInterrupt(timer_bitstream_received, &isr_timer_bitstream_received);
        // KORREKTUR: Der korrekte Timer und der korrekte Alarmwert werden hier verwendet.
        timerAlarm(timer_bitstream_received, ALARM_US_STREAM, /*autoreload=*/false, 0);

        // Externen RX-Interrupt aktivieren
        enable();

        // Laut Dokumentation starten die Timer nach timerBegin() automatisch. [2]
        // Wir stoppen sie hier, damit sie erst beim ersten Interrupt-Puls loslaufen.
        timerStop(timer_bit_received);
        timerStop(timer_bitstream_received);
    }

    /*
    * Function called by user, in main loop.
    * Needed for the decoding logic.
    */
    void loop() {
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

    /**
    * User function, called to see if new data is available.
    * @return Data pointer as GDOOR_RX_DATA class or NULL if no data is available
    */
    GDOOR_DATA* read() {
        if(rx_state & FLAG_DATA_READY) {
            rx_state &= (uint16_t)~FLAG_DATA_READY;
            return &retval;
        }
        return NULL;
    }
}