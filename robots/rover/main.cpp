// Rover r1: the entry Robot. Brief: https://github.com/t0mll/robot-kit-shop/blob/main/docs/briefs/rover.md
// Hardware: hardware/rover-r1.md in the shop repo. Not yet run on a real Rover: r1 is for the first build.
#include <RobotShell.h>
#include "Drive.h"
#include "Remote.h"
#include "RoverPins.h"
#include "Sensors.h"

Drive drive;
Sensors sensors;
Remote remote;

const char* currentModeName() { return RobotShell.modeName(); }
int currentDistanceMm() { return sensors.distanceMm(); }

namespace {

constexpr int EDGE_BACKUP_MS = 400, TURN_MS = 600;

/// Shared safety for every driving Mode: stop at a table edge or when lifted (no floor under the nose).
bool edgeAhead() {
  const LineReading& l = sensors.line();
  return l.valid && !l.floorSeen;
}

/// Base for driving Modes: clears a previous stall and stops the motors on exit.
class DrivingMode : public Mode {
 public:
  void enter() override {
    drive.clearStall();
    drive.resetCounts();
    onEnter();
  }
  void exit() override { drive.brake(); }
  void loop() override {
    if (drive.stalled()) {
      RobotShell.face.set(Expression::Confused);
      RobotShell.face.caption("Wheels stuck: long-press to reset");
      return;
    }
    step();
  }

 protected:
  virtual void onEnter() {}
  virtual void step() = 0;
};

// ---- Drive: the phone remote (or, later, a gamepad) ---------------------------------------------
class DriveMode : public DrivingMode {
 public:
  const char* name() const override { return "Drive"; }

 protected:
  void onEnter() override {
    RobotShell.face.set(Expression::Happy);
    RobotShell.face.caption(remote.hint());
  }
  void step() override {
    int l = 0, r = 0;
    if (!remote.command(l, r)) {
      drive.stop();
      return;
    }
    if (edgeAhead() && (l > 0 || r > 0)) {  // never drive forward off an edge
      drive.brake();
      RobotShell.face.set(Expression::Surprised);
      return;
    }
    drive.set(l, r);
    RobotShell.face.set(abs(l) + abs(r) > 120 ? Expression::Happy : Expression::Neutral);
  }
};

// ---- Line follow: on/off first; proportional and PID are lessons ---------------------------------
class LineFollowMode : public DrivingMode {
 public:
  const char* name() const override { return "Line follow"; }

 protected:
  void onEnter() override { RobotShell.face.set(Expression::Neutral); }
  void step() override {
    const LineReading& l = sensors.line();
    if (!l.valid) {
      drive.stop();
      RobotShell.face.set(Expression::Confused);
      RobotShell.face.caption("No line sensor: run Wiring check");
      return;
    }
    if (edgeAhead()) {
      drive.brake();
      RobotShell.face.set(Expression::Surprised);
      return;
    }
    if (isnan(l.position)) {  // lost the line: turn slowly to look for it
      drive.set(-25, 25);
      RobotShell.face.set(Expression::Confused);
      return;
    }
    constexpr int BASE = 35, TURN = 30;
    const int steer = (int)(l.position * TURN);
    drive.set(BASE + steer, BASE - steer);  // line to the right → turn right
    RobotShell.face.set(Expression::Happy);
  }
};

// ---- Explore: wander, turn away from obstacles and edges -----------------------------------------
class ExploreMode : public DrivingMode {
 public:
  const char* name() const override { return "Explore"; }

 protected:
  enum class Phase : uint8_t { Forward, Backup, Turn };
  void onEnter() override {
    phase_ = Phase::Forward;
    RobotShell.face.set(Expression::Happy);
  }
  void step() override {
    const uint32_t now = millis();
    const int d = sensors.distanceMm();
    switch (phase_) {
      case Phase::Forward:
        if (edgeAhead() || (d > 0 && d < 100)) {
          drive.brake();
          RobotShell.face.set(Expression::Surprised);
          RobotShell.sound.tone(880, 120);
          phase_ = Phase::Backup;
          until_ = now + EDGE_BACKUP_MS;
        } else {
          drive.set(40, 40);
        }
        break;
      case Phase::Backup:
        drive.set(-35, -35);
        if (now > until_) {
          phase_ = Phase::Turn;
          until_ = now + TURN_MS + (esp_random() % 400);
          turnLeft_ = esp_random() & 1;
        }
        break;
      case Phase::Turn:
        drive.set(turnLeft_ ? -35 : 35, turnLeft_ ? 35 : -35);
        if (now > until_) {
          phase_ = Phase::Forward;
          RobotShell.face.set(Expression::Happy);
        }
        break;
    }
  }

