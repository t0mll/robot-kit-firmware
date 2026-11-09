// Biped: the learning walker. Brief: https://github.com/t0mll/robot-kit-shop/blob/main/docs/briefs/biped.md
// Skeleton: the Robot Shell with the Biped's Modes as placeholders. The servo bus, IMU and trained
// policies arrive later (the 50 Hz spike: https://github.com/t0mll/robot-kit-shop/issues/102).
#include <RobotShell.h>

namespace {

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
PlaceholderMode explore("Explore", Expression::Surprised);
PlaceholderMode followHand("Follow my hand", Expression::Love);
PlaceholderMode pet("Pet", Expression::Sleepy);
PlaceholderMode calibrate("Calibrate", Expression::Confused);

}  // namespace

void setup() {
  RobotShellClass::Config config;
  config.robotName = "Biped";
  config.pins.buttonSelect = 16;
  config.pins.buzzer = 21;
  config.pins.batterySense = 2;  // battery-sense line on the Power Pack's 3-pin plug
  config.pins.batteryDivider = 4.0f;  // 2S pack: up to 8.4 V into a 3.3 V ADC

  RobotShell.addMode(drive);
  RobotShell.addMode(explore);
  RobotShell.addMode(followHand);
  RobotShell.addMode(pet);
  RobotShell.addMode(calibrate);
  RobotShell.begin(config);
}

void loop() { RobotShell.update(); }
