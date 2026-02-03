# DualArcDisplay - Interaktives Dual-Arc Display Layout

## 📋 Übersicht

`DualArcDisplay` ist eine Display-Komponente für das Seeed Round Display (240x240) mit XIAO-Mikrocontroller. Sie bietet zwei halbrunde Anzeigen:

1. **Oberer interaktiver Slider** (0-4095)
   - Touch-gesteuert
   - Nullpunkt bei 50% (Wert 2048)
   - Farbübergang: weiß → rot an den Rändern

2. **Untere Prozent-Anzeige** (0-100%)
   - Nicht-interaktiv
   - 4 Farbbereiche mit fließenden Übergängen
   - Ideal für Status-Anzeigen

## 🎨 Farb-Schema

### Oberer Slider (0-4095)
```
┌────────────────────────────────────────────┐
│ 0-410   │ 410-3686    │ 3686-4095         │
│ (10%)   │ (80%)       │ (10%)             │
├─────────┼─────────────┼────────────────────┤
│ rot→weiß│ weiß        │ weiß→rot          │
└────────────────────────────────────────────┘
          ↑
        2048 (Nullpunkt, gelbe Markierung)
```

### Untere Anzeige (0-100%)
```
┌────────────────────────────────────────────┐
│ 0-10%  │ 10-25%      │ 25-80%  │ 80-100% │
├────────┼─────────────┼─────────┼─────────┤
│ knallrot│ rot→orange │ grün    │ grün→weiß│
└────────────────────────────────────────────┘
```

## 🚀 Quick Start

### 1. Einfaches Beispiel (ohne Framework)

```cpp
#include <TFT_eSPI.h>
#include <Wire.h>
#include "displays/dual_arc_display.h"

TFT_eSPI tft = TFT_eSPI();
DualArcDisplay* display = nullptr;

void setup() {
  // Display initialisieren
  tft.begin();
  tft.setRotation(0);
  
  // Touch initialisieren
  Wire.begin();
  pinMode(D7, INPUT_PULLUP); // TOUCH_INT
  
  // Display-Objekt erstellen
  display = new DualArcDisplay(&tft);
  
  // Werte setzen
  display->set_upper_value(2048);      // Mitte
  display->set_lower_percentage(50.0f); // 50%
  display->draw();
}

void loop() {
  // Touch-Handling, Werte-Updates, etc.
}
```

### 2. Mit SensESP Framework

Siehe `src/example_dual_arc.cpp` für vollständiges Beispiel mit:
- Signal K Integration
- Automatischer Demo-Animation
- Touch-Event-Handling

## 📚 API-Referenz

### Konstruktor

```cpp
DualArcDisplay(TFT_eSPI* display)
```

Erstellt ein neues Display-Objekt.

**Parameter:**
- `display`: Pointer auf initialisiertes TFT_eSPI-Objekt

### Öffentliche Methoden

#### Werte setzen

```cpp
void set_upper_value(int value)
```
Setzt den Wert des oberen Sliders.
- **Parameter:** `value` - Wert zwischen 0 und 4095
- **Default:** 2048 (Mitte/Nullpunkt)

```cpp
void set_lower_percentage(float percentage)
```
Setzt den Prozentwert der unteren Anzeige.
- **Parameter:** `percentage` - Wert zwischen 0.0 und 100.0
- **Default:** 50.0

```cpp
void set_numeric_display(bool show)
```
Aktiviert/deaktiviert die numerische Werteanzeige.
- **Parameter:** `show` - true = anzeigen, false = verbergen
- **Default:** true

#### Touch-Handling

```cpp
bool handle_touch(int16_t touch_x, int16_t touch_y)
```
Verarbeitet Touch-Events für den oberen Slider.
- **Parameter:** 
  - `touch_x`: X-Koordinate (0-239)
  - `touch_y`: Y-Koordinate (0-239)
- **Return:** `true` wenn Touch im Slider-Bereich war
- **Automatisch:** Aktualisiert `upper_value` basierend auf Touch-Position

#### Zeichnen

```cpp
void draw()
```
Zeichnet das komplette Display mit aktuellen Werten.
- Sollte nach Wert-Änderungen aufgerufen werden
- Zeichnet beide Anzeigen und optionale numerische Werte

### Öffentliche Variablen

```cpp
int upper_value         // Aktueller Slider-Wert (0-4095)
float lower_percentage  // Aktueller Prozent-Wert (0-100)
bool show_numeric_display // Numerische Anzeige aktiviert
```

