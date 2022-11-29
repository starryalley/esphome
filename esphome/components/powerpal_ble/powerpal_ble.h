#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/defines.h"
#ifdef USE_HTTP_REQUEST
#include "esphome/components/http_request/http_request.h"
#include <ArduinoJson.h>
#endif

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#else
#include <ctime>
#endif

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {
namespace powerpal_ble {

namespace espbt = esphome::esp32_ble_tracker;

struct PowerpalMeasurement {
  uint16_t pulses;
  time_t timestamp;
  uint32_t watt_hours;
  float cost;
  // bool is_peak;
};

static const espbt::ESPBTUUID POWERPAL_SERVICE_UUID =
    espbt::ESPBTUUID::from_raw("59DAABCD-12F4-25A6-7D4F-55961DCE4205");
static const espbt::ESPBTUUID POWERPAL_CHARACTERISTIC_PAIRING_CODE_UUID =
    espbt::ESPBTUUID::from_raw("59DA0011-12F4-25A6-7D4F-55961DCE4205");  // indicate, notify, read, write
static const espbt::ESPBTUUID POWERPAL_CHARACTERISTIC_READING_BATCH_SIZE_UUID =
    espbt::ESPBTUUID::from_raw("59DA0013-12F4-25A6-7D4F-55961DCE4205");  // indicate, notify, read, write
static const espbt::ESPBTUUID POWERPAL_CHARACTERISTIC_MEASUREMENT_UUID =
    espbt::ESPBTUUID::from_raw("59DA0001-12F4-25A6-7D4F-55961DCE4205");  // notify, read, write
static const espbt::ESPBTUUID POWERPAL_CHARACTERISTIC_UUID_UUID =
    espbt::ESPBTUUID::from_raw("59DA0009-12F4-25A6-7D4F-55961DCE4205");  // indicate, notify, read, write
static const espbt::ESPBTUUID POWERPAL_CHARACTERISTIC_SERIAL_UUID =
    espbt::ESPBTUUID::from_raw("59DA0010-12F4-25A6-7D4F-55961DCE4205");  // indicate, notify, read, write

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

static const uint8_t seconds_in_minute = 60;    // seconds
static const float kw_to_w_conversion = 1000.0;    // conversion ratio

// static const std::string POWERPAL_API_URL = "https://readings.powerpal.net/api/v1/meter_reading/";

class Powerpal : public esphome::ble_client::BLEClientNode, public Component {
  // class Powerpal : public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  void setup() override;
  // void loop() override;
  bool gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) override;
  void dump_config() override;
  // float get_setup_priority() const override { return setup_priority::DATA; }
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }
  void set_battery(sensor::Sensor *battery) { battery_ = battery; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
  void set_energy_sensor(sensor::Sensor *energy_sensor) { energy_sensor_ = energy_sensor; }
  void set_daily_energy_sensor(sensor::Sensor *daily_energy_sensor) { daily_energy_sensor_ = daily_energy_sensor; }
#ifdef USE_HTTP_REQUEST
  void set_http_request(http_request::HttpRequestComponent *cloud_uploader) { cloud_uploader_ = cloud_uploader; }
#endif
#ifdef USE_TIME
  void set_time(time::RealTimeClock *time) { time_ = time; }
#endif
  void set_pulses_per_kwh(float pulses_per_kwh) { pulses_per_kwh_ = pulses_per_kwh; }
  void set_pairing_code(uint32_t pairing_code) {
    pairing_code_[0] = (pairing_code & 0x000000FF);
    pairing_code_[1] = (pairing_code & 0x0000FF00) >> 8;
    pairing_code_[2] = (pairing_code & 0x00FF0000) >> 16;
    pairing_code_[3] = (pairing_code & 0xFF000000) >> 24;
  }
  void set_notification_interval(uint8_t reading_batch_size) { reading_batch_size_[0] = reading_batch_size; }
  void set_device_id(std::string powerpal_device_id) { powerpal_device_id_ = powerpal_device_id; }
  void set_apikey(std::string powerpal_apikey) { powerpal_apikey_ = powerpal_apikey; }
  void set_energy_cost(float energy_cost) { energy_cost_ = energy_cost; }

 protected:
  std::string pkt_to_hex_(const uint8_t *data, uint16_t len);
  void decode_(const uint8_t *data, uint16_t length);
  void parse_battery_(const uint8_t *data, uint16_t length);
  void parse_measurement_(const uint8_t *data, uint16_t length);
  std::string uuid_to_device_id_(const uint8_t *data, uint16_t length);
  std::string serial_to_apikey_(const uint8_t *data, uint16_t length);
#ifdef USE_HTTP_REQUEST
  void store_measurement_(uint16_t measurement, time_t timestamp, uint32_t watt_hours, float cost);
  void upload_data_to_cloud_();
#endif

  bool authenticated_;

  sensor::Sensor *battery_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *energy_sensor_{nullptr};
  sensor::Sensor *daily_energy_sensor_{nullptr};
#ifdef USE_HTTP_REQUEST
  http_request::HttpRequestComponent *cloud_uploader_{nullptr};
#endif
#ifdef USE_TIME
  optional<time::RealTimeClock *> time_{};
#endif
  uint16_t day_of_last_measurement_{0};

  uint8_t pairing_code_[4];
  uint8_t reading_batch_size_[4] = {0x01, 0x00, 0x00, 0x00};
  float pulses_per_kwh_;
  float pulse_multiplier_;
  uint64_t daily_pulses_{0};
  uint64_t total_pulses_{0};

  uint8_t stored_measurements_count_{0};
  std::vector<PowerpalMeasurement> stored_measurements_;
  // std::string powerpal_api_root_ = "http://192.168.1.167:1880/powerpal/";
  std::string powerpal_api_root_ = "https://readings.powerpal.net/api/v1/meter_reading/";
  std::string powerpal_device_id_; // = "00002bc3";
  std::string powerpal_apikey_; // = "4a89e298-b17b-43e7-a0c1-fcd1412e98ef";
  float energy_cost_;

  uint16_t pairing_code_char_handle_ = 0x2e;
  uint16_t reading_batch_size_char_handle_ = 0x33;
  uint16_t measurement_char_handle_ = 0x14;

  uint16_t battery_char_handle_ = 0x10;
  uint16_t led_sensitivity_char_handle_ = 0x25;
  uint16_t firmware_char_handle_ = 0x3b;
  uint16_t uuid_char_handle_ = 0x28;
  uint16_t serial_number_char_handle_ = 0x2b;
};

}  // namespace powerpal_ble
}  // namespace esphome

#endif
