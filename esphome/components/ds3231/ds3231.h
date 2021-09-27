#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/ds1307/ds1307.h"

namespace esphome {
namespace ds3231 {

enum DS3231AlarmType {
  ALARM_1_EVERY_SECOND = 0x0F,
  ALARM_1_EVERY_SECOND_WITH_INTERUPT = 0x4F,
  ALARM_1_MATCH_SECOND = 0x0E,
  ALARM_1_MATCH_SECOND_WITH_INTERUPT = 0x4E,
  ALARM_1_MATCH_MINUTE_SECOND = 0x0C,
  ALARM_1_MATCH_MINUTE_SECOND_WITH_INTERUPT = 0x4C,
  ALARM_1_MATCH_HOUR_MINUTE_SECOND = 0x08,
  ALARM_1_MATCH_HOUR_MINUTE_SECOND_WITH_INTERUPT = 0x48,
  ALARM_1_MATCH_DAY_OF_MONTH_HOUR_MINUTE_SECOND = 0x00,
  ALARM_1_MATCH_DAY_OF_MONTH_HOUR_MINUTE_SECOND_WITH_INTERUPT = 0x40,
  ALARM_1_MATCH_DAY_OF_WEEK_HOUR_MINUTE_SECOND = 0x10,
  ALARM_1_MATCH_DAY_OF_WEEK_HOUR_MINUTE_SECOND_WITH_INTERUPT = 0x50,
  ALARM_2_EVERY_MINUTE = 0x8E,
  ALARM_2_EVERY_MINUTE_WITH_INTERUPT = 0xCE,
  ALARM_2_MATCH_MINUTE = 0x8C,
  ALARM_2_MATCH_MINUTE_WITH_INTERUPT = 0xCC,
  ALARM_2_MATCH_HOUR_MINUTE = 0x88,
  ALARM_2_MATCH_HOUR_MINUTE_WITH_INTERUPT = 0xC8,
  ALARM_2_MATCH_DAY_OF_MONTH_HOUR_MINUTE = 0x80,
  ALARM_2_MATCH_DAY_OF_MONTH_HOUR_MINUTE_WITH_INTERUPT = 0xC0,
  ALARM_2_MATCH_DAY_OF_WEEK_HOUR_MINUTE = 0x90,
  ALARM_2_MATCH_DAY_OF_WEEK_HOUR_MINUTE_WITH_INTERUPT = 0xD0,
};

enum DS3231SquareWaveMode {
  SQUARE_WAVE_1HZ = 0x00,
  SQUARE_WAVE_1024HZ = 0x01,
  SQUARE_WAVE_4096HZ = 0x02,
  SQUARE_WAVE_8192HZ = 0x03,
  ALARM_INTERUPT = 0x04,
};

enum DS3231AlarmNumber {
  ALARM_1,
  ALARM_2,
};

class DS3231Component : public ds1307::DS1307Component {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void read_time();
  void write_time();

 protected:
  bool read_rtc_();
  bool write_rtc_();
  union DS3231Reg {
    struct {
      uint8_t second : 4;
      uint8_t second_10 : 3;
      bool ch : 1;

      uint8_t minute : 4;
      uint8_t minute_10 : 3;
      uint8_t unused_1 : 1;

      uint8_t hour : 4;
      uint8_t hour_10 : 2;
      uint8_t unused_2 : 2;

      uint8_t weekday : 3;
      uint8_t unused_3 : 5;

      uint8_t day : 4;
      uint8_t day_10 : 2;
      uint8_t unused_4 : 2;

      uint8_t month : 4;
      uint8_t month_10 : 1;
      uint8_t unused_5 : 3;

      uint8_t year : 4;
      uint8_t year_10 : 4;

      uint8_t rs : 2;
      uint8_t unused_6 : 2;
      bool sqwe : 1;
      uint8_t unused_7 : 2;
      bool out : 1;
    } reg;
    mutable uint8_t raw[sizeof(reg)];
  } ds3231_;

  union DS3231Reg {
    union DS3231Rtc {
      struct {
        uint8_t second : 4;
        uint8_t second_10 : 3;
        uint8_t unused_1 : 1;

        uint8_t minute : 4;
        uint8_t minute_10 : 3;
        uint8_t unused_2 : 1;

        uint8_t hour : 4;
        uint8_t hour_10 : 2;
        uint8_t unused_3 : 2;

        uint8_t weekday : 3;
        uint8_t unused_4 : 5;

        uint8_t day : 4;
        uint8_t day_10 : 2;
        uint8_t unused_5 : 2;

        uint8_t month : 4;
        uint8_t month_10 : 1;
        uint8_t unused_6 : 2;
        uint8_t cent : 1;

        uint8_t year : 4;
        uint8_t year_10 : 4;
      } reg;
      mutable uint8_t raw[sizeof(reg)];
    } rtc;
    union DS3231Alrm1 {
      struct {
        uint8_t second : 4;
        uint8_t second_10 : 3;
        bool m1 : 1;

        uint8_t minute : 4;
        uint8_t minute_10 : 3;
        bool m2 : 1;

        uint8_t hour : 4;
        uint8_t hour_10 : 2;
        uint8_t unused : 1;
        bool m3 : 1;

        uint8_t day : 4;
        uint8_t day_10 : 2;
        uint8_t day_mode : 1;
        bool m4 : 1;
      } reg;
      mutable uint8_t raw[sizeof(reg)];
    } alrm_1;
    union DS3231Alrm2 {
      struct {
        uint8_t minute : 4;
        uint8_t minute_10 : 3;
        bool m2 : 1;

        uint8_t hour : 4;
        uint8_t hour_10 : 2;
        uint8_t unused : 1;
        bool m3 : 1;

        uint8_t day : 4;
        uint8_t day_10 : 2;
        uint8_t day_mode : 1;
        bool m4 : 1;
      } reg;
      mutable uint8_t raw[sizeof(reg)];
    } alrm_2;
    union DS3231Ctrl {
      struct {
        bool alrm_1_int : 1;
        bool alrm_2_int : 1;
        bool int_ctrl : 1;
        uint8_t rs : 2;
        bool conv_tmp : 1;
        bool bat_sqw : 1;
        bool osc_dis : 1;
      } reg;
      mutable uint8_t raw[sizeof(reg)];
    } ctrl;
    union DS3231Stat {
      struct {
        bool alrm_1_act : 1;
        bool alrm_2_act : 1;
        bool busy : 1;
        bool en32khz : 1;
        uint8_t unused : 3;
        bool osc_stop : 1;
      } reg;
      mutable uint8_t raw[sizeof(reg)];
    } stat;
  } ds3231_;
};

}  // namespace ds3231
}  // namespace esphome