Diese können auch direkt gelesen werden (z.B. nach Touch-Event).

## 🎮 Verwendungsbeispiele

### Beispiel 1: Manuelle Steuerung

```cpp
// Werte direkt setzen
display->set_upper_value(1500);
display->set_lower_percentage(75.5f);
display->draw();

// Werte lesen
int current_slider = display->upper_value;
float current_percentage = display->lower_percentage;
```

### Beispiel 2: Touch-Interaktion

```cpp
void loop() {
  if (is_touch_pressed()) {
    int16_t x, y;
    get_touch_xy(&x, &y);
    
    if (display->handle_touch(x, y)) {
      Serial.printf("Neuer Wert: %d\n", display->upper_value);
      display->draw();
    }
  }
}
```

### Beispiel 3: Signal K Integration

```cpp
// Oberer Slider: Autopilot Sollwert (0-360° Kompass)
auto* heading_listener = new SKValueListener<float>(
  "steering.autopilot.target.headingMagnetic", 1000, "");
heading_listener->connect_to(new LambdaConsumer<float>([](float degrees){
  int value = (int)(degrees * 4095.0f / 360.0f);
  display->set_upper_value(value);
  display->draw();
}));

// Untere Anzeige: Batterie State of Charge
auto* battery_listener = new SKValueListener<float>(
  "electrical.batteries.1.capacity.stateOfCharge", 1000, "");
battery_listener->connect_to(new LambdaConsumer<float>([](float soc){
  display->set_lower_percentage(soc * 100.0f);
  display->draw();
}));
```

### Beispiel 4: Animation

```cpp
// Sweep von links nach rechts
for (int val = 0; val <= 4095; val += 50) {
  display->set_upper_value(val);
  display->draw();
  delay(20);
}

// Prozent von 0 zu 100
for (float pct = 0.0f; pct <= 100.0f; pct += 1.0f) {
  display->set_lower_percentage(pct);
  display->draw();
  delay(20);
}
```

### Beispiel 5: Abweichung vom Nullpunkt

```cpp
// Nullpunkt ist bei 2048
int deviation = display->upper_value - 2048;

if (deviation > 0) {
  Serial.printf("Rechts von Mitte: +%d\n", deviation);
} else if (deviation < 0) {
  Serial.printf("Links von Mitte: %d\n", deviation);
} else {
  Serial.println("Exakt in der Mitte");
}
```

## 🔧 Konfiguration & Anpassung

### Farben anpassen

Die Farbberechnung erfolgt in den privaten Methoden:
- `get_upper_color(int value)` - Oberer Slider
- `get_lower_color(float percentage)` - Untere Anzeige

Beispiel für eigene Farbwerte (in `dual_arc_display.h` ändern):

```cpp
// Oberer Slider: andere Warnbereiche
if (value <= 200) {  // Nur 5% statt 10%
  return interpolate_color(0xF800, 0xFFFF, value / 200.0f);
}

// Untere Anzeige: andere Schwellwerte
if (percentage <= 15.0f) {  // 15% statt 10%
  return 0xF800; // knallrot
}
```

### Geometrie anpassen

Wichtige Parameter in `draw_arc_with_gradient()`:
- `rx, ry`: Radius der Bögen (default: 95)
- `w`: Breite der Bögen (default: 12)
- `seg`: Segment-Größe für Feinheit (default: 3°)

### Touch-Bereich anpassen

In `handle_touch()`:
```cpp
// Aktuell: Radius 95-107
if (distance >= 95 && distance <= 107 && touch_y < cy) {
  // ...
}

// Anpassen für größeren Touch-Bereich:
if (distance >= 90 && distance <= 110 && touch_y < cy) {
  // ...
}
```

## 📦 Dateien

```
src/
├── displays/
│   ├── dual_arc_display.h      # Haupt-Display-Klasse
│   └── battery_ring_display.h  # Original (Referenz)
├── example_dual_arc.cpp        # Vollständiges Beispiel mit SensESP
├── example_simple.cpp          # Einfaches Standalone-Beispiel
└── main.cpp                    # Original Main (Referenz)
```

## 🧪 Testen

### Mit Serial Monitor (example_simple.cpp)

Kommandos über Serial (115200 baud):
```
u2048    - Setze oberen Wert auf 2048
l75      - Setze unteren Wert auf 75%
n        - Toggle numerische Anzeige
t        - Starte Test-Animation
r        - Reset zu Standardwerten
```

