/**
 * @file dual_arc_display.h
 * @brief LVGL-basiertes Display mit interaktivem oberen Slider und unterer Anzeige
 * 
 * Dieses Display zeigt zwei halbrunde Komponenten:
 * 1. Oberer interaktiver Slider (0-4095, Touch-gesteuert)
 * 2. Untere Prozentanzeige (0-100%, 4 Farbbereiche)
 * 
 * @author DeepAgent
 * @date 2026-02-03
 */

#pragma once
#include <TFT_eSPI.h>
#include <math.h>

#define DEG2RAD 0.0174532925

/**
 * @class DualArcDisplay
 * @brief Hauptklasse für das Dual-Arc Display Layout
 */
class DualArcDisplay {
 public:
  // Öffentliche Werte
  int upper_value = 2048;          // Oberer Slider: 0-4095, default Mitte
  float lower_percentage = 50.0f;  // Untere Anzeige: 0-100%
  bool show_numeric_display = true; // Parameter für numerische Anzeige
  
  TFT_eSPI* tft = nullptr;
  
  /**
   * @brief Konstruktor
   * @param display Pointer auf TFT_eSPI Objekt
   */
  DualArcDisplay(TFT_eSPI* display) : tft(display) {}
  
  /**
   * @brief Setzt den Wert des oberen Sliders
   * @param value Wert zwischen 0 und 4095
   */
  void set_upper_value(int value) {
    upper_value = constrain(value, 0, 4095);
  }
  
  /**
   * @brief Setzt den Prozentwert der unteren Anzeige
   * @param percentage Wert zwischen 0 und 100
   */
  void set_lower_percentage(float percentage) {
    lower_percentage = constrain(percentage, 0.0f, 100.0f);
  }
  
  /**
   * @brief Aktiviert/deaktiviert die numerische Anzeige
   * @param show true = anzeigen, false = verbergen
   */
  void set_numeric_display(bool show) {
    show_numeric_display = show;
  }
  
  /**
   * @brief Verarbeitet Touch-Events für den oberen Slider
   * @param touch_x X-Koordinate des Touch-Points
   * @param touch_y Y-Koordinate des Touch-Points
   * @return true wenn Touch im Slider-Bereich war
   */
  bool handle_touch(int16_t touch_x, int16_t touch_y) {
    int16_t cx = 120, cy = 120;
    
    // Berechne Abstand vom Zentrum
    float dx = touch_x - cx;
    float dy = touch_y - cy;
    float distance = sqrt(dx * dx + dy * dy);
    
    // Prüfe ob Touch im oberen Slider-Bereich ist (Radius 95-107, obere Hälfte)
    if (distance >= 95 && distance <= 107 && touch_y < cy) {
      // Berechne Winkel (-180° bis 0°)
      float angle = atan2(dy, dx) * 180.0 / PI;
      
      // Konvertiere zu 0-180° (von links nach rechts)
      if (angle < -90) {
        angle = 180 + (angle + 180); // links: -180 bis -90 -> 0 bis 90
      } else if (angle >= -90 && angle <= 0) {
        angle = angle + 90; // rechts: -90 bis 0 -> 90 bis 180
      } else {
        return false; // untere Hälfte ignorieren
      }
      
      // Konvertiere Winkel zu Wert 0-4095
      upper_value = (int)(angle / 180.0 * 4095.0);
      upper_value = constrain(upper_value, 0, 4095);
      
      return true;
    }
    
    return false;
  }
  
  /**
   * @brief Hauptzeichnungsfunktion
   */
  void draw() {
    if (!tft) return;
    
    int16_t cx = 120, cy = 120;
    
    tft->fillScreen(TFT_BLACK);
    
    // === OBERER HALBRING: Interaktiver Slider (0-4095) ===
    draw_upper_slider(cx, cy);
    
    // === UNTERER HALBRING: Prozentanzeige (0-100%) ===
    draw_lower_gauge(cx, cy);
    
    // === NUMERISCHE ANZEIGEN ===
    if (show_numeric_display) {
      draw_numeric_values(cx, cy);
    }
  }
  
 private:
  
