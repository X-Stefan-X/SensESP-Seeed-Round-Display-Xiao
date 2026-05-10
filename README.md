# SensESP Seeed Round Display – XIAO ESP32C3

> Marine-Anzeigesystem für ein rundes 240×240-Touchscreen-Display auf Basis des Seeed XIAO ESP32C3. Verbindet sich mit einem Signal K Server und zeigt Motor-, Antriebs- und Batteriedaten in Echtzeit an.

---

## Überblick

Dieses Projekt implementiert eine vollständige Schiffsanzeige für Elektro- oder Hybridantriebe. Das runde LVGL-Display zeigt Drosselstellung, Drehzahl, Temperaturen und Batteriestatus an und erlaubt gleichzeitig die bidirektionale Steuerung über einen Signal K Server.

Das UI wurde mit **SquareLine Studio 1.6.0** entworfen und als LVGL 9-Code generiert. Die Kommunikation mit dem Signal K Server erfolgt über die **SensESP**-Bibliothek.

---

## Hardware-Anforderungen

| Komponente | Details |
|---|---|
| Mikrocontroller | Seeed Studio XIAO ESP32C3 |
| Display | GC9A01-Treiber, rund, 240 × 240 Pixel, TFT-LCD |
| Touch | CHSC6X kapazitiver Sensor (I2C) |

### Pinbelegung

| Signal | Pin |
|---|---|
| SPI SCLK | 8 |
| SPI MISO | 9 |
| SPI MOSI | 10 |
| Display CS | 3 |
| Display DC | 5 |
| Backlight | 21 |
| Touch INT | D7 (aktiv LOW) |

SPI-Taktfrequenz: 40 MHz (Schreiben), 20 MHz (Lesen)

---

## Software-Abhängigkeiten

