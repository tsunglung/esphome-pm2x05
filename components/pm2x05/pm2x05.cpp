#include "pm2x05.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace pm2x05 {

static const char *const TAG = "pm2x05";

void PM2X05Component::set_pm_1_0_sensor(sensor::Sensor *pm_1_0_sensor) { pm_1_0_sensor_ = pm_1_0_sensor; }
void PM2X05Component::set_pm_2_5_sensor(sensor::Sensor *pm_2_5_sensor) { pm_2_5_sensor_ = pm_2_5_sensor; }

static const uint8_t OUTPUT_REGISTER = 12;
static const uint8_t INPUT_REG = 12;

void PM2X05Component::loop() {
  const uint32_t now = millis();
  uint16_t value = 12;
  uint8_t buffer[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  if (now - this->last_update_ < this->update_interval_) {
    // Otherwise just leave the sensor powered up and come back when we hit the update
    // time
    return;
  }
  this->last_error_ = this->write_register(OUTPUT_REGISTER, (uint8_t *) &value, 2);
  if (last_error_ != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "write_register(): I2C I/O error: %d", (int) this->last_error_);
    return;
  }

  this->last_error_ = this->read_register(INPUT_REG, buffer, 12, false);
  if (last_error_ != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "read_register_(): I2C I/O error: %d", (int) this->last_error_);
    return;
  }
  memcpy(this->data_, buffer, 12);
  this->last_update_ = now;

  this->status_clear_warning();
  auto check = this->check_data_();
  if (check.has_value()) {
    // finished
    this->parse_data_();
  }
}

float PM2X05Component::get_setup_priority() const { return setup_priority::DATA; }
optional<bool> PM2X05Component::check_data_() {
    bool header_matches = false;
    uint8_t payload_header = 0x0;
    uint8_t payload_length = 0x0;

    switch (this->type_) {
        case PM2X05_TYPE_2005:
        payload_header = this->data_[1];
        payload_length = this->data_[2];
        this->situation_ = this->data_[3];
        break;
        case PM2X05_TYPE_2105:
        payload_header = this->data_[1];
        payload_length = this->data_[2];
        this->situation_ = this->data_[3];
        break;
    }
    header_matches = payload_header == 0x16;

    if (!header_matches) {
      ESP_LOGW(TAG, "PM2X05 header %u doesn't match. Are you using the correct PM2X05 type?", payload_header);
      return false;
    }
    switch (this->situation_) {
        case 1:
            ESP_LOGD(TAG, "Sensor situation: Close.");
            return false;
        case 2:
            ESP_LOGD(TAG, "Sensor situation: Malfunction.");
            return false;
        case 3:
            ESP_LOGD(TAG, "Sensor situation: Under detecting.");
            return false;
        case 0x80:
            ESP_LOGD(TAG, "Sensor situation: Detecting completed.");
            return true;
    }

  return {};
}

void PM2X05Component::parse_data_() {
  uint16_t measuring_mode = 0x0;

  switch (this->type_) {
    case PM2X05_TYPE_2005: {
      uint16_t pm_2_5_concentration = this->get_16_bit_uint_(7);
      uint16_t pm_1_0_concentration = this->get_16_bit_uint_(9);
      measuring_mode = this->get_16_bit_uint_(11);

      ESP_LOGD(TAG,
               "Got PM1.0 Concentration: %u µg/m^3, PM2.5 Concentration %u µg/m^3",
               pm_1_0_concentration, pm_2_5_concentration);

      if (this->pm_1_0_sensor_ != nullptr)
        this->pm_1_0_sensor_->publish_state(pm_1_0_concentration);
      if (this->pm_2_5_sensor_ != nullptr)
        this->pm_2_5_sensor_->publish_state(pm_2_5_concentration);
      break;
    }
    case PM2X05_TYPE_2105: {
      uint16_t pm_2_5_concentration = this->get_16_bit_uint_(6);
      uint16_t pm_1_0_concentration = this->get_16_bit_uint_(8);
      measuring_mode = this->get_16_bit_uint_(10);

      ESP_LOGD(TAG,
               "Got PM1.0 Concentration: %u µg/m^3, PM2.5 Concentration %u µg/m^3",
               pm_1_0_concentration, pm_2_5_concentration);

      if (this->pm_1_0_sensor_ != nullptr)
        this->pm_1_0_sensor_->publish_state(pm_1_0_concentration);
      if (this->pm_2_5_sensor_ != nullptr)
        this->pm_2_5_sensor_->publish_state(pm_2_5_concentration);
      break;
    }
  }

  switch (measuring_mode) {
    case 2:
        ESP_LOGD(TAG, "The measuring mode of sensor: Single measuring mode.");
    break;
    case 3:
        ESP_LOGD(TAG, "The measuring mode of sensor: Continuous measuring mode.");
    break;
    case 5:
        ESP_LOGD(TAG, "The measuring mode of sensor: Dynamic measuring mode.");
    break;
  }
  this->situation_ = 0x0;
}

uint16_t PM2X05Component::get_16_bit_uint_(uint8_t start_index) {
  return (uint16_t(this->data_[start_index + 1]) << 8) | uint16_t(this->data_[start_index]);
}

void PM2X05Component::dump_config() {
  ESP_LOGCONFIG(TAG, "PM2X05:");
  LOG_I2C_DEVICE(this);
  if (this->pm_1_0_sensor_ != nullptr)
    LOG_SENSOR("  ", "PM1.0", this->pm_1_0_sensor_);
  if (this->pm_2_5_sensor_ != nullptr)
    LOG_SENSOR("  ", "PM2.5", this->pm_2_5_sensor_);
}

}  // namespace pm2x05
}  // namespace esphome