  /**
   * @brief Berechnet Farbe mit Übergang für oberen Slider
   * @param value Wert zwischen 0 und 4095
   * @return RGB565 Farbe
   */
  uint16_t get_upper_color(int value) {
    // Bereiche:
    // 0-409.5 (10%): weiß -> rot
    // 409.5-3685.5 (80%): weiß
    // 3685.5-4095 (10%): weiß -> rot
    
    if (value <= 410) {
      // Links: rot (0) -> weiß (410)
      float t = value / 410.0f;
      return interpolate_color(0xF800, 0xFFFF, t); // rot -> weiß
    } else if (value >= 3686) {
      // Rechts: weiß (3686) -> rot (4095)
      float t = (value - 3686) / 409.0f;
      return interpolate_color(0xFFFF, 0xF800, t); // weiß -> rot
    } else {
      // Mitte: weiß
      return 0xFFFF;
    }
  }
  
  /**
   * @brief Berechnet Farbe mit Übergang für untere Anzeige
   * @param percentage Wert zwischen 0 und 100
   * @return RGB565 Farbe
   */
  uint16_t get_lower_color(float percentage) {
    // Bereiche:
    // 0-10%: knallrot
    // 10-25%: rot -> dunkelorange
    // 25-80%: grün
    // 80-100%: grün -> hellgrün/weiß
    
    if (percentage <= 10.0f) {
      // 0-10%: knallrot
      return 0xF800; // RGB565: 11111 000000 00000
    } else if (percentage <= 25.0f) {
      // 10-25%: rot -> dunkelorange
      float t = (percentage - 10.0f) / 15.0f;
      return interpolate_color(0xF800, 0xFC00, t); // rot -> orange
    } else if (percentage <= 80.0f) {
      // 25-80%: grün
      return 0x07E0; // RGB565: 00000 111111 00000
    } else {
      // 80-100%: grün -> hellgrün/weiß
      float t = (percentage - 80.0f) / 20.0f;
      return interpolate_color(0x07E0, 0xFFFF, t); // grün -> weiß
    }
  }
  
  /**
   * @brief Interpoliert zwischen zwei RGB565 Farben
   * @param color1 Start-Farbe
   * @param color2 End-Farbe
   * @param t Interpolationsfaktor (0.0 - 1.0)
   * @return Interpolierte RGB565 Farbe
   */
  uint16_t interpolate_color(uint16_t color1, uint16_t color2, float t) {
    // Extrahiere RGB-Komponenten aus RGB565
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;
    
    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;
    
    // Interpoliere
    uint8_t r = r1 + (r2 - r1) * t;
    uint8_t g = g1 + (g2 - g1) * t;
    uint8_t b = b1 + (b2 - b1) * t;
    
    // Zurück zu RGB565
    return (r << 11) | (g << 5) | b;
  }
  
  /**
   * @brief Zeichnet den oberen interaktiven Slider
   */
  void draw_upper_slider(int16_t cx, int16_t cy) {
    // Berechne Füllgrad (0-4095 -> 0.0-1.0)
    float fill_ratio = upper_value / 4095.0f;
    
    // Zeichne Halbring mit Farbübergang (180° Bereich, oben)
    // Start bei 180° (links), Ende bei 0° (rechts)
    draw_arc_with_gradient(cx, cy, 180, 180, 95, 95, 12, fill_ratio, true);
    
    // Markierung bei 50% (Nullpunkt)
    int16_t mid_angle = 270; // 270° = oben Mitte
    float sx = cos((mid_angle - 90) * DEG2RAD);
    float sy = sin((mid_angle - 90) * DEG2RAD);
    int16_t x1 = sx * 85 + cx;
    int16_t y1 = sy * 85 + cy;
    int16_t x2 = sx * 107 + cx;
    int16_t y2 = sy * 107 + cy;
    tft->drawLine(x1, y1, x2, y2, TFT_YELLOW);
    tft->drawLine(x1-1, y1, x2-1, y2, TFT_YELLOW); // dicker
  }
  
  /**
   * @brief Zeichnet die untere Prozent-Anzeige
   */
  void draw_lower_gauge(int16_t cx, int16_t cy) {
    // Berechne Füllgrad (0-100 -> 0.0-1.0)
    float fill_ratio = lower_percentage / 100.0f;
    
    // Zeichne Halbring mit Farbübergang (180° Bereich, unten)
    // Start bei 0° (rechts), Ende bei 180° (links)
    draw_arc_with_gradient(cx, cy, 0, 180, 95, 95, 12, fill_ratio, false);
  }
  
