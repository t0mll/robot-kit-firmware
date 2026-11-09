#include "Sensors.h"
#include <VL53L0X.h>
#include <Wire.h>
#include <BoardPins.h>

namespace {
VL53L0X tof;
constexpr uint8_t TOF_ADDRESS = 0x29;

// The 5-channel I2C line sensors on AliExpress don't publish a protocol, so r1 tries this list of
// candidate addresses and reads one status byte (bit n = channel n over a line). The samples ordered
// for the prototype decide the real driver: https://github.com/t0mll/robot-kit-shop/issues/87 fog.
constexpr uint8_t LINE_CANDIDATES[] = {0x12, 0x57, 0x58, 0x3C, 0x50};
constexpr uint8_t LINE_CHANNELS = 5;
}  // namespace

void Sensors::begin() {
  Wire.begin(pins::I2C_SDA, pins::I2C_SCL, 400000);
  tof.setTimeout(30);
  tofOk_ = tof.init();
  if (tofOk_) {
    tof.setMeasurementTimingBudget(33000);  // ~30 Hz
    tof.startContinuous();
  }
  for (uint8_t addr : LINE_CANDIDATES) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      lineAddress_ = addr;
      break;
    }
  }
}

void Sensors::update() {
  if (tofOk_) {
    const uint16_t mm = tof.readRangeContinuousMillimeters();
    distanceMm_ = (tof.timeoutOccurred() || mm >= 2000) ? -1 : mm;
  }
  const uint32_t now = millis();
  if (now - lastLineMs_ >= 5) {  // 200 Hz is plenty for line following
    lastLineMs_ = now;
    readLine();
  }
}

void Sensors::readLine() {
  line_ = LineReading{};
  if (!lineAddress_) return;
  if (Wire.requestFrom(lineAddress_, (uint8_t)1) != 1) return;
  const uint8_t bits = Wire.read();
  line_.valid = true;
  line_.channels = LINE_CHANNELS;
  int count = 0;
  float sum = 0;
  for (int i = 0; i < LINE_CHANNELS; i++) {
    line_.onLine[i] = bits & (1 << i);
    if (line_.onLine[i]) {
      count++;
      sum += (i - (LINE_CHANNELS - 1) / 2.0f) / ((LINE_CHANNELS - 1) / 2.0f);
    }
  }
  line_.position = count ? sum / count : NAN;
  // Every channel "dark" means no reflection at all: no floor under the nose (edge, or lifted).
  line_.floorSeen = count < LINE_CHANNELS;
}

int Sensors::scan(uint8_t* found, int max) {
  int n = 0;
  for (uint8_t addr = 1; addr < 127 && n < max; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) found[n++] = addr;
  }
  return n;
}
