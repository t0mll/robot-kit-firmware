#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "BatteryGuard.h"
#include "BoardPins.h"
#include "Display.h"
#include "Expression.h"
#include "Face.h"
#include "Input.h"
#include "Mode.h"
#include "Sound.h"

#ifndef FW_VERSION
#define FW_VERSION "dev"
#endif

/// The Robot Shell: the Face by default, a menu of Modes on demand, buttons, buzzer, settings and battery.
/// Every Robot sketch (and every Customer's C program) calls begin() once and update() every loop.
class RobotShellClass {
 public:
  struct Config {
    const char* robotName = "Robot";
    ShellPins pins;
    /// Battery thresholds for this Robot's cells; the guard is off while `battery.empty` is 0.
    BatteryLimits battery{};
  };

  void begin(const Config& config);
  /// Register a Mode for the menu (up to 12). Call before or after begin().
  void addMode(Mode& mode);
  /// Run one pass: buttons, menu, the current Mode, sound.
  void update();

  /// Battery voltage, or 0 when no battery-sense pin is wired.
  float batteryVolts();
  /// Ok, Low or Empty from the battery guard; Unknown with no sense pin, no limits or no cell (USB only).
  BatteryState batteryState() const { return battery_.state(); }
  const char* robotName() const { return config_.robotName; }
  /// The running Mode's name, or "menu" / "face".
  const char* modeName() const;
  const char* version() const { return FW_VERSION; }

  Face face;
  Sound sound;
  Input input;
  Preferences settings;  // NVS: survives browser re-flashing unless the flash is erased

 private:
  enum class Screen : uint8_t { Face, Menu, Mode };
  static void renderTask(void* self);
  void drawMenu();
  void enterMode(int index);
  void leaveMode();
  void watchBattery(uint32_t nowMs);

  static constexpr int MAX_MODES = 12;
  Config config_;
  BrainDisplay display_;
  Mode* modes_[MAX_MODES] = {nullptr};
  int modeCount_ = 0;
  volatile int menuIndex_ = 0;
  volatile int activeMode_ = -1;
  volatile Screen screen_ = Screen::Face;
  bool started_ = false;
  BatteryGuard battery_;
  uint32_t lastBatteryMs_ = 0;
};

extern RobotShellClass RobotShell;
