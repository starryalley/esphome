#include "powerpal_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32

namespace esphome {
namespace powerpal_ble {

static const char *const TAG = "powerpal_ble";

void Powerpal::dump_config() {
  ESP_LOGCONFIG(TAG, "POWERPAL");
  LOG_SENSOR(" ", "Battery", this->battery_);
  LOG_SENSOR(" ", "Power", this->power_sensor_);
  LOG_SENSOR(" ", "Energy", this->energy_sensor_);
}

void Powerpal::setup() { this->authenticated_ = false; }

std::string Powerpal::pkt_to_hex_(const uint8_t *data, uint16_t len) {
  char buf[64];
  memset(buf, 0, 64);
  for (int i = 0; i < len; i++)
    sprintf(&buf[i * 2], "%02x", data[i]);
  std::string ret = buf;
  return ret;
}

void Powerpal::decode_(const uint8_t *data, uint16_t length) {
  ESP_LOGD("powerpal_ble", "DEC(%d): 0x%s", length, this->pkt_to_hex_(data, length).c_str());
}

void Powerpal::parse_battery_(const uint8_t *data, uint16_t length) {
  ESP_LOGD("powerpal_ble", "Battery: DEC(%d): 0x%s", length, this->pkt_to_hex_(data, length).c_str());
  if (length == 1) {
    this->battery_->publish_state(data[0]);
  }
}

void Powerpal::parse_measurement_(const uint8_t *data, uint16_t length) {
  ESP_LOGD("powerpal_ble", "Meaurement: DEC(%d): 0x%s", length, this->pkt_to_hex_(data, length).c_str());
  if (length >= 6) {
    uint32_t unix_time = data[0];
    unix_time += (data[1] << 8);
    unix_time += (data[2] << 16);
    unix_time += (data[3] << 24);

    uint16_t pulses_within_interval = data[4];
    pulses_within_interval += data[5] << 8;

    float total_kwh_within_interval = pulses_within_interval / this->pulses_per_kwh_;

    ESP_LOGI("powerpal_ble", "Timestamp: %d, Pulses: %d, Energy Used: %f kWh", unix_time, pulses_within_interval,
             total_kwh_within_interval);

    if (this->power_sensor_ != nullptr) {
      this->power_sensor_->publish_state(total_kwh_within_interval);
    }

    if (this->energy_sensor_ != nullptr) {
      total_pulses_ += pulses_within_interval;
      float energy = total_pulses_ / this->pulses_per_kwh_;
      this->energy_sensor_->publish_state(energy);
    }
  }
}

void Powerpal::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                   esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_DISCONNECT_EVT: {
      this->authenticated_ = false;
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      // auto *pairing_code_char_ = this->parent_->get_characteristic(POWERPAL_SERVICE_UUID,
      // POWERPAL_CHARACTERISTIC_PAIRING_CODE_UUID); if (pairing_code_char_ == nullptr) {
      //   ESP_LOGE(TAG, "[%s] No Powerpal service or Pairing Code Characteristic found at device, not a POWERPAL..?",
      //             this->parent_->address_str().c_str());
      //   break;
      // } else {
      //   this->pairing_code_char_handle_ = pairing_code_char_->handle;
      // }

      // auto *reading_batch_size_char_ = this->parent_->get_characteristic(POWERPAL_SERVICE_UUID,
      // POWERPAL_CHARACTERISTIC_READING_BATCH_SIZE_UUID); if (reading_batch_size_char_ == nullptr) {
      //   ESP_LOGE(TAG, "[%s] No Powerpal service or Reading Batch Size Characteristic found at device, not a
      //   POWERPAL..?",
      //             this->parent_->address_str().c_str());
      //   break;
      // } else {
      //   this->reading_batch_size_char_handle_ = reading_batch_size_char_->handle;
      // }

      // auto *measurement_char_ = this->parent_->get_characteristic(POWERPAL_SERVICE_UUID,
      // POWERPAL_CHARACTERISTIC_MEASUREMENT_UUID); if (measurement_char_ == nullptr) {
      //   ESP_LOGE(TAG, "[%s] No Powerpal service or Measurement Characteristic found at device, not a POWERPAL..?",
      //             this->parent_->address_str().c_str());
      //   break;
      // } else {
      //   this->measurement_char_handle_ = measurement_char_->handle;
      // }

      break;
    }
    case ESP_GATTC_READ_CHAR_EVT: {
      ESP_LOGE(TAG, "[%s] ESP_GATTC_READ_CHAR_EVT (Received READ)", this->parent_->address_str().c_str());
      if (param->read.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error reading char at handle %d, status=%d", param->read.handle, param->read.status);
        break;
      }
      // reading batch size
      if (param->read.handle == this->reading_batch_size_char_handle_) {
        ESP_LOGD(TAG, "Recieved reading_batch_size read event");
        this->decode_(param->read.value, param->read.value_len);
        if (param->read.value_len == 4) {
          if (param->read.value[0] != this->reading_batch_size_[0]) {
            // reading batch size needs changing, so write
            auto status =
                esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id,
                                         this->reading_batch_size_char_handle_, sizeof(this->reading_batch_size_),
                                         this->reading_batch_size_, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
            if (status) {
              ESP_LOGW(TAG, "Error sending write request for batch_size, status=%d", status);
            }
          } else {
            // reading batch size is set correctly so subscribe to measurement notifications
            auto status = esp_ble_gattc_register_for_notify(this->parent_->gattc_if, this->parent_->remote_bda,
                                                            this->measurement_char_handle_);
            if (status) {
              ESP_LOGW(TAG, "[%s] esp_ble_gattc_register_for_notify failed, status=%d",
                       this->parent_->address_str().c_str(), status);
            }
          }
        } else {
          // error, length should be 4
        }
        break;
      }

      // battery
      if (param->read.handle == this->battery_char_handle_) {
        ESP_LOGD(TAG, "Recieved battery read event");
        this->parse_battery_(param->read.value, param->read.value_len);
        break;
      }

      // firmware
      if (param->read.handle == this->firmware_char_handle_) {
        ESP_LOGD(TAG, "Recieved firmware read event");
        this->decode_(param->read.value, param->read.value_len);
        break;
      }

      // led sensitivity
      if (param->read.handle == this->led_sensitivity_char_handle_) {
        ESP_LOGD(TAG, "Recieved led sensitivity read event");
        this->decode_(param->read.value, param->read.value_len);
        break;
      }

      break;
    }