| Bibliothek | Version |
|---|---|
| [SensESP](https://github.com/SignalK/SensESP) | >= 3.0.0-beta.6, < 4.0.0-alpha.1 |
| [LVGL](https://lvgl.io) | ^9.3.0 |
| [TFT_eSPI (Seeed-Fork)](https://github.com/Seeed-Projects/SeeedStudio_TFT_eSPI) | aktuell |
| [Seeed_Arduino_RoundDisplay](https://github.com/Seeed-Studio/Seeed_Arduino_RoundDisplay) | aktuell |
| I2C BM8563 RTC | ^1.0.4 |
| PlatformIO | aktuell |

---

## Funktionsumfang

### Anzeigen

| Anzeige | Beschreibung |
|---|---|
| **Batteriebogen (unten)** | Batteriespannung in Prozent (9 V = 0 %, 14,7 V = 100 %) mit Farbkodierung |
| **Drosselbogen (oben)** | Drosselstellung 0–100 %, bidirektional steuerbar |
| **RPM-Label** | Motordrehzahl in U/min (Hz → RPM) |
| **Motortemperatur-Balken** | Temperatur 0–125 °C, Farbverlauf Grün → Orange → Rot |
| **Kühlmitteltemperatur-Balken** | Temperatur 0–125 °C, Farbverlauf Grün → Orange → Rot |
| **Status-Indikator** | Grün = bereit, Rot blinkend = Fehler |
| **ECO-Label** | Wird eingeblendet, wenn ECO-Modus aktiv |

### Schalter

| Schalter | Funktion |
|---|---|
| **Motor Ein/Aus** | Schaltet den Antrieb (Rot = Aus, Grün = Ein) |
| **Rekuperation** | Aktiviert / deaktiviert Rekuperationsmodus |

### Farbkodierung Batterie

| Ladezustand | Farbe |
|---|---|
| < 10 % | Rot |
| 10–25 % | Orange |
| 25–80 % | Grün |
| > 80 % | Blau |

---

## Signal K Pfade

### Empfangene Pfade (Listeners)

| Signal K Pfad | Einheit | Widget |
|---|---|---|
| `electrical.motor.remote.Batlevel` | % | Batteriebogen |
| `propulsion.main.revolutions` | Hz | RPM-Label |
| `propulsion.main.temp` | Kelvin | Motortemperatur-Balken |
| `sensors.temperature.Coolant.Temperature.0` | Kelvin | Kühlmitteltemperatur-Balken |
| `propulsion.main.ready` | bool | Status-Indikator (grün) |
| `propulsion.main.error` | bool | Status-Indikator (rot, blinkend) |
| `electrical.motor.eco.enable` | bool | ECO-Label |

### Gesendete Pfade (Outputs)

| Signal K Pfad | Einheit | Quelle |
|---|---|---|
| `propulsion/main/throttle` | % | Drosselbogen (Touch) |

---

## Einrichtung & Inbetriebnahme

### 1. Repository klonen

```bash
git clone https://github.com/X-Stefan-X/SensESP-Seeed-Round-Display-Xiao.git
cd SensESP-Seeed-Round-Display-Xiao
```

### 2. PlatformIO öffnen

Öffne den Ordner in **Visual Studio Code** mit installiertem PlatformIO-Plugin. Das Standard-Environment ist `arduino_xiao_esp32c3`.

### 3. Firmware flashen

```bash
pio run --target upload
```

### 4. Seriellen Monitor öffnen

```bash
pio device monitor
```

Baudrate: **115.200 Baud**

### 5. WiFi & Signal K Server konfigurieren

Nach dem ersten Start öffnet das Gerät einen WLAN-Hotspot:

- **SSID**: `my-sensesp-round-display`
- **Passwort**: `thisisfine`

Verbinde dich mit dem Hotspot und öffne im Browser `http://192.168.4.1`. Dort kannst du:
- WLAN-Zugangsdaten eingeben
- Signal K Server-Adresse und Port einstellen

---

## Build-Konfiguration

### Verfügbare Environments

| Environment | Plattform |
|---|---|
| `arduino_xiao_esp32c3` *(Standard)* | Seeed XIAO ESP32C3, Arduino |
| `arduino_esp32` | Generischer ESP32, Arduino |
| `pioarduino_esp32c3` | ESP32C3, PIO Arduino |
| `espidf_esp32c3` | ESP32C3, ESP-IDF |
| `shesp32`, `halmet`, `halser` | Custom Hardware-Varianten |

### Build-Flags (wichtige Einstellungen)

```ini
-D USER_SETUP_LOADED=1
-D GC9A01_DRIVER=1
-D SPI_FREQUENCY=40000000
-D LV_COLOR_DEPTH=16
```

---

## Projektstruktur

```
├── src/
│   ├── main.cpp                  # SensESP-Initialisierung & Signal K Integration
│   ├── ui.cpp / ui.h             # LVGL UI-Setup
│   ├── driver.h                  # Hardware-Definitionen
│   └── lv_xiao_round_screen.h    # Display- & Touch-Treiber
├── ui/                           # Generierter LVGL-Code (SquareLine Studio)
│   ├── ui.c / ui.h
│   ├── ui_Screen1.c / ui_Screen1.h
│   ├── ui_events.c / ui_events.h
│   ├── ui_helpers.c / ui_helpers.h
│   └── assets/
├── squareline/                   # SquareLine Studio Projektdateien
│   ├── SensESP Round Display.spj
│   └── SensESP Round Display.sll
├── lv_conf.h                     # LVGL 9.3 Konfiguration
└── platformio.ini                # Build-Konfiguration
```

---

## UI-Anpassung mit SquareLine Studio

Das Display-Layout wurde mit **SquareLine Studio 1.6.0** entworfen. Die Projektdatei liegt unter `squareline/SensESP Round Display.spj`.

Nach Änderungen im Designer muss der Code neu exportiert werden:

1. SquareLine Studio öffnen und `.spj`-Datei laden
2. Änderungen vornehmen
3. **Export → Export UI Files** nach `ui/` exportieren
4. Projekt neu kompilieren und flashen

---

## Weiterführende Links

- [SensESP Dokumentation](https://signalk.org/SensESP/)
- [Signal K Protokoll](https://signalk.org/)
- [LVGL Dokumentation](https://docs.lvgl.io/9.3/)
- [Seeed XIAO ESP32C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)
- [Seeed Round Display Wiki](https://wiki.seeedstudio.com/get_start_round_display/)
- [SquareLine Studio](https://squareline.io/)
