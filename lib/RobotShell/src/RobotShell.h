#pragma once
#include <Arduino.h>
#include <Preferences.h>
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
  };

  void begin(const Config& config);
  /// Register a Mode for the menu (up to 12). Call before or after begin().
  void addMode(Mode& mode);
  /// Run one pass: buttons, menu, the current Mode, sound.
  void update();

  /// Battery voltage, or 0 when no battery-sense pin is wired.
  float batteryVolts();
  const char* robotName() const { return config_.robotName; }
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

  static constexpr int MAX_MODES = 12;
  Config config_;
  BrainDisplay display_;
  Mode* modes_[MAX_MODES] = {nullptr};
  int modeCount_ = 0;
  volatile int menuIndex_ = 0;
  volatile int activeMode_ = -1;
  volatile Screen screen_ = Screen::Face;
  bool started_ = false;
};

extern RobotShellClass RobotShell;
