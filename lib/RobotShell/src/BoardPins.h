#pragma once
// LilyGO T-Display-S3 (the Brain). Display and on-board buttons are fixed by the board;
// the 3rd button, buzzer and battery sense are wired by the Builder (defaults below, per Robot sketch overridable).
// Never reconfigure GPIO19/20: they're the native USB pins, and browser re-flashing breaks.

namespace pins {
constexpr int LCD_POWER = 15;   // must be driven HIGH, or the backlight stays off on battery
constexpr int LCD_BACKLIGHT = 38;
constexpr int LCD_D0 = 39, LCD_D1 = 40, LCD_D2 = 41, LCD_D3 = 42, LCD_D4 = 45, LCD_D5 = 46, LCD_D6 = 47, LCD_D7 = 48;
constexpr int LCD_WR = 8, LCD_RD = 9, LCD_DC = 7, LCD_CS = 6, LCD_RST = 5;

constexpr int BUTTON_PREV = 0;    // on-board BOOT button
constexpr int BUTTON_NEXT = 14;   // on-board KEY button
}  // namespace pins

/// Pins the Builder wires; each Robot sketch passes its own in Shell::Config.
struct ShellPins {
  int buttonSelect = 16;   // the 3rd button
  int buzzer = 17;         // passive buzzer
  int batterySense = -1;   // ADC pin behind a divider, or -1 when not wired
  float batteryDivider = 2.0f;  // volts at the pin × this = battery volts (100k/100k → 2.0)
};
