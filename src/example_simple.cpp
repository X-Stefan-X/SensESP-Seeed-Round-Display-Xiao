/**
 * @file example_simple.cpp
 * @brief Einfaches Standalone-Beispiel ohne SensESP-Framework
 * 
 * Dieses minimal Beispiel zeigt die Grundfunktionalität:
 * - Display-Initialisierung
 * - Touch-Interaktion
 * - Manuelle Wertsteuerung
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Wire.h>
#include "displays/dual_arc_display.h"

// Hardware-Konfiguration
#define TOUCH_INT D7
#define XIAO_BL D6
#define CHSC6X_I2C_ID 0x2e
#define CHSC6X_READ_POINT_LEN 5

// Globale Objekte
TFT_eSPI tft = TFT_eSPI();
DualArcDisplay* display = nullptr;

// Touch-Status
bool is_touch_pressed() {
  return (digitalRead(TOUCH_INT) == LOW);
}

void get_touch_xy(int16_t* x, int16_t* y) {
  uint8_t temp[CHSC6X_READ_POINT_LEN] = {0};
  uint8_t read_len = Wire.requestFrom(CHSC6X_I2C_ID, CHSC6X_READ_POINT_LEN);
  if(read_len == CHSC6X_READ_POINT_LEN) {
    Wire.readBytes(temp, read_len);
    if (temp[0] == 0x01) {
      *x = temp[2];
      *y = temp[4];
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== DualArcDisplay - Einfaches Beispiel ===\n");
  
  // Backlight aktivieren
  pinMode(XIAO_BL, OUTPUT);
  digitalWrite(XIAO_BL, HIGH);
  
  // Display initialisieren
  Serial.println("[1/3] Initialisiere Display...");
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  // Touch initialisieren
  Serial.println("[2/3] Initialisiere Touch...");
  pinMode(TOUCH_INT, INPUT_PULLUP);
  Wire.begin();
  delay(100);
  
  // Display-Objekt erstellen
  Serial.println("[3/3] Erstelle Display-Objekt...");
  display = new DualArcDisplay(&tft);
  
  // Startwerte setzen
  display->set_upper_value(2048);      // Mitte (Nullpunkt)
  display->set_lower_percentage(50.0f); // 50%
  display->set_numeric_display(true);   // Zahlen anzeigen
  
  display->draw();
  
  Serial.println("\n✓ Setup abgeschlossen!\n");
  Serial.println("=== Bedienung ===");
  Serial.println("• Berühre den OBEREN Halbring zum Steuern");
  Serial.println("• Sende über Serial:");
  Serial.println("  - 'u####' : Oberen Wert setzen (u2048)");
  Serial.println("  - 'l##'   : Unteren Wert setzen (l75)");
  Serial.println("  - 'n'     : Toggle numerische Anzeige");
  Serial.println("  - 't'     : Test-Animation");
  Serial.println("  - 'r'     : Reset zu Standardwerten");
  Serial.println();
}

void loop() {
  static unsigned long last_update = 0;
  static unsigned long last_touch_check = 0;
  
  // === TOUCH-HANDLING (alle 50ms) ===
  if (millis() - last_touch_check > 50) {
    last_touch_check = millis();
    
    if (is_touch_pressed()) {
      int16_t touch_x, touch_y;
      get_touch_xy(&touch_x, &touch_y);
      
      if (display->handle_touch(touch_x, touch_y)) {
        Serial.printf("✓ Touch: Slider = %d (Abweichung: %+d)\n", 
                      display->upper_value, 
                      display->upper_value - 2048);
        display->draw();
        last_update = millis();
      }
    }
  }
  
  // === SERIAL-KOMMANDOS ===
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.startsWith("u")) {
      // Oberen Wert setzen: u1234
      int value = cmd.substring(1).toInt();
      display->set_upper_value(value);
      Serial.printf("✓ Oberer Wert: %d\n", value);
      display->draw();
      
    } else if (cmd.startsWith("l")) {
      // Unteren Wert setzen: l75
      float percentage = cmd.substring(1).toFloat();
      display->set_lower_percentage(percentage);
      Serial.printf("✓ Unterer Wert: %.1f%%\n", percentage);
      display->draw();
      
    } else if (cmd == "n") {
      // Toggle numerische Anzeige
      display->set_numeric_display(!display->show_numeric_display);
      Serial.printf("✓ Numerische Anzeige: %s\n", 
                    display->show_numeric_display ? "AN" : "AUS");
      display->draw();
      
    } else if (cmd == "t") {
      // Test-Animation
      Serial.println("\n=== Test-Animation ===");
      
      // Oberer Slider durchlaufen
      Serial.println("Sweep oberer Slider...");
      for (int val = 0; val <= 4095; val += 100) {
        display->set_upper_value(val);
        display->draw();
        delay(20);
      }
      
      // Zurück zur Mitte
      display->set_upper_value(2048);
      
      // Untere Anzeige durchlaufen
      Serial.println("Sweep untere Anzeige...");
      for (float pct = 0.0f; pct <= 100.0f; pct += 2.0f) {
        display->set_lower_percentage(pct);
        display->draw();
        delay(30);
      }
      
      Serial.println("✓ Animation abgeschlossen\n");
      
    } else if (cmd == "r") {
      // Reset
      display->set_upper_value(2048);
      display->set_lower_percentage(50.0f);
      display->set_numeric_display(true);
      Serial.println("✓ Reset zu Standardwerten\n");
      display->draw();
      
    } else {
      Serial.println("? Unbekanntes Kommando");
      Serial.println("Verfügbar: u####, l##, n, t, r");
    }
  }
  
  // Kleine Pause
  delay(10);
}
