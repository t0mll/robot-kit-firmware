#include "Sound.h"

void Sound::begin(int pin) {
  pin_ = pin;
  if (pin_ < 0) return;
  ledcSetup(CHANNEL, 2000, 10);
  ledcAttachPin(pin_, CHANNEL);
  ledcWriteTone(CHANNEL, 0);
}

void Sound::tone(uint16_t hz, uint16_t ms) {
  if (pin_ < 0) return;
  ledcWriteTone(CHANNEL, hz);
  stopAtMs_ = millis() + ms;
}

void Sound::chirp() { tone(1760, 40); }

void Sound::update() {
  if (stopAtMs_ && millis() >= stopAtMs_) {
    ledcWriteTone(CHANNEL, 0);
    stopAtMs_ = 0;
  }
}
