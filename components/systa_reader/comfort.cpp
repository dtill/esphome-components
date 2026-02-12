#include "comfort.h"
#include "esphome/core/log.h"

namespace esphome {
namespace systa_reader {

static const char *const TAG_COMFORT = "systa_reader.comfort";

void ComfortDecoder::on_fc_frame(const std::vector<uint8_t> &frame,
                                 const std::vector<uint8_t> &payload,
                                 const std::string &hex) {
  // Only COMFORT Frames: FC .. 0C 02 ..
  // Note: Comfort might also use 0C 01 (shared with Modula) but user
  // specifically asked for 0C 02 here. We check for 0C 02 here.
  if (frame.size() < 6 || frame[0] != 0xFC || frame[2] != 0x0C ||
      frame[3] != 0x02)
    return;

  // Since enabled_mask filtering routes here, we double check header
  r_.publish_hex_comfort(hex);
  ESP_LOGV(TAG_COMFORT, "COMFORT len=%u, frame.size()=%u, payload.size()=%u",
           frame[1], (unsigned)frame.size(), (unsigned)payload.size());

  // Helper lambdas
  auto u8 = [&](int i) { return read_u8(payload, i); };
  auto s16 = [&](int i) { return read_s16_be(payload, i); };
  auto u32 = [&](int i) { return read_u32_be(payload, i); };
  auto u16 = [&](int i) { return read_u16_be(payload, i); };

  // Mapping from systa-decoder (offsets are payload based)
  // Payload starts at frame[5] (index 4 in systa-decoder array p is payload[3]
  // here? No) sysa-decoder "p" array is 1-based from frame byte 5. payload in
  // this C++ code is vector of bytes starting from frame[4]. Wait,
  // systa_reader.cpp: std::vector<uint8_t> payload(cur_.begin() + 4, cur_.end()
  // - 1); So payload[0] corresponds to frame[4]. frame[0]=FC, frame[1]=len,
  // frame[2]=f2(0C), frame[3]=f3(02). frame[4] is the first byte of payload.

  // systa-decoder logic:
  // p array is created from byte 5 (index 5, 0-based is 6th byte?).
  // systa-decoder:
  //   for (( k=5; k<=${#b}; k++ )); do p+=("${b[$k]}"); done
  //   b is 1-based array of hex strings. b[1] is frame[0] (FC). b[5] is
  //   frame[4]. So p[1] in systa-decoder is frame[4]. So payload[0] in C++ IS
  //   p[1] in systa-decoder. offsets in systa-decoder are 0-based from payload
  //   start? read_s16_be(off) -> idx = off+1 -> p[idx], p[idx+1] So
  //   read_s16_be(4) means p[5], p[6]. Since p[1] is payload[0], p[5] is
  //   payload[4]. So offset 4 in systa-decoder corresponds to payload[4] in
  //   C++.

  // 0C 02 Frame Layout (offsets relative to payload start):
  // 0-3: ?? (systa-decoder says TI-SOLL, TI2-S .. wait)
  // print_frame_comfort_0c02 colors bytes 0-1, 2-3...
  // process_frame_comfort_0c02:
  // ti_s = read_s16_be(4) -> payload[4..5]
  // Wait, Bytes 0-3 seem unused in `process` function?
  // Let's re-read `process_frame_comfort_0c02` in systa-decoder.
  //   local ti_s=$(read_s16_be 4);  ...
  // Seems bytes 0-3 are NOT decoded in `process` function, but colored in
  // `print`. `print_frame_comfort_0c02`:
  //   0|1 -> TI-SOLL ?
  //   2|3 -> TI2-S ?
  // BUT `process` function starts reading at offset 4.
  // Let's look at `systa-decoder` again carefully.
  //   read_s16_be 4 -> p[5], p[6] -> payload[4], payload[5]

  // Confirmed: process_frame starts at offset 4. What is at offset 0?
  // Maybe unused or unknown.

  // Values:
  // Values:
  int16_t ti_s = s16(4);
  int16_t ti2_s = s16(6);
  int16_t tv_s = s16(8);
  int16_t tv2_s = s16(10);
  int16_t tw_s = s16(12);
  int16_t tp_s = s16(14);

  uint16_t stat = u16(16);
  uint32_t bst = u32(18);
  uint32_t kst = u32(22);
  uint16_t err = u16(26);
  uint8_t sens = u8(28);

  uint8_t ba1 = u8(29);
  uint8_t niv1 = u8(30);
  uint8_t ba2 = u8(31);
  uint8_t niv2 = u8(32);
  uint8_t p_hk1 = u8(33);
  uint8_t p_hk2 = u8(34);
  uint8_t p_kes = u8(35);

  // Publish
  r_.pub_comfort_ti_s(ti_s);
  r_.pub_comfort_ti2_s(ti2_s);
  r_.pub_comfort_tv_s(tv_s);
  r_.pub_comfort_tv2_s(tv2_s);
  r_.pub_comfort_tw_s(tw_s);
  r_.pub_comfort_tp_s(tp_s);

  r_.pub_comfort_stat(stat);
  r_.pub_comfort_bst(bst);
  r_.pub_comfort_kst(kst);
  r_.pub_comfort_err(err);
  r_.pub_comfort_sens(sens);

  r_.pub_comfort_ba1(ba1);
  r_.pub_comfort_niv1(niv1);
  r_.pub_comfort_ba2(ba2);
  r_.pub_comfort_niv2(niv2);
  r_.pub_comfort_phk1(p_hk1);
  r_.pub_comfort_phk2(p_hk2);
  r_.pub_comfort_pkes(p_kes);

  // Status Bitfield
  // Bit 0: PHK
  r_.pub_comfort_stat_phk((bool)(stat & 1));
  // Bit 1: PHK2
  r_.pub_comfort_stat_phk2((bool)(stat & 2));
  // Bit 2: PK
  r_.pub_comfort_stat_pk((bool)(stat & 4));
  // Bit 3: M1_open
  r_.pub_comfort_stat_m1_open((bool)(stat & 8));
  // Bit 4: M1_close
  r_.pub_comfort_stat_m1_close((bool)(stat & 16));
  // Bit 5: M2_open
  r_.pub_comfort_stat_m2_open((bool)(stat & 32));
  // Bit 6: M2_close
  r_.pub_comfort_stat_m2_close((bool)(stat & 64));
  // Bit 7: ULV
  r_.pub_comfort_stat_ulv((bool)(stat & 128));
  // Bit 8: PZI
  r_.pub_comfort_stat_pzi((bool)(stat & 256));
  // Bit 9: B1
  r_.pub_comfort_stat_b1((bool)(stat & 512));
  // Bit 10: Taster
  r_.pub_comfort_stat_taster((bool)(stat & 1024));
  // Bit 11: LON
  r_.pub_comfort_stat_lon((bool)(stat & 2048));
  // Bit 12: OT
  r_.pub_comfort_stat_ot((bool)(stat & 4096));

  ESP_LOGI(TAG_COMFORT,
           "COMFORT: TI_S=%d TV_S=%d BST=%u KST=%u STAT=0x%04X ERR=0x%04X",
           ti_s, tv_s, bst, kst, stat, err);
}

} // namespace systa_reader
} // namespace esphome
