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
#include "gdoor_tx.h"
#include "gdoor_rx.h"
#include "gdoor_utils.h"
#include "esphome/core/log.h"

static const char *TAG = "gdoor_esphome.gdoor_tx";

// ---------------------------------------------------------------------------
// configurable timing constants (60 kHz carrier)
// ---------------------------------------------------------------------------
constexpr uint32_t CARRIER_HZ     = 60000;
constexpr uint32_t HALF_WAVE_US   = 1000000UL / CARRIER_HZ;  // ≈ 17 µs
constexpr uint16_t START_PULSES   = 60;                      // 60 * 17 µs ≈ 1 ms
constexpr uint16_t ONE_PULSES     = 12;                      // 12 * 17 µs ≈ 0.2 ms
constexpr uint16_t ZERO_PULSES    = 32;                      // 32 * 17 µs ≈ 0.55 ms
constexpr uint32_t PAUSE_US       = 10 * HALF_WAVE_US;       // ≈ 0.17 ms gap

// ---------------------------------------------------------------------------
// local TX state
// ---------------------------------------------------------------------------
namespace GDOOR_TX {
  enum TX_STATE : uint8_t { IDLE = 0, PULSE, GAP };

  struct TxContext {
    uint16_t bit_table[MAX_WORDLEN * 9 + 1];   // start + data + CRC
    uint16_t total_bits      = 0;
    uint16_t index           = 0;
    uint32_t deadline_us     = 0;
    TX_STATE state           = IDLE;
  } ctx;

  static uint8_t PIN_TX      = 0;     // outputs 60 kHz carrier (LEDC ch0)
  static uint8_t PIN_TX_EN   = 0;     // high = connect driver to bus

  // -------------------------------------------------------------------------
  // helpers
  // -------------------------------------------------------------------------
  static inline uint16_t bit2pulses(bool bit) {
    return bit ? ONE_PULSES : ZERO_PULSES;
  }

  static inline uint16_t byte2word(uint8_t byte) {
    uint16_t w = byte & 0xFF;
    if (GDOOR_UTILS::parity_odd(byte)) w |= 0x100;  // add odd parity (9th bit)
    return w;
  }

  // -------------------------------------------------------------------------
  // loop-driven state machine (call from your main loop)
  // -------------------------------------------------------------------------
  void loop() {
    if (ctx.state == IDLE) return;

    // wait until current phase is over
    if ((int32_t)(micros() - ctx.deadline_us) < 0) return;

    switch (ctx.state) {
      case PULSE: {               // finished sending carrier burst
        ledcWrite(0, 0);          // carrier off
        ctx.state       = GAP;
        ctx.deadline_us = micros() + PAUSE_US;
        break;
      }
      case GAP: {                 // finished inter-bit gap
        if (ctx.index >= ctx.total_bits) {   // frame done
          digitalWrite(PIN_TX_EN, LOW);      // disconnect driver
          ctx.state = IDLE;
          GDOOR_RX::enable();                // re-enable RX
          ESP_LOGV(TAG, "TX finished");
          break;
        }
        // prepare next carrier burst
        uint16_t pulses = ctx.bit_table[ctx.index++];
        ctx.deadline_us = micros() + (uint32_t)pulses * HALF_WAVE_US;
        ledcWrite(0, 128);                   // 50 % duty ⇒ carrier on
        ctx.state = PULSE;
        break;
      }
      default: break;
    }
  }

  // -------------------------------------------------------------------------
  // public API
  // -------------------------------------------------------------------------
  void setup(uint8_t txpin, uint8_t txenpin) {
    PIN_TX    = txpin;
    PIN_TX_EN = txenpin;

    pinMode(PIN_TX_EN, OUTPUT);
    digitalWrite(PIN_TX_EN, LOW);

    // 60 kHz carrier, 8-bit resolution (channel 0)
    ledcAttach(PIN_TX, CARRIER_HZ, 8);
    ledcWrite(0, 0);               // off by default

    ctx.state = IDLE;
  }

  void send(uint8_t *data, uint16_t len) {
    if (ctx.state != IDLE || len >= MAX_WORDLEN) return;

    // ------------------------------------------------------ build bit table --
    ctx.index      = 0;
    ctx.total_bits = 0;
    ctx.bit_table[ctx.total_bits++] = START_PULSES;

    for (uint16_t i = 0; i < len; ++i) {
      uint16_t word = byte2word(data[i]);
      for (uint8_t b = 0; b < 9; ++b) {
        ctx.bit_table[ctx.total_bits++] = bit2pulses(word & (1 << b));
      }
    }
    uint8_t crc = GDOOR_UTILS::crc(data, len);
    uint16_t word = byte2word(crc);
    for (uint8_t b = 0; b < 9; ++b)
      ctx.bit_table[ctx.total_bits++] = bit2pulses(word & (1 << b));

    // ------------------------------------------------------------ go live --
    GDOOR_RX::disable();           // avoid self-echo
    digitalWrite(PIN_TX_EN, HIGH); // connect driver

    ledcWrite(0, 128);             // first carrier burst
    ctx.deadline_us = micros() + (uint32_t)START_PULSES * HALF_WAVE_US;
    ctx.state       = PULSE;

    ESP_LOGV(TAG, "TX started, %u bits", ctx.total_bits - 1); // -1 = start
  }

  void send(String hex) {
    hex.toUpperCase();
    uint8_t buf[MAX_WORDLEN];
    uint16_t n = 0;

    for (uint16_t i = 0; i + 1 < hex.length() && n < MAX_WORDLEN; i += 2) {
      int hi = strchr("0123456789ABCDEF", hex[i])  - "0123456789ABCDEF";
      int lo = strchr("0123456789ABCDEF", hex[i+1]) - "0123456789ABCDEF";
      if (hi < 0 || lo < 0) break;
      buf[n++] = (hi << 4) | lo;
    }
    if (n) send(buf, n);
  }

  bool busy() { return ctx.state != IDLE; }

} // namespace