    case ESP_GATTC_WRITE_CHAR_EVT: {
      ESP_LOGE(TAG, "[%s] ESP_GATTC_WRITE_CHAR_EVT (Write confirmed)", this->parent_->address_str().c_str());
      if (param->write.status != ESP_GATT_OK) {
        ESP_LOGW(TAG, "Error writing value to char at handle %d, status=%d", param->write.handle, param->write.status);
        break;
      }

      if (param->write.handle == this->pairing_code_char_handle_ && !this->authenticated_) {
        this->authenticated_ = true;

        auto read_reading_batch_size_status =
            esp_ble_gattc_read_char(this->parent()->gattc_if, this->parent()->conn_id,
                                    this->reading_batch_size_char_handle_, ESP_GATT_AUTH_REQ_NONE);
        if (read_reading_batch_size_status) {
          ESP_LOGW(TAG, "Error sending read request for reading batch size, status=%d", read_reading_batch_size_status);
        }

        if (this->battery_ != nullptr) {
          // read battery
          auto read_battery_status = esp_ble_gattc_read_char(this->parent()->gattc_if, this->parent()->conn_id,
                                                             this->battery_char_handle_, ESP_GATT_AUTH_REQ_NONE);
          if (read_battery_status) {
            ESP_LOGW(TAG, "Error sending read request for battery, status=%d", read_battery_status);
          }
          // Enable notifications for battery
          auto notify_battery_status = esp_ble_gattc_register_for_notify(
              this->parent_->gattc_if, this->parent_->remote_bda, this->battery_char_handle_);
          if (notify_battery_status) {
            ESP_LOGW(TAG, "[%s] esp_ble_gattc_register_for_notify failed, status=%d",
                     this->parent_->address_str().c_str(), notify_battery_status);
          }
        }

        // read firmware version

        // read led sensitivity
        auto read_led_sensitivity_status =
            esp_ble_gattc_read_char(this->parent()->gattc_if, this->parent()->conn_id,
                                    this->led_sensitivity_char_handle_, ESP_GATT_AUTH_REQ_NONE);
        if (read_led_sensitivity_status) {
          ESP_LOGW(TAG, "Error sending read request for battery, status=%d", read_led_sensitivity_status);
        }

        break;
      }
      if (param->write.handle == this->reading_batch_size_char_handle_) {
        // reading batch size is now set correctly so subscribe to measurement notifications
        auto status = esp_ble_gattc_register_for_notify(this->parent_->gattc_if, this->parent_->remote_bda,
                                                        this->measurement_char_handle_);
        if (status) {
          ESP_LOGW(TAG, "[%s] esp_ble_gattc_register_for_notify failed, status=%d",
                   this->parent_->address_str().c_str(), status);
        }
        break;
      }

      ESP_LOGE(TAG, "[%s] Seemed to miss any handle matches, what is the handel?: %d",
               this->parent_->address_str().c_str(), param->write.handle);
      break;
    }  // ESP_GATTC_WRITE_CHAR_EVT

    case ESP_GATTC_NOTIFY_EVT: {
      ESP_LOGE(TAG, "[%s] Received Notification", this->parent_->address_str().c_str());

      // battery
      if (param->notify.handle == this->battery_char_handle_) {
        ESP_LOGD(TAG, "Recieved battery notify event");
        this->parse_battery_(param->notify.value, param->notify.value_len);
        break;
      }

      // battery
      if (param->notify.handle == this->measurement_char_handle_) {
        ESP_LOGD(TAG, "Recieved measurement notify event");
        this->parse_measurement_(param->notify.value, param->notify.value_len);
        break;
      }
      break;  // registerForNotify
    }
    default:
      break;
  }
}

void Powerpal::gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
  switch (event) {
    // This event is sent once authentication has completed
    case ESP_GAP_BLE_AUTH_CMPL_EVT: {
      if (param->ble_security.auth_cmpl.success) {
        ESP_LOGI(TAG, "[%s] Writing pairing code to Powerpal", this->parent_->address_str().c_str());
        auto status = esp_ble_gattc_write_char(this->parent()->gattc_if, this->parent()->conn_id,
                                               this->pairing_code_char_handle_, sizeof(this->pairing_code_),
                                               this->pairing_code_, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
        if (status) {
          ESP_LOGW(TAG, "Error sending write request for pairing_code, status=%d", status);
        }
      }
      break;
    }
    default:
      break;
  }
}

}  // namespace powerpal_ble
}  // namespace esphome

#endif
