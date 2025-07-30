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

  // Sampling frequency and period
  static constexpr uint32_t SAMPLE_FREQ_HZ   = 120000;               // 120 kHz
  static constexpr uint32_t SAMPLE_PERIOD_US = 1000000 / SAMPLE_FREQ_HZ; // ~8 µs per sample

  // High-speed timer at 1MHz base
  static hw_timer_t *gdoor_timer = nullptr;

  // Reception buffers and state
  uint16_t counts[MAX_WORDLEN * 9];  // Pulse counts per bit
  uint16_t isr_cnt = 0;              // Pulses counted within current bit
  uint8_t  bitcounter = 0;           // Number of bits received so far
  uint16_t rx_state = 0;             // State flags
  GDOOR_DATA retval;                 // Parsed data
  uint8_t pin_rx = 0;                // Input pin

  // External interrupt: triggered on falling edge of 60kHz pulse
  void IRAM_ATTR isr_extint_rx() {
    rx_state |= FLAG_RX_ACTIVE;
    isr_cnt++;
  }

  // Timer ISR: called every SAMPLE_PERIOD_US µs
  void IRAM_ATTR gdoor_timer_isr() {
    static bool prev_level = true;
    bool cur_level = digitalRead(pin_rx);
    // detect rising edge in sampled signal
    if (!prev_level && cur_level && (rx_state & FLAG_RX_ACTIVE)) {
      isr_cnt++;
    }
    prev_level = cur_level;

    // simple timeout logic to detect end of a burst
    static uint32_t idle_ticks = 0;
    idle_ticks++;
    if (idle_ticks * SAMPLE_PERIOD_US >= (SAMPLE_PERIOD_US * 10)) {
      // End of bit train detected (~83µs without pulses)
      counts[bitcounter++] = isr_cnt;
      isr_cnt = 0;
      idle_ticks = 0;

      if (bitcounter >= MAX_WORDLEN * 9) {
        rx_state |= FLAG_BITSTREAM_RECEIVED;
        rx_state &= ~FLAG_RX_ACTIVE;
        timerStop(gdoor_timer);
      }
    }
  }

  // Reset internal counters
  void reset() {
    bitcounter = 0;
    isr_cnt = 0;
    rx_state = 0;
  }

  // Enable RX: attach ext interrupt and start timer
  void enable() {
    reset();
    attachInterrupt(pin_rx, isr_extint_rx, FALLING);
    if (gdoor_timer) {
      timerAlarm(gdoor_timer, SAMPLE_PERIOD_US, true, 0);
      timerStart(gdoor_timer);
    }
  }

  // Disable RX: detach interrupt and stop timer
  void disable() {
    detachInterrupt(pin_rx);
    if (gdoor_timer) timerStop(gdoor_timer);
  }

  // Setup function: configure pin, timer, and interrupts
  void setup(uint8_t rxpin) {
    pin_rx = rxpin;
    pinMode(pin_rx, INPUT_PULLUP);
    retval.len = 0;
    retval.valid = 0;

    // initialize high-frequency timer at 1MHz base for µs precision
    gdoor_timer = timerBegin(1000000);
    timerAttachInterrupt(gdoor_timer, &gdoor_timer_isr);
    timerAlarm(gdoor_timer, SAMPLE_PERIOD_US, true, 0);
    timerStop(gdoor_timer);

    enable();
  }

  // Loop: called from main loop, handle completed bitstream
  void loop() {
    if (rx_state & FLAG_BITSTREAM_RECEIVED) {
      rx_state &= ~FLAG_BITSTREAM_RECEIVED;
      ESP_LOGVV(TAG, "Gira RX done, bits=%u", bitcounter);
      if (retval.parse(counts, bitcounter)) {
        ESP_LOGVV(TAG, "Gira RX parsed, len=%u", retval.len);
        rx_state |= FLAG_DATA_READY;
      }
      reset();
      // Timer remains running for next reception
    }
  }

  // Read parsed data if available
  GDOOR_DATA* read() {
    if (rx_state & FLAG_DATA_READY) {
      rx_state &= ~FLAG_DATA_READY;
      return &retval;
    }
    return nullptr;
  }

}  // namespace GDOOR_RX
