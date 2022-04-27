#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {
namespace powerpal_ble {

namespace espbt = esphome::esp32_ble_tracker;

static const espbt::ESPBTUUID POWERPAL_SERVICE_UUID =
    espbt::ESPBTUUID::from_raw("59DAABCD-12F4-25A6-7D4F-55961DCE4205");
static const espbt::ESPBTUUID POWERPAL_CHARACTERISTIC_PAIRING_CODE_UUID =
    espbt::ESPBTUUID::from_raw("59DA0011-12F4-25A6-7D4F-55961DCE4205");  // indicate, notify, read, write
static const espbt::ESPBTUUID POWERPAL_CHARACTERISTIC_READING_BATCH_SIZE_UUID =
    espbt::ESPBTUUID::from_raw("59DA0013-12F4-25A6-7D4F-55961DCE4205");  // indicate, notify, read, write
static const espbt::ESPBTUUID POWERPAL_CHARACTERISTIC_MEASUREMENT_UUID =
    espbt::ESPBTUUID::from_raw("59DA0001-12F4-25A6-7D4F-55961DCE4205");  // notify, read, write

static const espbt::ESPBTUUID POWERPAL_BATTERY_SERVICE_UUID = espbt::ESPBTUUID::from_uint16(0x180F);
static const espbt::ESPBTUUID POWERPAL_BATTERY_CHARACTERISTIC_UUID = espbt::ESPBTUUID::from_uint16(0x2A19);

// time: '59DA0004-12F4-25A6-7D4F-55961DCE4205',
// ledSensitivity: '59DA0008-12F4-25A6-7D4F-55961DCE4205',
// uuid: '59DA0009-12F4-25A6-7D4F-55961DCE4205',
// serialNumber: '59DA0010-12F4-25A6-7D4F-55961DCE4205',
// pairingCode: '59DA0011-12F4-25A6-7D4F-55961DCE4205',
// measurement: '59DA0001-12F4-25A6-7D4F-55961DCE4205',
// pulse: '59DA0003-12F4-25A6-7D4F-55961DCE4205',
// millisSinceLastPulse: '59DA0012-12F4-25A6-7D4F-55961DCE4205',
// firstRec: '59DA0005-12F4-25A6-7D4F-55961DCE4205',
// measurementAccess: '59DA0002-12F4-25A6-7D4F-55961DCE4205',
// readingBatchSize: '59DA0013-12F4-25A6-7D4F-55961DCE4205',

class Powerpal : public esphome::ble_client::BLEClientNode, public Component {
  // class Powerpal : public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  void setup() override;
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }
  void set_battery(sensor::Sensor *battery) { battery_ = battery; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_energy_sensor(sensor::Sensor *energy_sensor) { energy_sensor_ = energy_sensor; }
  void set_pulses_per_kwh(float pulses_per_kwh) { pulses_per_kwh_ = pulses_per_kwh; }
  void set_pairing_code(uint32_t pairing_code) {
    pairing_code_[0] = (pairing_code & 0x000000FF);
    pairing_code_[1] = (pairing_code & 0x0000FF00) >> 8;
    pairing_code_[2] = (pairing_code & 0x00FF0000) >> 16;
    pairing_code_[3] = (pairing_code & 0xFF000000) >> 24;
  }
  void set_notification_interval(uint8_t reading_batch_size) { reading_batch_size_[0] = reading_batch_size; }

 protected:
  std::string pkt_to_hex_(const uint8_t *data, uint16_t len);
  void decode_(const uint8_t *data, uint16_t length);
  void parse_battery_(const uint8_t *data, uint16_t length);
  void parse_measurement_(const uint8_t *data, uint16_t length);

  bool authenticated_;

  sensor::Sensor *battery_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *energy_sensor_{nullptr};

  uint8_t pairing_code_[4];
  uint8_t reading_batch_size_[4] = {0x01, 0x00, 0x00, 0x00};
  float pulses_per_kwh_;
  uint64_t total_pulses_{0};

  uint16_t pairing_code_char_handle_ = 0x2e;
  uint16_t reading_batch_size_char_handle_ = 0x33;
  uint16_t measurement_char_handle_ = 0x14;

  uint16_t battery_char_handle_ = 0x10;
  uint16_t led_sensitivity_char_handle_ = 0x25;
  uint16_t firmware_char_handle_;
};

}  // namespace powerpal_ble
}  // namespace esphome

#endif