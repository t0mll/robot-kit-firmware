#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "BoardPins.h"

/// LovyanGFX config for the T-Display-S3: 1.9" ST7789 (170×320) on an 8-bit parallel bus.
class BrainDisplay : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 panel_;
  lgfx::Bus_Parallel8 bus_;
  lgfx::Light_PWM light_;

 public:
  BrainDisplay() {
    {
      auto cfg = bus_.config();
      cfg.freq_write = 20000000;
      cfg.pin_wr = pins::LCD_WR;
      cfg.pin_rd = pins::LCD_RD;
      cfg.pin_rs = pins::LCD_DC;
      cfg.pin_d0 = pins::LCD_D0;
      cfg.pin_d1 = pins::LCD_D1;
      cfg.pin_d2 = pins::LCD_D2;
      cfg.pin_d3 = pins::LCD_D3;
      cfg.pin_d4 = pins::LCD_D4;
      cfg.pin_d5 = pins::LCD_D5;
      cfg.pin_d6 = pins::LCD_D6;
      cfg.pin_d7 = pins::LCD_D7;
      bus_.config(cfg);
      panel_.setBus(&bus_);
    }
    {
      auto cfg = panel_.config();
      cfg.pin_cs = pins::LCD_CS;
      cfg.pin_rst = pins::LCD_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = 170;
      cfg.panel_height = 320;
      cfg.offset_x = 35;
      cfg.offset_y = 0;
      cfg.readable = false;
      cfg.invert = true;
      cfg.rgb_order = false;
      cfg.dlen_16bit = false;
      cfg.bus_shared = false;
      panel_.config(cfg);
    }
    {
      auto cfg = light_.config();
      cfg.pin_bl = pins::LCD_BACKLIGHT;
      cfg.invert = false;
      cfg.freq = 22000;
      cfg.pwm_channel = 7;  // the buzzer uses channel 6
      light_.config(cfg);
      panel_.setLight(&light_);
    }
    setPanel(&panel_);
  }
};
