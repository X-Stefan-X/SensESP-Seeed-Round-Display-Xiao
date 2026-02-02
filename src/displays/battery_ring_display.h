#pragma once
#include <sensesp.h>
#include <sensesp/ui/graphic_display.h>
#include <SensESPDebug.h>

class BatteryRingDisplay : public Drawable {
 public:
  BatteryRingDisplay(int16_t x, int16_t y, int16_t width, int16_t height,
                     float* battery_value, float* voltage_value)
      : Drawable(x, y, width, height),
        battery_value_(battery_value),
        voltage_value_(voltage_value) {}

  void draw(Adafruit_GFX* gfx) override {
    int16_t center_x = x + width / 2;
    int16_t center_y = y + height / 2;
    int16_t ring_radius = min(width, height) / 2 - 5;

    gfx->setTextSize(1);
    gfx->setTextColor(TFT_WHITE);

    // Unterer Halbring: Batterieladestand (0-100%)
    drawHalfRing(gfx, center_x, center_y + ring_radius / 3, ring_radius / 2,
                 *battery_value_ / 100.0, 180, 360, TFT_GREEN, TFT_RED);

    // Wert-Label unten
    gfx->setCursor(center_x - 20, center_y + ring_radius / 3 + ring_radius / 2 + 5);
    gfx->printf("BATT:%.0f%%", *battery_value_);

    // Oberer Ring mit Mittelstrich: Spannung
    drawRingWithCenterLine(gfx, center_x, center_y - ring_radius / 3,
                          ring_radius / 2, *voltage_value_ / 14.4, TFT_CYAN, TFT_RED);

    // Wert-Label oben
    gfx->setCursor(center_x - 25, center_y - ring_radius / 3 - ring_radius / 2 - 15);
    gfx->printf("V:%.1f", *voltage_value_);
  }

 private:
  float* battery_value_;
  float* voltage_value_;

  void drawHalfRing(Adafruit_GFX* gfx, int16_t cx, int16_t cy, int16_t r,
                   float value, int16_t start_angle, int16_t end_angle,
                   uint16_t fill_color, uint16_t empty_color) {
    // Leerer Halbring
    gfx->drawArc(cx, cy, r, r, start_angle, end_angle, 8, empty_color);
    
    // Gefüllter Teil
    int16_t fill_end = start_angle + (end_angle - start_angle) * value;
    gfx->fillArc(cx, cy, r, r, start_angle, fill_end, 8, fill_color);
  }

  void drawRingWithCenterLine(Adafruit_GFX* gfx, int16_t cx, int16_t cy, int16_t r,
                             float value, uint16_t fill_color, uint16_t empty_color) {
    // Voller Ring leeren
    gfx->drawCircle(cx, cy, r, empty_color);
    gfx->drawCircle(cx, cy, r - 8, empty_color);
    
    // Gefüllter Bogen
    gfx->fillArc(cx, cy, r, r, 0, 360 * value, 8, fill_color);
    
    // Wichtiger Mittelstrich (0-Position)
    gfx->drawLine(cx, cy - r, cx, cy - r + 8, TFT_WHITE);
    gfx->drawLine(cx, cy + r - 8, cx, cy + r, TFT_WHITE);
  }
};