 private:
  Phase phase_ = Phase::Forward;
  uint32_t until_ = 0;
  bool turnLeft_ = false;
};

// ---- Follow my hand: keep 12 cm from whatever is in front ----------------------------------------
class FollowMode : public DrivingMode {
 public:
  const char* name() const override { return "Follow my hand"; }

 protected:
  void onEnter() override { RobotShell.face.set(Expression::Love); }
  void step() override {
    const int d = sensors.distanceMm();
    if (d < 0 || d > 400) {  // nothing to follow
      drive.stop();
      RobotShell.face.set(Expression::Sleepy);
      return;
    }
    const int err = d - 120;
    int speed = constrain(err * 2 / 5, -40, 40);
    if (abs(err) < 15) speed = 0;
    if (speed > 0 && edgeAhead()) speed = 0;
    drive.set(speed, speed);
    RobotShell.face.set(Expression::Love);
  }
};

// ---- Wiring check: name what's connected, spin each wheel, show encoder counts -------------------
class WiringCheckMode : public Mode {
 public:
  const char* name() const override { return "Wiring check"; }
  void enter() override {
    step_ = 0;
    until_ = millis();
    drive.clearStall();
    RobotShell.face.set(Expression::Confused);
  }
  void exit() override { drive.brake(); }
  void loop() override {
    if (millis() < until_) return;
    char line[48];
    switch (step_++) {
      case 0: {
        uint8_t found[8];
        const int n = sensors.scan(found, 8);
        int len = snprintf(line, sizeof(line), "I2C:");
        for (int i = 0; i < n && len < (int)sizeof(line) - 6; i++) len += snprintf(line + len, sizeof(line) - len, " %02X", found[i]);
        if (!n) snprintf(line, sizeof(line), "I2C: nothing found");
        RobotShell.face.caption(line);
        until_ = millis() + 2500;
        break;
      }
      case 1:
        drive.resetCounts();
        drive.set(30, 0);
        RobotShell.face.caption("Left wheel: forward?");
        until_ = millis() + 900;
        break;
      case 2:
        drive.brake();
        snprintf(line, sizeof(line), "Left counts %+ld", drive.countsLeft());
        RobotShell.face.caption(line);
        until_ = millis() + 1500;
        break;
      case 3:
        drive.resetCounts();
        drive.set(0, 30);
        RobotShell.face.caption("Right wheel: forward?");
        until_ = millis() + 900;
        break;
      case 4:
        drive.brake();
        snprintf(line, sizeof(line), "Right counts %+ld", drive.countsRight());
        RobotShell.face.caption(line);
        until_ = millis() + 1500;
        break;
      case 5:
        snprintf(line, sizeof(line), "ToF %s  line %s  %.2f V", sensors.tofPresent() ? "ok" : "MISSING",
                 sensors.line().valid ? "ok" : "MISSING", RobotShell.batteryVolts());
        RobotShell.face.caption(line);
        RobotShell.face.set(sensors.tofPresent() && sensors.line().valid ? Expression::Happy : Expression::Sad);
        until_ = UINT32_MAX;  // stays until long-press
        break;
    }
  }

 private:
  int step_ = 0;
  uint32_t until_ = 0;
};

DriveMode driveMode;
LineFollowMode lineFollowMode;
ExploreMode exploreMode;
FollowMode followMode;
WiringCheckMode wiringCheckMode;

}  // namespace

void setup() {
  RobotShellClass::Config config;
  config.robotName = "Rover";
  config.pins.buttonSelect = rover::BUTTON_SELECT;
  config.pins.buzzer = rover::BUZZER;
  config.pins.batterySense = rover::BATTERY_SENSE;
  config.pins.batteryDivider = 2.0f;

  RobotShell.addMode(driveMode);
  RobotShell.addMode(lineFollowMode);
  RobotShell.addMode(exploreMode);
  RobotShell.addMode(followMode);
  RobotShell.addMode(wiringCheckMode);
  RobotShell.begin(config);

  drive.begin();
  sensors.begin();
  remote.begin(config.robotName);
}

void loop() {
  sensors.update();
  remote.update();
  RobotShell.update();  // runs the active Mode
  drive.update();       // ramps and watches for stalls, whatever the Mode asked for
}