  /**
   * @brief Zeichnet einen Bogen mit Farbverlauf
   */
  void draw_arc_with_gradient(int16_t x, int16_t y, int16_t start_angle, 
                               int16_t seg_count, int16_t rx, int16_t ry, 
                               int16_t w, float value, bool is_upper) {
    // Leerer Ring als Hintergrund
    draw_empty_arc(x, y, start_angle, seg_count, rx, ry, w, TFT_DARKGREY);
    
    // Gefüllter Teil mit Farbverlauf
    int16_t max_segs = seg_count * value;
    byte seg = 3;  // 3° Segmente für feineren Verlauf
    byte inc = 3;
    
    // Erster Segment-Start
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    int16_t x0 = sx * (rx - w) + x;
    int16_t y0 = sy * (ry - w) + y;
    int16_t x1 = sx * rx + x;
    int16_t y1 = sy * ry + y;
    
    // Zeichne Segmente mit individueller Farbe
    for (int i = start_angle; i < start_angle + seg * max_segs; i += inc) {
      float sx2 = cos((i + seg - 90) * DEG2RAD);
      float sy2 = sin((i + seg - 90) * DEG2RAD);
      int16_t x2 = sx2 * (rx - w) + x;
      int16_t y2 = sy2 * (ry - w) + y;
      int16_t x3 = sx2 * rx + x;
      int16_t y3 = sy2 * ry + y;
      
      // Berechne Farbe basierend auf Position
      uint16_t segment_color;
      if (is_upper) {
        // Oberer Slider: Berechne Wert basierend auf Winkel
        float angle_progress = (float)(i - start_angle) / (float)seg_count;
        int segment_value = (int)(angle_progress * 4095.0f);
        segment_color = get_upper_color(segment_value);
      } else {
        // Untere Anzeige: Berechne Prozent basierend auf Winkel
        float angle_progress = (float)(i - start_angle) / (float)seg_count;
        float segment_percentage = angle_progress * 100.0f;
        segment_color = get_lower_color(segment_percentage);
      }
      
      tft->fillTriangle(x0, y0, x1, y1, x2, y2, segment_color);
      tft->fillTriangle(x1, y1, x2, y2, x3, y3, segment_color);
      
      x0 = x2; y0 = y2; x1 = x3; y1 = y3;
    }
  }
  
  /**
   * @brief Zeichnet einen leeren Bogen (Hintergrund)
   */
  void draw_empty_arc(int16_t x, int16_t y, int16_t start_angle, 
                      int16_t seg_count, int16_t rx, int16_t ry, 
                      int16_t w, uint16_t color) {
    byte seg = 3;
    byte inc = 3;
    
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    int16_t x0 = sx * (rx - w) + x;
    int16_t y0 = sy * (ry - w) + y;
    int16_t x1 = sx * rx + x;
    int16_t y1 = sy * ry + y;
    
    for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {
      float sx2 = cos((i + seg - 90) * DEG2RAD);
      float sy2 = sin((i + seg - 90) * DEG2RAD);
      int16_t x2 = sx2 * (rx - w) + x;
      int16_t y2 = sy2 * (ry - w) + y;
      int16_t x3 = sx2 * rx + x;
      int16_t y3 = sy2 * ry + y;
      
      tft->fillTriangle(x0, y0, x1, y1, x2, y2, color);
      tft->fillTriangle(x1, y1, x2, y2, x3, y3, color);
      
      x0 = x2; y0 = y2; x1 = x3; y1 = y3;
    }
  }
  
  /**
   * @brief Zeichnet die numerischen Werteanzeigen
   */
  void draw_numeric_values(int16_t cx, int16_t cy) {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(MC_DATUM); // Middle Center
    
    // Oberer Wert (0-4095)
    tft->setTextSize(2);
    tft->drawString(String(upper_value), cx, cy - 40);
    
    tft->setTextSize(1);
    tft->drawString("(Sollwert)", cx, cy - 25);
    
    // Unterer Wert (0-100%)
    tft->setTextSize(2);
    char buffer[16];
    sprintf(buffer, "%.1f%%", lower_percentage);
    tft->drawString(String(buffer), cx, cy + 40);
    
    tft->setTextSize(1);
    tft->drawString("(Istwert)", cx, cy + 55);
    
    // Zusätzliche Info: Abweichung vom Nullpunkt (2048)
    int deviation = upper_value - 2048;
    tft->setTextSize(1);
    sprintf(buffer, "Dev: %+d", deviation);
    tft->drawString(String(buffer), cx, cy);
  }
};
