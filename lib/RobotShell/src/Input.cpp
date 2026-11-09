#include "Input.h"

namespace {
constexpr uint32_t DEBOUNCE_MS = 25;
constexpr uint32_t LONG_MS = 700;
}  // namespace

void Input::begin(int prevPin, int selectPin, int nextPin) {
  prev_.pin = prevPin;
  select_.pin = selectPin;
  next_.pin = nextPin;
  for (Button* b : {&prev_, &select_, &next_})
    if (b->pin >= 0) pinMode(b->pin, INPUT_PULLUP);
}

bool Input::read(Button& b, uint32_t now, bool& released, bool& longPress) {
  released = longPress = false;
  if (b.pin < 0) return false;
  const bool isDown = digitalRead(b.pin) == LOW;
  if (isDown != b.down && now - b.changedMs > DEBOUNCE_MS) {
    b.down = isDown;
    b.changedMs = now;
    if (!isDown) released = !b.longFired;
    b.longFired = false;
  }
  if (b.down && !b.longFired && now - b.changedMs > LONG_MS) {
    b.longFired = true;
    longPress = true;
  }
  return b.down;
}

ButtonEvent Input::poll() {
  const uint32_t now = millis();
  bool rel, lng;
  read(prev_, now, rel, lng);
  if (rel) return ButtonEvent::Prev;
  read(next_, now, rel, lng);
  if (rel) return ButtonEvent::Next;
  read(select_, now, rel, lng);
  if (lng) return ButtonEvent::LongSelect;
  if (rel) return ButtonEvent::Select;
  return ButtonEvent::None;
}

bool Input::held(ButtonEvent which) const {
  switch (which) {
    case ButtonEvent::Prev: return prev_.down;
    case ButtonEvent::Next: return next_.down;
    case ButtonEvent::Select:
    case ButtonEvent::LongSelect: return select_.down;
    default: return false;
  }
}