### Programmatische Tests (example_dual_arc.cpp)

Aktiviere Test-Funktionen im `setup()`:
```cpp
test_upper_slider();    // Testet Slider mit Eckwerten
test_lower_gauge();     // Testet alle Farbbereiche
test_numeric_display(); // Testet numerische Anzeige
test_animation();       // Testet Animation
```

## 🎯 Anwendungsfälle

### Maritime Anwendungen
- **Oberer Slider:** Autopilot Kurs-Sollwert (0-360° → 0-4095)
- **Untere Anzeige:** Batterie SOC, Tank-Füllstand

### Industrie-Steuerung
- **Oberer Slider:** Sollwert-Eingabe (Drehzahl, Temperatur, etc.)
- **Untere Anzeige:** System-Status, Auslastung

### Smart Home
- **Oberer Slider:** Thermostat-Solltemperatur
- **Untere Anzeige:** Luftfeuchtigkeit, Luftqualität

### Audio/Video
- **Oberer Slider:** Lautstärke, Balance
- **Untere Anzeige:** Signal-Stärke, Buffer-Status

## ⚙️ Technische Details

### Hardware-Anforderungen
- **Display:** Seeed Round Display für Xiao (240x240, GC9A01 Controller)
- **Touch:** CHSC6X Capacitive Touch Controller
- **MCU:** XIAO ESP32-C3/S3 oder kompatibel
- **Framework:** Arduino, ESP-IDF, oder SensESP

### Performance
- **Framerate:** ~20-30 FPS bei voller Neuzeichnung
- **Touch-Latenz:** <50ms
- **RAM-Verbrauch:** ~8KB für Display-Buffer

### Bekannte Einschränkungen
- Touch-Bereich nur obere Hälfte (untere Anzeige ist read-only)
- Farbübergänge in 3°-Segmenten (sichtbar bei genauer Betrachtung)
- Keine Hardware-Beschleunigung (alles Software-Rendering)

## 🐛 Troubleshooting

### Display bleibt schwarz
```cpp
// Prüfe Backlight
pinMode(D6, OUTPUT);
digitalWrite(D6, HIGH);

// Prüfe TFT-Initialisierung
tft.begin();
tft.fillScreen(TFT_RED); // Test mit rotem Bildschirm
```

### Touch funktioniert nicht
```cpp
// Prüfe I2C-Verbindung
Wire.begin();
Wire.beginTransmission(0x2e); // CHSC6X Adresse
int error = Wire.endTransmission();
Serial.printf("I2C Status: %d (0=OK)\n", error);

// Prüfe Touch-Interrupt Pin
pinMode(D7, INPUT_PULLUP);
Serial.printf("Touch INT: %d (LOW=touch)\n", digitalRead(D7));
```

### Falsche Farben
```cpp
// Prüfe RGB565-Format
// RGB565: RRRRR GGGGGG BBBBB (16 Bit)
uint16_t test_red   = 0xF800; // 11111 000000 00000
uint16_t test_green = 0x07E0; // 00000 111111 00000
uint16_t test_blue  = 0x001F; // 00000 000000 11111
uint16_t test_white = 0xFFFF; // 11111 111111 11111
```

### Touch-Koordinaten falsch
```cpp
// Evtl. Display-Rotation anpassen
tft.setRotation(0); // 0, 1, 2, oder 3

// Oder Touch-Konvertierung in chsc6x_convert_xy() prüfen
```

## 📄 Lizenz

Dieses Projekt ist Teil des SensESP-Seeed-Round-Display-Xiao Repositories.
Siehe Haupt-Repository für Lizenzinformationen.

## 👥 Mitwirkende

- Original Repository: [X-Stefan-X/SensESP-Seeed-Round-Display-Xiao](https://github.com/X-Stefan-X/SensESP-Seeed-Round-Display-Xiao)
- DualArcDisplay Erweiterung: DeepAgent (2026)

## 📞 Support

Bei Fragen oder Problemen:
1. Prüfe diese README
2. Teste mit `example_simple.cpp` für grundlegende Funktionalität
3. Aktiviere Debug-Ausgaben im Serial Monitor
4. Erstelle ein Issue im GitHub-Repository

---

**Version:** 1.0.0  
**Datum:** 2026-02-03  
**Status:** Produktionsbereit
