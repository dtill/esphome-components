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
#include <esp32-hal-ledc.h>

static const char *TAG = "gdoor_esphome.gdoor_tx";

// ---------------------------------------------------------------------------
// configurable timing constants (58 kHz carrier)
// ---------------------------------------------------------------------------
constexpr uint32_t CARRIER_HZ   = 58000;                    // target carrier
constexpr uint8_t  LEDC_BITS    =  8;                       // 1–14 bit PWM
constexpr uint32_t HALF_WAVE_US = 1000000UL / CARRIER_HZ;   // ≈17.24 µs
constexpr uint16_t START_PULSES = 60;                       // ≈1 ms start burst
constexpr uint16_t ONE_PULSES   = 12;                       // ≈0.21 ms (“1” bit)
constexpr uint16_t ZERO_PULSES  = 32;                       // ≈0.55 ms (“0” bit)
constexpr uint32_t PAUSE_US     = 10 * HALF_WAVE_US;        // ≈0.17 ms gap

namespace GDOOR_TX {

  enum TX_STATE : uint8_t { IDLE = 0, PULSE, GAP };

  struct TxContext {
    uint16_t bit_table[MAX_WORDLEN * 9 + 1];
    uint16_t total_bits = 0;
    uint16_t index      = 0;
    uint32_t deadline_us= 0;
    TX_STATE state      = IDLE;
  } ctx;

  static uint8_t tx_pin_hw = 0;  // GPIO driving the PWM carrier
  static uint8_t tx_en_hw  = 0;  // GPIO to enable the external driver

  // helper to convert one bit → pulse count
  static inline uint16_t bit2pulses(bool bit) {
    return bit ? ONE_PULSES : ZERO_PULSES;
  }
  // helper to pack one byte + odd-parity into a 9-bit word
  static inline uint16_t byte2word(uint8_t b) {
    uint16_t w = b;
    if (GDOOR_UTILS::parity_odd(b)) w |= 0x100;
    return w;
  }

  // must be called from your main loop()
  void loop() {
    if (ctx.state == IDLE) return;
    // wait until the current phase (pulse or gap) is over
    if ((int32_t)(micros() - ctx.deadline_us) < 0) return;

    if (ctx.state == PULSE) {
      // just finished sending the carrier burst
      ledcWrite(tx_pin_hw, 0);
      ctx.state       = GAP;
      ctx.deadline_us = micros() + PAUSE_US;
      return;
    }

    // GAP state
    if (ctx.index >= ctx.total_bits) {
      // entire frame done
      digitalWrite(tx_en_hw, LOW);     // disable driver
      ctx.state = IDLE;
      GDOOR_RX::enable();              // re-enable RX
      ESP_LOGV(TAG, "TX finished");
      return;
    }

    // start next carrier burst
    uint16_t pulses = ctx.bit_table[ctx.index++];
    ledcWrite(tx_pin_hw, 1 << (LEDC_BITS - 1));  // 50% duty
    ctx.deadline_us = micros() + pulses * HALF_WAVE_US;
    ctx.state       = PULSE;
  }

  // ----------------------------------------------------------------------------
  // Initialize TX: must be called once in setup()
  // ----------------------------------------------------------------------------
  void setup(uint8_t txpin, uint8_t txenpin) {
    tx_pin_hw = txpin;
    tx_en_hw  = txenpin;

    pinMode(tx_en_hw, OUTPUT);
    digitalWrite(tx_en_hw, LOW);

    // single-call LEDC: picks a free channel, sets freq & resolution, attaches to pin
    bool attached = ledcAttach(tx_pin_hw, CARRIER_HZ, LEDC_BITS);
    ledcWrite(tx_pin_hw, 0);  // ensure PWM is off

    ESP_LOGCONFIG(TAG, "  TX PWM pin      : GPIO %u", tx_pin_hw);
    ESP_LOGCONFIG(TAG, "  TX EN pin       : GPIO %u", tx_en_hw);
    ESP_LOGCONFIG(TAG, "  Carrier freq    : %u Hz", CARRIER_HZ);
    ESP_LOGCONFIG(TAG, "  Resolution      : %u bits", LEDC_BITS);
    ESP_LOGCONFIG(TAG, "  LEDC attached   : %s", attached ? "yes" : "no");

    ctx.state = IDLE;
  }

  // ----------------------------------------------------------------------------
  // Send exactly the bytes you pass (including CRC), LSB-first + parity
  // ----------------------------------------------------------------------------
  void send(uint8_t *data, uint16_t len) {
    if (ctx.state != IDLE || len >= MAX_WORDLEN) return;

    // build the flat pulse-count table: [start] [bits of data[0]] … [bits of data[len-1]]
    ctx.index      = 0;
    ctx.total_bits = 0;
    ctx.bit_table[ctx.total_bits++] = START_PULSES;
    for (uint16_t i = 0; i < len; ++i) {
      uint16_t word = byte2word(data[i]);
      for (uint8_t b = 0; b < 9; ++b)
        ctx.bit_table[ctx.total_bits++] = bit2pulses(word & (1 << b));
    }

    // optional verbose dump of the table
    #if ESPHOME_LOG_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
    {
      char buf[256];
      int off = snprintf(buf, sizeof(buf), "Burst table (cnt=%u): [", ctx.total_bits);
      for (uint16_t i = 0; i < ctx.total_bits && off + 8 < (int)sizeof(buf); ++i)
        off += snprintf(buf + off, sizeof(buf) - off, "%u,", ctx.bit_table[i]);
      snprintf(buf + off, sizeof(buf) - off, "]");
      ESP_LOGV(TAG, "%s", buf);
    }
    #endif

    // fire the first burst
    GDOOR_RX::disable();              // mute Rx while we TX
    digitalWrite(tx_en_hw, HIGH);     // enable external driver
    ledcWrite(tx_pin_hw, 1 << (LEDC_BITS - 1));  // carrier ON
    ctx.deadline_us = micros() + START_PULSES * HALF_WAVE_US;
    ctx.state       = PULSE;

    ESP_LOGV(TAG, "TX started, %u bits", ctx.total_bits - 1);
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

}  // namespace GDOOR_TX
