#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/core/component.h"

namespace esphome {
namespace pm2x05 {

enum PM2X05Type {
  PM2X05_TYPE_2005 = 0,
  PM2X05_TYPE_2105,
};


class PM2X05Component : public i2c::I2CDevice, public Component {
 public:
  PM2X05Component() = default;
  void loop() override;
  float get_setup_priority() const override;
  void dump_config() override;

  void set_type(PM2X05Type type) { type_ = type; }

  void set_update_interval(uint32_t val) { update_interval_ = val; };

  void set_pm_1_0_sensor(sensor::Sensor *pm_1_0_sensor);
  void set_pm_2_5_sensor(sensor::Sensor *pm_2_5_sensor);

 protected:
  optional<bool> check_data_();
  void parse_data_();
  uint16_t get_16_bit_uint_(uint8_t start_index);

  uint8_t data_[64];
  uint8_t situation_{0};
  uint32_t last_update_{0};
  uint32_t update_interval_{0};
  esphome::i2c::ErrorCode last_error_;
  uint8_t state_{0};
  PM2X05Type type_;

  // "Under Atmospheric Pressure"
  sensor::Sensor *pm_1_0_sensor_{nullptr};
  sensor::Sensor *pm_2_5_sensor_{nullptr};

};

}  // namespace pm2x05
}  // namespace esphome
