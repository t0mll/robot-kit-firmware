#pragma once

/// A behaviour picked from the Robot Shell menu ("Drive", "Line follow", a Customer's own program...).
/// enter() runs once when chosen, loop() every pass of the main loop, exit() when the Builder leaves
/// (long-press Select) or another Mode starts.
class Mode {
 public:
  virtual ~Mode() = default;
  virtual const char* name() const = 0;
  virtual void enter() {}
  virtual void loop() {}
  virtual void exit() {}
  /// True when the Mode moves motors or servos. The Shell won't start one on an empty battery and
  /// leaves it when the battery runs empty. Leave it true for anything that might move the Robot.
  virtual bool usesActuators() const { return true; }
};
