#pragma once
#include "Display.h"
#include "Expression.h"

/// The animated eyes. Drawn into a full-screen sprite in PSRAM, monochrome, with blinking.
/// Thread-safe setters: the render task draws while Modes change the Expression.
class Face {
 public:
  void begin(BrainDisplay& display);
  void set(Expression e);
  Expression current() const { return expression_; }
  /// Draw one frame of the Face (called by the Shell's render task).
  void draw(uint32_t nowMs);
  /// Optional status line under the eyes (empty string hides it).
  void caption(const char* text);
  /// The full-screen frame buffer, for the Shell's menu screens.
  LGFX_Sprite* sprite() { return sprite_; }

 private:
  void drawEye(int cx, int cy, int w, int h, bool left);
  BrainDisplay* display_ = nullptr;
  LGFX_Sprite* sprite_ = nullptr;
  volatile Expression expression_ = Expression::Neutral;
  uint32_t nextBlinkMs_ = 2500;
  char caption_[48] = {0};
};
