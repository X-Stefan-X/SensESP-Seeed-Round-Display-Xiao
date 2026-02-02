#pragma once
#include <Adafruit_GFX.h>
#include <sensesp/ui/nvalues.h>

class BatteryRingDisplay : public NVValues<float, float> {
 public:
  BatteryRingDisplay(float* battery_soc, float* voltage)
      : NVValues<float, float>(2, battery_soc, voltage) {}

  virtual void draw(Adafruit_GFX* gfx) override {
    float soc = values[0];
    float volt = values[1];
    
    int16_t cx = 120, cy = 120;
    int16_t r_outer = 100;
    int16_t r_inner = 80;
    int16_t r_small = 50;

    gfx->fillScreen(TFT_BLACK);
    
    // Unterer Halbring: Battery SoC (180°-360°)
    drawArc(gfx, cx, cy + 20, r_outer, r_inner, 180, 360, soc / 100.0,
            TFT_GREEN, TFT_RED);
    
    // Label unten
    gfx->setTextColor(TFT_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(cx - 25, cy + 80);
    gfx->printf("BATT:%.0f%%", soc);

    // Oberer Ring mit 0°-Strich: Voltage (0°-360°)
    drawArc(gfx, cx, cy - 20, r_small, r_small - 12, 0, 360, volt / 14.4,
            TFT_CYAN, TFT_RED);
    
    // WICHTIGER 0° Mittelstrich
    gfx->drawLine(cx, cy - 40, cx, cy - 65, TFT_WHITE);
    gfx->drawLine(cx, cy + 35, cx, cy + 50, TFT_WHITE);
    
    // Label oben
    gfx->setCursor(cx - 20, cy - 110);
    gfx->printf("V:%.1f", volt);
  }

 private:
  void drawArc(Adafruit_GFX* gfx, int16_t cx, int16_t cy, int16_t r1, int16_t r2,
               int16_t start, int16_t end, float value, uint16_t fill_col,
               uint16_t empty_col) {
    // Leerer Ring
    for (int16_t r = r2; r <= r1; r += 2) {
      gfx->drawCircle(cx, cy, r, empty_col);
    }
    
    // Gefüllter Teil
    int16_t fill_end = start + (end - start) * value;
    gfx->fillArc(cx, cy, r1, r1, start, fill_end, r1 - r2, fill_col);
  }
};
