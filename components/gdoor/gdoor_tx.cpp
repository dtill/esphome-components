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
// configurable timing constants (58 kHz carrier)
// ---------------------------------------------------------------------------
constexpr uint32_t CARRIER_HZ     = 58000;
constexpr uint32_t HALF_WAVE_US   = 1000000UL / CARRIER_HZ;  // ≈ 17.24 µs
constexpr uint16_t START_PULSES   = 60;                      // ≈ 1 ms
constexpr uint16_t ONE_PULSES     = 12;                      // ≈ 0.21 ms
constexpr uint16_t ZERO_PULSES    = 32;                      // ≈ 0.55 ms
constexpr uint32_t PAUSE_US       = 10 * HALF_WAVE_US;       // ≈ 0.17 ms gap
constexpr uint8_t  LEDC_BITS      =   8;                     // 1–14 bits resolution

// ---------------------------------------------------------------------------
// local TX state
// ---------------------------------------------------------------------------
namespace GDOOR_TX {
  enum TX_STATE : uint8_t { IDLE = 0, PULSE, GAP };

  struct TxContext {
    uint16_t bit_table[MAX_WORDLEN * 9 + 1];   // start + data (incl CRC you supply)
    uint16_t total_bits      = 0;
    uint16_t index           = 0;
    uint32_t deadline_us     = 0;
    TX_STATE state           = IDLE;
  } ctx;

  static uint8_t tx_pin_hw   = 0;   // outputs the PWM carrier
  static uint8_t tx_en_hw    = 0;   // high = connect driver
  static int     ledc_ch     = -1;  // auto-chosen LEDC channel

  // -------------------------------------------------------------------------
  // simple helpers
  // -------------------------------------------------------------------------
  static inline uint16_t bit2pulses(bool bit) {
    return bit ? ONE_PULSES : ZERO_PULSES;
  }

  static inline uint16_t byte2word(uint8_t byte) {
    uint16_t w = byte;
    if (GDOOR_UTILS::parity_odd(byte)) w |= 0x100;  // add parity as bit 8
    return w;
  }

  // -------------------------------------------------------------------------
  // must be called from your main loop()
  // -------------------------------------------------------------------------
  void loop() {
    if (ctx.state == IDLE) return;
    // wait until the current burst or gap has elapsed
    if ((int32_t)(micros() - ctx.deadline_us) < 0) return;

    if (ctx.state == PULSE) {
      // just finished a carrier burst → start the inter-bit gap
      ledcWrite(ledc_ch, 0);
      ctx.state       = GAP;
      ctx.deadline_us = micros() + PAUSE_US;
      return;
    }

    // GAP state
    if (ctx.index >= ctx.total_bits) {
      // all bits done
      digitalWrite(tx_en_hw, LOW);
      ctx.state = IDLE;
      GDOOR_RX::enable();
      ESP_LOGV(TAG, "TX finished");
      return;
    }
    // start next burst
    uint16_t pulses = ctx.bit_table[ctx.index++];
    ledcWrite(ledc_ch, 1 << (LEDC_BITS - 1));  // 50 % duty
    ctx.deadline_us = micros() + (uint32_t)pulses * HALF_WAVE_US;
    ctx.state       = PULSE;
  }

  // -------------------------------------------------------------------------
  // public API
  // -------------------------------------------------------------------------
  void setup(uint8_t txpin, uint8_t txenpin) {
    tx_pin_hw = txpin;
    tx_en_hw  = txenpin;

    pinMode(tx_en_hw, OUTPUT);
    digitalWrite(tx_en_hw, LOW);

    // Attach and configure the carrier PWM in one call (v3.x API)
    ledc_ch = ledcAttach(tx_pin_hw, CARRIER_HZ, LEDC_BITS);
    ledcWrite(ledc_ch, 0);

    ESP_LOGCONFIG(TAG, "  LEDC channel   : %d", ledc_ch);
    ESP_LOGCONFIG(TAG, "  Carrier freq   : %u Hz", CARRIER_HZ);
    ESP_LOGCONFIG(TAG, "  TX pin         : GPIO %u", tx_pin_hw);
    ESP_LOGCONFIG(TAG, "  TX-EN pin      : GPIO %u", tx_en_hw);

    ctx.state = IDLE;
  }

  void send(uint8_t *data, uint16_t len) {
    if (ctx.state != IDLE || len >= MAX_WORDLEN) return;

    // ----------------------------------------------------------------------
    // 1) build a flat table of pulse-counts: start + each bit of your data
    // ----------------------------------------------------------------------
    ctx.index      = 0;
    ctx.total_bits = 0;
    ctx.bit_table[ctx.total_bits++] = START_PULSES;

    for (uint16_t i = 0; i < len; ++i) {
      uint16_t word = byte2word(data[i]);
      for (uint8_t b = 0; b < 9; ++b)
        ctx.bit_table[ctx.total_bits++] = bit2pulses(word & (1 << b));
    }

    // ----------------------------------------------------------------------
    // 2) optional VERBOSE dump of the table
    // ----------------------------------------------------------------------
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

    // ----------------------------------------------------------------------
    // 3) fire off the first burst
    // ----------------------------------------------------------------------
    GDOOR_RX::disable();
    digitalWrite(tx_en_hw, HIGH);

    ledcWrite(ledc_ch, 1 << (LEDC_BITS - 1));  // 50% duty = carrier on
    ctx.deadline_us = micros() + (uint32_t)START_PULSES * HALF_WAVE_US;
    ctx.state       = PULSE;

    ESP_LOGV(TAG, "TX started, %u bits (LEDC ch=%d)",
             ctx.total_bits - 1, ledc_ch);  // -1 hides the start-burst
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
