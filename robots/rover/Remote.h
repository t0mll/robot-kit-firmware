#pragma once
#include <Arduino.h>

/// Phone remote over Wi-Fi: home Wi-Fi when saved (preferred), otherwise the Rover's own hotspot.
/// Serves a joystick page; drive commands expire after 500 ms so a dropped phone stops the Rover.
class Remote {
 public:
  void begin(const char* robotName);
  void update();
  /// Latest joystick command (-100..100 per wheel), or false when none is fresh.
  bool command(int& left, int& right) const;
  /// One line for the Face's caption: where to point the phone.
  const char* hint() const { return hint_; }

 private:
  int left_ = 0, right_ = 0;
  uint32_t commandMs_ = 0;
  char hint_[48] = {0};
};
