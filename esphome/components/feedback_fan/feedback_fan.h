#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/feedback_fan/feedback_fan.h"
#include "esphome/components/stepper/stepper.h"
#include "esphome/components/sensor/sensor.h"

// #include "esphome/components/binary/fan/binary_fan.h"
#include "esphome/components/hbridge/fan/hbridge_fan.h"
// #include "esphome/components/speed/fan/speed_fan.h"

// #include "esphome/components/pulse_counter/pulse_counter_sensor.h"

namespace esphome {
namespace feedback_fan {

enum FeedbackFanState { FAN_STOPPED = 0, FAN_MOVING = 1, FAN_ACCELERATING = 2, FAN_DECELERATING = 3 };

class FeedbackFan : public stepper::Stepper, public Component {
 public:
  void set_hbridge_fan(hbridge::HBridgeFan *hbridge_fan_output) { this->hbridge_fan_output_ = hbridge_fan_output; }
  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; };
  void set_initial_output(int initial_output) { this->initial_output_ = initial_output; }
  void set_min_output(int min_output) { this->min_output_ = min_output; }
  void set_positional_threshold(int positional_threshold) { this->positional_threshold_ = positional_threshold; }

  void setup() override;
  void dump_config() override;
  void loop() override;
  // PROCESSOR, to wait for sensor
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

 protected:
  FeedbackFanState fan_feedback_state_{FAN_STOPPED};

  float target_speed_{0.0f};
  uint32_t initial_output_{0};
  int32_t min_output_{0};
  uint32_t state_change_time_{0};
  int32_t steps_to_decelerate_{0};
  int32_t starting_position_;
  int32_t last_step_interval_;
  float target_step_;
  int16_t positional_threshold_;

  void turn_on_fan_(int32_t dir);

  // if called externally with now(micros()), it will give a faulty speed
  // only meant to be called when feedback is received
  void calculate_speed_(uint32_t now);
  void calculate_target_step_(float initial_velocity, uint32_t now, float accel);

  // speed::SpeedFan *speed_fan_output_{nullptr};
  // binary::BinaryFan *binary_fan_output_{nullptr};
  hbridge::HBridgeFan *hbridge_fan_output_{nullptr};

  /// The sensor used for positional feedback
  sensor::Sensor *sensor_{nullptr};
  // pulse_counter::PulseCounterSensor *pulse_counter_feedback_{nullptr};
};

template<typename... Ts> class SetInitialOutputAction : public Action<Ts...> {
 public:
  explicit SetInitialOutputAction(FeedbackFan *parent) : parent_(parent) {}

  TEMPLATABLE_VALUE(uint32_t, output)

  void play(Ts... x) override { this->parent_->set_initial_output(this->output_.value(x...)); }

 protected:
  FeedbackFan *parent_;
};

}  // namespace feedback_fan
}  // namespace esphome
