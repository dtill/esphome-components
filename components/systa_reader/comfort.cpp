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
  float ti_s = s16(0) / 10.0f;
  float ti2_s = s16(2) / 10.0f;
  float tv_s = s16(4) / 10.0f;
  float tv2_s = s16(6) / 10.0f;
  float tw_s = s16(8) / 10.0f;
  float tp_s = s16(10) / 10.0f;

  int stat = u16(12);
  int bst = u32(14);
  int kst = u32(18);
  int err = u16(22);
  int sens = u8(24);

  int ba1 = u8(25);
  int niv1 = u8(26);
  int ba2 = u8(27);
  int niv2 = u8(28);
  int p_hk1 = u8(29);
  int p_hk2 = u8(30);
  int p_kes = u8(31);

  std::string err_text;
  // Erzeugt einen String im Format 0x0001, 0x0123, etc.
  char buf[10];
  sprintf(buf, "0x%04X", err);
  err_text = buf;

  // Publish
  r_.pub_comfort_boiler_err_text(err_text);

  r_.pub_comfort_ti_s(ti_s);
  r_.pub_comfort_ti2_s(ti2_s);
  r_.pub_comfort_tv_s(tv_s);
  r_.pub_comfort_tv2_s(tv2_s);
  r_.pub_comfort_tw_s(tw_s);
  r_.pub_comfort_tp_s(tp_s);

  r_.pub_comfort_bst(bst);
  r_.pub_comfort_kst(kst);
  r_.pub_comfort_sens(sens);
  r_.pub_comfort_ba1(ba1);
  r_.pub_comfort_niv1(niv1);
  r_.pub_comfort_ba2(ba2);
  r_.pub_comfort_niv2(niv2);

  r_.pub_comfort_phk1(p_hk1);
  r_.pub_comfort_phk2(p_hk2);
  r_.pub_comfort_pkes(p_kes);

ESP_LOGI(TAG_COMFORT, "COMFORT: TI_S=%.1f  TI2_S=%.1f TV_S=%.1f  TV2_S=%.1f TW_S=%.1f TP_S=%.1f STAT=0x%04X BST=%u KST=%u ERR=0x%04X SENS=%u",
                    ti_s, ti2_s, tv_s, tv2_s, tw_s, tp_s, stat, bst, kst, err, sens);

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
  "COMFORT STATUS: PHK:%d PHK2:%d PK:%d M1:%d/%d M2:%d/%d ULV:%d PZI:%d B1:%d T:%d LON:%d OT:%d",
  (bool)(stat & 1),    // PHK
  (bool)(stat & 2),    // PHK2
  (bool)(stat & 4),    // PK
  (bool)(stat & 8),    // M1 open
  (bool)(stat & 16),   // M1 close
  (bool)(stat & 32),   // M2 open
  (bool)(stat & 64),   // M2 close
  (bool)(stat & 128),  // ULV
  (bool)(stat & 256),  // PZI
  (bool)(stat & 512),  // B1
  (bool)(stat & 1024), // Taster
  (bool)(stat & 2048), // LON
  (bool)(stat & 4096)  // OT
);

}

void ComfortDecoder::on_fd_version_frame(uint8_t major, uint8_t minor,
                                         uint8_t patch) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%u.%u.%u", major, minor, patch);
  ESP_LOGI(TAG_COMFORT, "COMFORT: Firmware=%s", buf);
  r_.pub_comfort_fw_version(buf);
}

} // namespace systa_reader
} // namespace esphome
