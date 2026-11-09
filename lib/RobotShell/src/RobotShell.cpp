#include "RobotShell.h"

RobotShellClass RobotShell;

namespace {
constexpr uint32_t FRAME_MS = 33;  // ~30 fps

/// Built-in Mode: robot name, Firmware version and battery on the Face's caption.
class AboutMode : public Mode {
 public:
  const char* name() const override { return "About"; }
  void enter() override {
    RobotShell.face.set(Expression::Happy);
    char line[48];
    const float v = RobotShell.batteryVolts();
    if (v > 0.1f) snprintf(line, sizeof(line), "%s  fw %s  %.2f V", RobotShell.robotName(), RobotShell.version(), v);
    else snprintf(line, sizeof(line), "%s  fw %s", RobotShell.robotName(), RobotShell.version());
    RobotShell.face.caption(line);
  }
  void exit() override { RobotShell.face.caption(""); }
};
AboutMode aboutMode;
}  // namespace

void RobotShellClass::begin(const Config& config) {
  config_ = config;
  pinMode(pins::LCD_POWER, OUTPUT);
  digitalWrite(pins::LCD_POWER, HIGH);  // backlight power when running on battery

  display_.init();
  display_.setRotation(1);  // landscape, 320×170
  display_.setBrightness(200);
  face.begin(display_);
  input.begin(pins::BUTTON_PREV, config_.pins.buttonSelect, pins::BUTTON_NEXT);
  sound.begin(config_.pins.buzzer);
  settings.begin("shell", false);
  if (config_.pins.batterySense >= 0) analogReadResolution(12);

  addMode(aboutMode);
  started_ = true;
  // Rendering runs on core 0 so animation never stutters; Modes run in loop() on core 1.
  xTaskCreatePinnedToCore(renderTask, "render", 6144, this, 1, nullptr, 0);
  sound.chirp();
}

void RobotShellClass::addMode(Mode& mode) {
  for (int i = 0; i < modeCount_; i++)
    if (modes_[i] == &mode) return;
  if (modeCount_ < MAX_MODES) modes_[modeCount_++] = &mode;
}

const char* RobotShellClass::modeName() const {
  if (screen_ == Screen::Mode && activeMode_ >= 0) return modes_[activeMode_]->name();
  return screen_ == Screen::Menu ? "menu" : "face";
}

float RobotShellClass::batteryVolts() {
  if (config_.pins.batterySense < 0) return 0.0f;
  return analogReadMilliVolts(config_.pins.batterySense) / 1000.0f * config_.pins.batteryDivider;
}

void RobotShellClass::enterMode(int index) {
  if (index < 0 || index >= modeCount_) return;
  activeMode_ = index;
  screen_ = Screen::Mode;
  face.caption(modes_[index]->name());
  modes_[index]->enter();
}

void RobotShellClass::leaveMode() {
  if (activeMode_ >= 0) modes_[activeMode_]->exit();
  activeMode_ = -1;
  face.caption("");
  face.set(Expression::Neutral);
  screen_ = Screen::Face;
}

void RobotShellClass::update() {
  if (!started_) return;
  const ButtonEvent ev = input.poll();
  if (ev != ButtonEvent::None) sound.chirp();

  switch (screen_) {
    case Screen::Face:
      if (ev == ButtonEvent::Select || ev == ButtonEvent::Next || ev == ButtonEvent::Prev) screen_ = Screen::Menu;
      break;
    case Screen::Menu:
      if (ev == ButtonEvent::Next) menuIndex_ = (menuIndex_ + 1) % modeCount_;
      else if (ev == ButtonEvent::Prev) menuIndex_ = (menuIndex_ + modeCount_ - 1) % modeCount_;
      else if (ev == ButtonEvent::Select) enterMode(menuIndex_);
      else if (ev == ButtonEvent::LongSelect) screen_ = Screen::Face;
      break;
    case Screen::Mode:
      if (ev == ButtonEvent::LongSelect) leaveMode();
      else if (activeMode_ >= 0) modes_[activeMode_]->loop();
      break;
  }
  sound.update();
}

void RobotShellClass::drawMenu() {
  LGFX_Sprite* s = face.sprite();
  if (!s) return;
  s->fillSprite(TFT_BLACK);
  s->setTextColor(TFT_WHITE, TFT_BLACK);
  s->setFont(&fonts::Font2);
  s->setTextDatum(lgfx::top_left);
  s->drawString(config_.robotName, 12, 8);
  s->drawFastHLine(12, 28, s->width() - 24, TFT_WHITE);

  // Up to five rows around the selection.
  const int rows = 5, rowH = 26, top = 36;
  const int first = max(0, min(menuIndex_ - rows / 2, modeCount_ - rows));
  for (int i = 0; i < rows && first + i < modeCount_; i++) {
    const int idx = first + i;
    const int y = top + i * rowH;
    if (idx == menuIndex_) {
      s->fillRect(8, y - 2, s->width() - 16, rowH - 2, TFT_WHITE);
      s->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
      s->setTextColor(TFT_WHITE, TFT_BLACK);
    }
    s->drawString(modes_[idx]->name(), 16, y + 3);
  }
  s->pushSprite(0, 0);
}

void RobotShellClass::renderTask(void* self) {
  auto* shell = static_cast<RobotShellClass*>(self);
  for (;;) {
    const uint32_t start = millis();
    if (shell->screen_ == Screen::Menu) shell->drawMenu();
    else shell->face.draw(start);
    const uint32_t spent = millis() - start;
    vTaskDelay(pdMS_TO_TICKS(spent < FRAME_MS ? FRAME_MS - spent : 1));
  }
}
