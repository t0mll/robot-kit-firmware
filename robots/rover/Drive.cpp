#include "Drive.h"
#include <driver/pcnt.h>
#include "RoverPins.h"

namespace {
// LEDC channels 0–3 drive the DRV8833 inputs (the Shell uses 6 for the buzzer, 7 for the backlight).
constexpr int CH_L1 = 0, CH_L2 = 1, CH_R1 = 2, CH_R2 = 3;
constexpr int PWM_HZ = 20000, PWM_BITS = 10, PWM_MAX = (1 << PWM_BITS) - 1;
constexpr float RAMP_PER_S = 250.0f;   // percent per second: 0 → 100 in 0.4 s
constexpr int STALL_MIN_SPEED = 35;    // only judge a stall when driving this hard
constexpr uint32_t STALL_MS = 400;     // no encoder movement for this long = stalled
constexpr long STALL_MIN_COUNTS = 2;

void setupCounter(pcnt_unit_t unit, int pulsePin, int dirPin) {
  pcnt_config_t cfg = {};
  cfg.pulse_gpio_num = pulsePin;
  cfg.ctrl_gpio_num = dirPin;
  cfg.channel = PCNT_CHANNEL_0;
  cfg.unit = unit;
  cfg.pos_mode = PCNT_COUNT_INC;
  cfg.neg_mode = PCNT_COUNT_DIS;
  cfg.lctrl_mode = PCNT_MODE_REVERSE;  // B low: count the other way
  cfg.hctrl_mode = PCNT_MODE_KEEP;
  cfg.counter_h_lim = 32767;
  cfg.counter_l_lim = -32768;
  pcnt_unit_config(&cfg);
  pcnt_set_filter_value(unit, 100);  // ignore glitches shorter than ~1.25 µs
  pcnt_filter_enable(unit);
  pcnt_counter_clear(unit);
  pcnt_counter_resume(unit);
}

long readCounter(pcnt_unit_t unit) {
  int16_t v = 0;
  pcnt_get_counter_value(unit, &v);
  return v;
}

int clampSpeed(int v) { return v > 100 ? 100 : (v < -100 ? -100 : v); }
}  // namespace

void Drive::begin() {
  const int pins[4] = {rover::MOTOR_L_IN1, rover::MOTOR_L_IN2, rover::MOTOR_R_IN1, rover::MOTOR_R_IN2};
  for (int ch = 0; ch < 4; ch++) {
    ledcSetup(ch, PWM_HZ, PWM_BITS);
    ledcAttachPin(pins[ch], ch);
    ledcWrite(ch, 0);
  }
  setupCounter(PCNT_UNIT_0, rover::ENC_L_A, rover::ENC_L_B);
  setupCounter(PCNT_UNIT_1, rover::ENC_R_A, rover::ENC_R_B);
  lastMs_ = millis();
}

void Drive::set(int left, int right) {
  targetL_ = clampSpeed(left);
  targetR_ = clampSpeed(right);
}

void Drive::brake() {
  targetL_ = targetR_ = 0;
  nowL_ = nowR_ = 0;
  apply(CH_L1, CH_L2, 0);
  apply(CH_R1, CH_R2, 0);
}

// DRV8833 slow decay: hold one input high and PWM the other inverted, so speed is linear in duty.
void Drive::apply(int chA, int chB, int speed) {
  const int duty = abs(speed) * PWM_MAX / 100;
  if (speed > 0) {
    ledcWrite(chA, PWM_MAX);
    ledcWrite(chB, PWM_MAX - duty);
  } else if (speed < 0) {
    ledcWrite(chA, PWM_MAX - duty);
    ledcWrite(chB, PWM_MAX);
  } else {
    ledcWrite(chA, 0);  // coast
    ledcWrite(chB, 0);
  }
}

long Drive::countsLeft() const { return readCounter(PCNT_UNIT_0); }
long Drive::countsRight() const { return -readCounter(PCNT_UNIT_1); }  // mirrored motor

void Drive::resetCounts() {
  pcnt_counter_clear(PCNT_UNIT_0);
  pcnt_counter_clear(PCNT_UNIT_1);
  lastL_ = lastR_ = 0;
}

void Drive::update() {
  const uint32_t now = millis();
  const float dt = (now - lastMs_) / 1000.0f;
  lastMs_ = now;
  const float step = RAMP_PER_S * dt;
  auto ramp = [step](float cur, int target) {
    if (cur < target) return min(cur + step, (float)target);
    if (cur > target) return max(cur - step, (float)target);
    return cur;
  };
  if (stalled_) targetL_ = targetR_ = 0;
  nowL_ = ramp(nowL_, targetL_);
  nowR_ = ramp(nowR_, targetR_);
  apply(CH_L1, CH_L2, (int)nowL_);
  apply(CH_R1, CH_R2, (int)nowR_);

  // Stall: a motor driven hard whose encoder hasn't moved. Cut both before the IP5306 browns out.
  const long l = countsLeft(), r = countsRight();
  const bool drivenL = abs((int)nowL_) >= STALL_MIN_SPEED, drivenR = abs((int)nowR_) >= STALL_MIN_SPEED;
  const bool stuckL = drivenL && labs(l - lastL_) < STALL_MIN_COUNTS;
  const bool stuckR = drivenR && labs(r - lastR_) < STALL_MIN_COUNTS;
  if (stuckL || stuckR) {
    if (!stillSinceMs_) stillSinceMs_ = now;
    if (now - stillSinceMs_ > STALL_MS) {
      stalled_ = true;
      brake();
    }
  } else {
    stillSinceMs_ = 0;
    lastL_ = l;
    lastR_ = r;
  }
}
