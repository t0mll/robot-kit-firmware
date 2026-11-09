#pragma once
#include <Arduino.h>

/// Passive buzzer on an LEDC channel (core 2.0.17 API). Non-blocking tones.
class Sound {
 public:
  void begin(int pin);
  void tone(uint16_t hz, uint16_t ms);
  void chirp();  // short "I heard you" beep
  void update();  // call every loop to end tones on time

 private:
  int pin_ = -1;
  uint32_t stopAtMs_ = 0;
  static constexpr int CHANNEL = 6;
};
