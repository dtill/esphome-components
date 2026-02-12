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

  // Values:
  int16_t ti_s = s16(0);
  int16_t ti2_s = s16(2);
  int16_t tv_s = s16(4);
  int16_t tv2_s = s16(6);
  int16_t tw_s = s16(8);
  int16_t tp_s = s16(10);

  uint16_t stat = u16(12);
  uint32_t bst = u32(14);
  uint32_t kst = u32(22);
  uint16_t err = u16(30);
  uint8_t sens = u8(34);

  uint8_t ba1 = u8(35);
  uint8_t niv1 = u8(36);
  uint8_t ba2 = u8(37);
  uint8_t niv2 = u8(38);
  uint8_t p_hk1 = u8(39);
  uint8_t p_hk2 = u8(40);
  uint8_t p_kes = u8(41);

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
