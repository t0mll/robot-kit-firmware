#pragma once
#include <stdint.h>

/// One Robot's battery thresholds, in battery volts; `empty` at 0 means no guard. A plain aggregate so
/// sketches can write `config.battery = {2.5f, 3.45f, 3.30f, 3.70f};` on the core's C++ standard.
struct BatteryLimits {
  float absentBelow;  // below this there's no cell (running on USB): the guard stays out of the way
  float low;          // warn: a tone, and a sleepy Face when no Mode is running; Modes keep running
  float empty;        // stop: leave the running Mode (actuators off) and refuse new ones until charged
  float recover;      // leave Low or Empty only above this, so a cell that sags and recovers can't flap
};

enum class BatteryState : uint8_t { Unknown, Ok, Low, Empty };

/// The battery logic of the Robot Shell, kept free of Arduino calls so it's unit-tested on the host
/// (test/test_battery). Readings are smoothed, and a new level must hold for HOLD_MS before the state
/// changes, so the dip when motors or servos start doesn't stop the Robot. Pulling the cell (a reading
/// below absentBelow) switches to Unknown at once.
class BatteryGuard {
 public:
  static constexpr uint32_t HOLD_MS = 3000;
  static constexpr float SMOOTHING = 0.2f;  // per reading; the Shell reads every 100 ms

  void setLimits(const BatteryLimits& limits) {
    limits_ = limits;
    state_ = pending_ = BatteryState::Unknown;
    primed_ = false;
  }
  bool enabled() const { return limits_.empty > 0; }

  /// Feed one reading (battery volts) and get the state after it.
  BatteryState update(float volts, uint32_t nowMs) {
    if (!enabled()) return state_ = BatteryState::Unknown;
    smoothed_ = primed_ ? smoothed_ + SMOOTHING * (volts - smoothed_) : volts;
    primed_ = true;

    if (smoothed_ < limits_.absentBelow) {
      state_ = pending_ = BatteryState::Unknown;
      return state_;
    }
    const BatteryState target = targetFor(smoothed_);
    if (target == state_) {
      pending_ = state_;
    } else if (target != pending_) {
      pending_ = target;
      pendingSinceMs_ = nowMs;
    } else if (nowMs - pendingSinceMs_ >= HOLD_MS) {
      state_ = target;
    }
    return state_;
  }

  BatteryState state() const { return state_; }
  float volts() const { return smoothed_; }

 private:
  static int rank(BatteryState s) { return s == BatteryState::Empty ? 0 : s == BatteryState::Low ? 1 : 2; }

  BatteryState targetFor(float v) const {
    const BatteryState level = v < limits_.empty ? BatteryState::Empty : v < limits_.low ? BatteryState::Low : BatteryState::Ok;
    // Getting better than Low or Empty needs the recover voltage (charging), not just a rested cell.
    const bool down = state_ == BatteryState::Low || state_ == BatteryState::Empty;
    if (down && rank(level) > rank(state_) && v <= limits_.recover) return state_;
    return level;
  }

  BatteryLimits limits_{};
  BatteryState state_ = BatteryState::Unknown;
  BatteryState pending_ = BatteryState::Unknown;
  uint32_t pendingSinceMs_ = 0;
  float smoothed_ = 0;
  bool primed_ = false;
};
