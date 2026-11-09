#pragma once
#include <Arduino.h>

/// Two N20 motors on a DRV8833 with Hall encoders: ramped speed, stall cut-off.
/// Speeds are -100..100 (percent of full PWM). Call update() every loop.
class Drive {
 public:
  void begin();
  /// Ask for wheel speeds; the motors ramp towards them.
  void set(int left, int right);
  void stop() { set(0, 0); }
  /// Cut power immediately (no ramp), e.g. at a table edge.
  void brake();
  void update();

  long countsLeft() const;
  long countsRight() const;
  void resetCounts();
  /// True once a motor was cut because its encoder saw no movement while driven.
  bool stalled() const { return stalled_; }
  void clearStall() { stalled_ = false; }

 private:
  void apply(int channelA, int channelB, int speed);
  int targetL_ = 0, targetR_ = 0;
  float nowL_ = 0, nowR_ = 0;
  uint32_t lastMs_ = 0;
  long lastL_ = 0, lastR_ = 0;
  uint32_t stillSinceMs_ = 0;
  bool stalled_ = false;
};
