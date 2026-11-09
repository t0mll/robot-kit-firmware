// Rover: the entry Robot. Brief: https://github.com/t0mll/robot-kit-shop/blob/main/docs/briefs/rover.md
// Skeleton: the Robot Shell with the Rover's Modes as placeholders. Motors, sensors and the phone
// remote arrive with "Write the Rover r1 Firmware" (https://github.com/t0mll/robot-kit-shop/issues/95).
#include <RobotShell.h>

namespace {

/// Placeholder Mode: shows its Expression and name until the real behaviour lands.
class PlaceholderMode : public Mode {
 public:
  PlaceholderMode(const char* name, Expression expression) : name_(name), expression_(expression) {}
  const char* name() const override { return name_; }
  void enter() override { RobotShell.face.set(expression_); }

 private:
  const char* name_;
  Expression expression_;
};

PlaceholderMode drive("Drive", Expression::Happy);
PlaceholderMode lineFollow("Line follow", Expression::Neutral);
PlaceholderMode explore("Explore", Expression::Surprised);
PlaceholderMode followHand("Follow my hand", Expression::Love);
PlaceholderMode wiringCheck("Wiring check", Expression::Confused);

}  // namespace

void setup() {
  RobotShellClass::Config config;
  config.robotName = "Rover";
  config.pins.buttonSelect = 16;
  config.pins.buzzer = 17;
  config.pins.batterySense = 1;  // 100k/100k divider from the Light Power cell
  config.pins.batteryDivider = 2.0f;

  RobotShell.addMode(drive);
  RobotShell.addMode(lineFollow);
  RobotShell.addMode(explore);
  RobotShell.addMode(followHand);
  RobotShell.addMode(wiringCheck);
  RobotShell.begin(config);
}

void loop() { RobotShell.update(); }
