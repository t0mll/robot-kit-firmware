#pragma once
#include <Arduino.h>

/// The three Brain buttons: previous, select, next. Debounced; short and long presses.
enum class ButtonEvent : uint8_t { None, Prev, Select, Next, LongSelect };

class Input {
 public:
  void begin(int prevPin, int selectPin, int nextPin);
  /// Poll once per loop; returns at most one event.
  ButtonEvent poll();
  bool held(ButtonEvent which) const;

 private:
  struct Button {
    int pin = -1;
    bool down = false;
    bool longFired = false;
    uint32_t changedMs = 0;
  };
  bool read(Button& b, uint32_t now, bool& released, bool& longPress);
  Button prev_, select_, next_;
};
