#pragma once
#include <Arduino.h>

/// What the line sensor sees under the nose. `valid` is false when no sensor answered.
struct LineReading {
  bool valid = false;
  uint8_t channels = 0;
  bool onLine[8] = {false};  // true over a dark line
  bool floorSeen = true;     // false when nothing reflects back: a table edge, or lifted
  /// Line position from -1 (far left) to +1 (far right), or NAN when no channel sees the line.
  float position = NAN;
};

/// The Rover's sensors on the Brain's I2C (Qwiic): VL53L0X distance and the line sensor.
class Sensors {
 public:
  void begin();
  void update();
  /// Distance ahead in millimetres, or -1 when the ToF isn't answering or sees nothing in range.
  int distanceMm() const { return distanceMm_; }
  const LineReading& line() const { return line_; }
  bool tofPresent() const { return tofOk_; }
  /// Addresses that answered on the I2C bus (for the wiring check). Returns how many.
  int scan(uint8_t* found, int max);

 private:
  void readLine();
  bool tofOk_ = false;
  int distanceMm_ = -1;
  uint8_t lineAddress_ = 0;
  LineReading line_;
  uint32_t lastLineMs_ = 0;
};
