#pragma once
#include <TFT_eSPI.h>

#define DEG2RAD 0.0174532925

class BatteryRingDisplay {
 public:
  float battery_soc = 0.0f;
  float voltage = 0.0f;
  TFT_eSPI* tft = nullptr;
  
  BatteryRingDisplay(TFT_eSPI* display) : tft(display) {}
  
  void update_soc(float soc) {
    battery_soc = soc;
  }
  void update_volt(float volt) {
    voltage = volt;
  }

  void draw() {
    if (!tft) return;
    
    int16_t cx = 120, cy = 120;
    
    tft->fillScreen(TFT_BLACK);
    
    // === UNTERER HALBRING: Battery SoC (180°-360°) ===
    fillArc(cx, cy + 30, 180, 30, 95, 95, 12, battery_soc / 100.0, TFT_GREEN, TFT_DARKGREY);
    
    // Battery Label
    tft->setTextColor(TFT_WHITE);
    tft->setTextSize(1);
    tft->setCursor(cx - 25, cy + 95);
    tft->printf("BATT:%.0f%%", battery_soc);
    
    // === OBERER RING: Voltage (0°-360°) ===
    fillArc(cx, cy - 30, 0, 60, 55, 55, 12, voltage / 14.4, TFT_CYAN, TFT_DARKGREY);
    
    // === WICHTIGER 0°-STRICH (12 Uhr Position) ===
    tft->drawLine(cx-2, cy-65, cx+2, cy-65, TFT_WHITE);
    tft->drawLine(cx, cy-67, cx, cy-60, TFT_WHITE);
    
    // Voltage Label
    tft->setCursor(cx - 20, cy - 110);
    tft->printf("V:%.1fV", voltage);
  }
  
 private:
  // ECHTE TFT_eSPI fillArc Funktion aus dem offiziellen Beispiel!
  void fillArc(int16_t x, int16_t y, int16_t start_angle, int16_t seg_count,
               int16_t rx, int16_t ry, int16_t w, float value,
               uint16_t fill_col, uint16_t empty_col) {
    
    // Leerer Ring erstellen
    tft->drawCircle(x, y, rx, empty_col);
    tft->drawCircle(x, y, rx-w, empty_col);
    
    // Nur bis zum gefüllten Wert zeichnen
    int16_t max_segs = seg_count * value;
    
    byte seg = 6;  // 6° Segmente
    byte inc = 6;  // Alle 6°
    
    // Erster Segment-Start
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    int16_t x0 = sx * (rx - w) + x;
    int16_t y0 = sy * (ry - w) + y;
    int16_t x1 = sx * rx + x;
    int16_t y1 = sy * ry + y;
    
    // Gefüllte Segmente zeichnen
    for (int i = start_angle; i < start_angle + seg * max_segs; i += inc) {
      float sx2 = cos((i + seg - 90) * DEG2RAD);
      float sy2 = sin((i + seg - 90) * DEG2RAD);
      int16_t x2 = sx2 * (rx - w) + x;
      int16_t y2 = sy2 * (ry - w) + y;
      int16_t x3 = sx2 * rx + x;
      int16_t y3 = sy2 * ry + y;
      
      tft->fillTriangle(x0, y0, x1, y1, x2, y2, fill_col);
      tft->fillTriangle(x1, y1, x2, y2, x3, y3, fill_col);
      
      x0 = x2; y0 = y2; x1 = x3; y1 = y3;
    }
  }
};
