/**
 * @file example_dual_arc.cpp
 * @brief Beispielcode für DualArcDisplay mit Touch-Interaktivität
 * 
 * Dieses Beispiel zeigt:
 * - Initialisierung des Displays
 * - Touch-gesteuerten oberen Slider
 * - Automatische Animation der unteren Anzeige
 * - Verwendung der API
 */

#include "sensesp.h"
#include "sensesp/sensors/analog_input.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_app_builder.h"
#include "sensesp/signalk/signalk_value_listener.h"

#include <TFT_eSPI.h>
#include <Wire.h>
#include "displays/dual_arc_display.h"

using namespace sensesp;

// Globale Objekte
DualArcDisplay* display = nullptr;
TFT_eSPI tft = TFT_eSPI();

// Touch-Konfiguration (CHSC6X)
#define CHSC6X_I2C_ID 0x2e
#define CHSC6X_READ_POINT_LEN 5
#define TOUCH_INT D7

// Touch-Hilfsfunktionen
bool is_touch_pressed() {
  if(digitalRead(TOUCH_INT) != LOW) {
    delay(1);
    if(digitalRead(TOUCH_INT) != LOW)
      return false;
  }
  return true;
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

/**
 * @brief Testet den oberen Slider mit vordefinierten Werten
 */
void test_upper_slider() {
  Serial.println("\n=== Test: Oberer Slider ===");
  
  int test_values[] = {0, 410, 2048, 3686, 4095};
  const char* labels[] = {"Min (0)", "10% Links (410)", "Mitte/Nullpunkt (2048)", 
                          "90% Rechts (3686)", "Max (4095)"};
  
  for (int i = 0; i < 5; i++) {
    Serial.printf("Setze %s: %d\n", labels[i], test_values[i]);
    display->set_upper_value(test_values[i]);
    display->draw();
    delay(2000);
  }
}

/**
 * @brief Testet die untere Anzeige mit vordefinierten Werten
 */
void test_lower_gauge() {
  Serial.println("\n=== Test: Untere Anzeige ===");
  
  float test_percentages[] = {0.0f, 5.0f, 10.0f, 20.0f, 50.0f, 80.0f, 95.0f, 100.0f};
  const char* labels[] = {"0% (rot)", "5% (rot)", "10% (rot->orange)", 
                          "20% (orange)", "50% (grün)", 
                          "80% (grün->weiß)", "95% (hellgrün)", "100% (weiß)"};
  
  for (int i = 0; i < 8; i++) {
    Serial.printf("Setze %s: %.1f%%\n", labels[i], test_percentages[i]);
    display->set_lower_percentage(test_percentages[i]);
    display->draw();
    delay(2000);
  }
}

/**
 * @brief Testet die numerische Anzeige
 */
void test_numeric_display() {
  Serial.println("\n=== Test: Numerische Anzeige ===");
  
  Serial.println("Mit numerischer Anzeige:");
  display->set_numeric_display(true);
  display->set_upper_value(2500);
  display->set_lower_percentage(65.0f);
  display->draw();
  delay(3000);
  
  Serial.println("Ohne numerische Anzeige:");
  display->set_numeric_display(false);
  display->draw();
  delay(3000);
  
  // Zurück aktivieren
  display->set_numeric_display(true);
}

/**
 * @brief Simuliert eine animierte Sweep-Sequenz
 */
void test_animation() {
  Serial.println("\n=== Test: Animation ===");
  
  // Oberer Slider von links nach rechts
  Serial.println("Sweep oberer Slider...");
  for (int val = 0; val <= 4095; val += 50) {
    display->set_upper_value(val);
    display->draw();
    delay(10);
  }
  
  // Untere Anzeige von 0 zu 100%
  Serial.println("Sweep untere Anzeige...");
  display->set_upper_value(2048); // Zurück zur Mitte
  for (float pct = 0.0f; pct <= 100.0f; pct += 1.0f) {
    display->set_lower_percentage(pct);
    display->draw();
    delay(20);
  }
}

// Setup-Funktion
void setup() {
  SetupLogging(ESP_LOG_DEBUG);
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== DualArcDisplay Beispiel ===");
  
  // SensESP App initialisieren
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
    ->set_hostname("dual-arc-display-demo")
    ->set_sk_server("demo.signalk.org", 80)
    ->get_app();
  
  // Display initialisieren
  Serial.println("Initialisiere TFT Display...");
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  
  // Touch initialisieren
  Serial.println("Initialisiere Touch Controller...");
  pinMode(TOUCH_INT, INPUT_PULLUP);
  Wire.begin();
  
  // Display-Objekt erstellen
  display = new DualArcDisplay(&tft);
  
  Serial.println("Display bereit!");
  
  // === OPTIONALE TESTS (auskommentieren zum Aktivieren) ===
  // test_upper_slider();
  // test_lower_gauge();
  // test_numeric_display();
  // test_animation();
  
  // Initiale Werte setzen
  display->set_upper_value(2048);  // Mitte
  display->set_lower_percentage(50.0f);
  display->draw();
  
  // === TOUCH-HANDLER ===
  // Prüft alle 50ms auf Touch-Events
  event_loop()->onRepeat(50, [](){
    if (is_touch_pressed()) {
      int16_t touch_x, touch_y;
      get_touch_xy(&touch_x, &touch_y);
      
      if (display->handle_touch(touch_x, touch_y)) {
        Serial.printf("Touch: Neuer Slider-Wert: %d\n", display->upper_value);
        display->draw();
      }
    }
  });
  
  // === AUTOMATISCHE DEMO-ANIMATION (optional) ===
  // Simuliert sich ändernde Werte für die untere Anzeige
  event_loop()->onRepeat(100, [](){
    static float demo_percentage = 0.0f;
    static bool increasing = true;
    
    if (increasing) {
      demo_percentage += 0.5f;
      if (demo_percentage >= 100.0f) {
        demo_percentage = 100.0f;
        increasing = false;
      }
    } else {
      demo_percentage -= 0.5f;
      if (demo_percentage <= 0.0f) {
        demo_percentage = 0.0f;
        increasing = true;
      }
    }
    
    display->set_lower_percentage(demo_percentage);
    display->draw();
  });
  
  // === SIGNAL K INTEGRATION (optional) ===
  // Beispiel: Verbinde mit Signal K Datenpfaden
  /*
  int listen_delay = 1000;
  
  // Oberer Slider: Steuerungswert (z.B. Sollwert)
  auto* setpoint_listener = new SKValueListener<float>(
    "steering.autopilot.target.headingMagnetic", 
    listen_delay, 
    "steering/autopilot/target"
  );
  setpoint_listener->connect_to(new LambdaConsumer<float>([](float input){
    // Konvertiere Signal K Wert zu 0-4095 Bereich
    int value = (int)(input * 4095.0f / 360.0f); // Beispiel: 0-360° zu 0-4095
    display->set_upper_value(value);
  }));
  
  // Untere Anzeige: Prozentwert (z.B. Batterie, Tank, etc.)
  auto* percentage_listener = new SKValueListener<float>(
    "electrical.batteries.1.capacity.stateOfCharge", 
    listen_delay, 
    "electrical/batteries/stateOfCharge"
  );
  percentage_listener->connect_to(new LambdaConsumer<float>([](float input){
    display->set_lower_percentage(input * 100.0f); // 0.0-1.0 zu 0-100%
  }));
  */
  
  Serial.println("\n=== Steuerung ===");
  Serial.println("- Berühre den oberen Halbring um den Slider zu steuern");
  Serial.println("- Die untere Anzeige zeigt eine Demo-Animation");
  Serial.println("- Werte werden auf Serial Monitor ausgegeben");
  
  // Event Loop starten
  while (true) {
    loop();
  }
}

void loop() { 
  event_loop()->tick(); 
}
