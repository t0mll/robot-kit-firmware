#include "Face.h"

namespace {
constexpr uint16_t INK = TFT_WHITE;
constexpr uint16_t PAPER = TFT_BLACK;
constexpr uint32_t BLINK_MS = 140;
}  // namespace

void Face::begin(BrainDisplay& display) {
  display_ = &display;
  sprite_ = new LGFX_Sprite(&display);
  sprite_->setPsram(true);
  sprite_->setColorDepth(16);
  sprite_->createSprite(display.width(), display.height());
}

void Face::set(Expression e) { expression_ = e; }

void Face::caption(const char* text) {
  strncpy(caption_, text ? text : "", sizeof(caption_) - 1);
  caption_[sizeof(caption_) - 1] = 0;
}

void Face::drawEye(int cx, int cy, int w, int h, bool left) {
  auto& s = *sprite_;
  const Expression e = expression_;
  switch (e) {
    case Expression::Surprised:
      s.fillCircle(cx, cy, h / 2 + 6, INK);
      break;
    case Expression::Sleepy:
      s.fillRoundRect(cx - w / 2, cy + h / 4, w, 8, 4, INK);
      break;
    case Expression::Happy:  // upside-down U
      s.fillRoundRect(cx - w / 2, cy - h / 2, w, h, w / 3, INK);
      s.fillEllipse(cx, cy + h / 2, w / 2 + 2, h / 2 + 4, PAPER);
      break;
    case Expression::Sad:  // outer top corner cut down
      s.fillRoundRect(cx - w / 2, cy - h / 2, w, h, w / 3, INK);
      s.fillTriangle(left ? cx - w / 2 - 2 : cx + w / 2 + 2, cy - h / 2 - 2, cx, cy - h / 2 - 2,
                     left ? cx - w / 2 - 2 : cx + w / 2 + 2, cy, PAPER);
      break;
    case Expression::Angry:  // inner top corner cut down
      s.fillRoundRect(cx - w / 2, cy - h / 2, w, h, w / 3, INK);
      s.fillTriangle(left ? cx + w / 2 + 2 : cx - w / 2 - 2, cy - h / 2 - 2, cx, cy - h / 2 - 2,
                     left ? cx + w / 2 + 2 : cx - w / 2 - 2, cy, PAPER);
      break;
    case Expression::Love: {  // heart: two circles and a triangle
      const int r = w / 4 + 2;
      s.fillCircle(cx - r, cy - r / 2, r, INK);
      s.fillCircle(cx + r, cy - r / 2, r, INK);
      s.fillTriangle(cx - 2 * r, cy - r / 4, cx + 2 * r, cy - r / 4, cx, cy + 2 * r, INK);
      break;
    }
    case Expression::Confused:  // one eye smaller
      if (left) s.fillRoundRect(cx - w / 2, cy - h / 2, w, h, w / 3, INK);
      else s.fillRoundRect(cx - w / 3, cy - h / 4, w * 2 / 3, h / 2, w / 4, INK);
      break;
    case Expression::Neutral:
    default:
      s.fillRoundRect(cx - w / 2, cy - h / 2, w, h, w / 3, INK);
      break;
  }
}

void Face::draw(uint32_t nowMs) {
  if (!sprite_) return;
  auto& s = *sprite_;
  s.fillSprite(PAPER);

  const int W = s.width(), H = s.height();
  const int eyeW = W / 6, eyeH = H / 2;
  int h = eyeH;
  // Blink: squash the eyes for a moment every few seconds (not when sleepy or surprised).
  if (nowMs >= nextBlinkMs_) {
    const Expression e = expression_;
    if (e != Expression::Sleepy && e != Expression::Surprised) h = 8;
    if (nowMs >= nextBlinkMs_ + BLINK_MS) nextBlinkMs_ = nowMs + 2500 + (esp_random() % 3500);
  }
  const int cy = caption_[0] ? H / 2 - 12 : H / 2;
  drawEye(W / 2 - W / 5, cy, eyeW, h, true);
  drawEye(W / 2 + W / 5, cy, eyeW, h, false);

  if (caption_[0]) {
    s.setTextColor(INK, PAPER);
    s.setTextDatum(lgfx::bottom_center);
    s.setFont(&fonts::Font2);
    s.drawString(caption_, W / 2, H - 6);
  }
  s.pushSprite(0, 0);
}
