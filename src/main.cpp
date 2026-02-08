#include <memory>

#include "sensesp.h"
#include "sensesp/sensors/analog_input.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_app_builder.h"
#include "sensesp/signalk/signalk_value_listener.h"

#include "I2C_BM8563.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "ui.h"

extern "C" {
  #include "../ui/ui.h"
}

using namespace sensesp;

// Timer Handle für Blinken
static lv_timer_t* blink_timer = NULL;
static bool blink_state = false;

// Blink Callback
void blink_cb(lv_timer_t* timer) {
    blink_state = !blink_state;
    if (blink_state) {
        lv_obj_set_style_bg_color(ui_PlState, lv_color_hex(0xFF8000), LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(ui_PlState, lv_color_hex(0x06480C), LV_PART_MAIN);
    }
}

void start_blinking() {
    if (!blink_timer) {
        blink_timer = lv_timer_create(blink_cb, 500, NULL);  // 500ms Intervall
    }
}

void stop_blinking() {
    if (blink_timer) {
        lv_timer_delete(blink_timer);
        blink_timer = NULL;
        blink_state = false;
    }
}

// The setup function performs one-time application initialization.
void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  // Construct the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
                    // Set a custom hostname for the app.
                    ->set_hostname("my-sensesp-round-display")
                    // Optionally, hard-code the WiFi and Signal K server
                    // settings. This is normally not needed.
                    //->set_wifi_client("My WiFi SSID", "my_wifi_password")
                    //->set_wifi_access_point("My AP SSID", "my_ap_password")
                    //->set_sk_server("demo.signalk.org", 8)
                    ->get_app();

    setup_ui();  // UI-Setup

int listendelay = 1000;


//ARC unten Batterie 
auto* bat_listener = new SKValueListener<float>("electrical.motor.remote.Batlevel", listendelay, "electrical/batteries/voltage");
bat_listener->connect_to(new LambdaConsumer<float>([](float voltage) {
    // 9V = 0, 14.7V = 4095
    int arc_value = (int)((voltage - 9.0) / (14.7 - 9.0) * 4095);
    arc_value = constrain(arc_value, 0, 4095);
    lv_arc_set_value(ui_ARCbatLevel, arc_value);
    
    // Farbverlauf
    lv_color_t color;
    if (arc_value < 410) {            // < 10% (< 9.6V)
        color = lv_color_hex(0xFF0000);  // Rot
    } else if (arc_value < 1024) {    // 10-25% (9.6 - 10.4V)
        color = lv_color_hex(0xFF8000);  // Orange
    } else if (arc_value < 3276) {    // 25-80% (10.4 - 13.6V)
        color = lv_color_hex(0x00FF00);  // Grün
    } else {                          // 80-100% (> 13.6V)
        color = lv_color_hex(0x0080FF);  // Blau
    }
    lv_obj_set_style_arc_color(ui_ARCbatLevel, color, 
                                LV_PART_INDICATOR | LV_STATE_DEFAULT);
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1fV", voltage);
    lv_label_set_text(ui_LBLunten, buf);
}));


//ARC oben Throttle
auto* throttle_out = new SKOutput<float>("propulsion.main.throttle", "propulsion/main/throttle");

// Event Callback registrieren: bei jeder Änderung am Arc
lv_obj_add_event_cb(ui_ARCoben, [](lv_event_t* e) {
    // Pointer auf SKOutput aus user_data holen
    auto* output = (SKOutput<float>*)lv_event_get_user_data(e);
    
    int raw = lv_arc_get_value(ui_ARCoben);  // 0-4095
    float value = (float)raw / 4095.0 * 100.0;  // z.B. auf 0-100% umrechnen
    
    // Label aktualisieren
    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f%%", value);
    lv_label_set_text(ui_LBLoben, buf);
    
    // An SignalK senden
    output->set(value);
    
}, LV_EVENT_VALUE_CHANGED, throttle_out);

// Drehzahl 
auto* rpm_listener = new SKValueListener<float>(
    "propulsion.main.revolutions", 1000);  // SignalK Pfad anpassen!

rpm_listener->connect_to(new LambdaConsumer<float>([](float hz) {
    float rpm = hz * 60.0;  // Hz → RPM
    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f", rpm);
    lv_label_set_text(ui_lblRPM, buf);
}));



// Switch Event: Blinken starten
lv_obj_add_event_cb(ui_SWon, [](lv_event_t* e) {
    start_blinking();
}, LV_EVENT_VALUE_CHANGED, NULL);

// Motor bereit
auto* ready_listener = new SKValueListener<bool>(
    "propulsion.main.ready", 1000);

ready_listener->connect_to(new LambdaConsumer<bool>([](bool ready) {
    if (ready) {
        stop_blinking();
        // Hellgrün
        lv_obj_set_style_bg_color(ui_PlState, lv_color_hex(0x00FF00), LV_PART_MAIN);
    } else {
        stop_blinking();
        // Ursprungsfarbe
        lv_obj_set_style_bg_color(ui_PlState, lv_color_hex(0x06480C), LV_PART_MAIN);
    }
}));

// ECO aktiv
auto* eco_listener = new SKValueListener<bool>(
    "electrical.motor.eco.enable", 1000);

eco_listener->connect_to(new LambdaConsumer<bool>([](bool eco) {
    if (eco) {
        lv_obj_add_flag(ui_lblECO, LV_OBJ_FLAG_HIDDEN);  
    } else {
        lv_obj_remove_flag(ui_lblECO, LV_OBJ_FLAG_HIDDEN);  
    }
}));

// Motor Fehler
auto* error_listener = new SKValueListener<bool>(
    "propulsion.main.error", 1000);

error_listener->connect_to(new LambdaConsumer<bool>([](bool error) {
    if (error) {
        stop_blinking();
        // Rot
        lv_obj_set_style_bg_color(ui_PlState, lv_color_hex(0xFF0000), LV_PART_MAIN);
    }
}));


// Motortemperatur von SignalK empfangen
auto* temp_motor_listener = new SKValueListener<float>(
    "propulsion.main.temp", 1000);

temp_motor_listener->connect_to(new LambdaConsumer<float>([](float temp_k) {
    // SignalK liefert Temperatur in Kelvin → Celsius
    float temp_c = temp_k - 273.15;
    
    // Bar-Wert setzen (Range in SquareLine anpassen: 0-120°C)
    lv_bar_set_value(ui_BarTempMotor, (int)temp_c, LV_ANIM_ON);
    
    // Farbe je nach Temperatur
    lv_color_t color;
    if (temp_c >= 80) {
        color = lv_color_hex(0xFF0000);  // Rot
    } else if (temp_c >= 60) {
        color = lv_color_hex(0xFF8000);  // Orange
    } else {
        color = lv_color_hex(0x00FF00);  // Grün
    }
    lv_obj_set_style_bg_color(ui_BarTempMotor, color, LV_PART_INDICATOR);
    
}));

// Coolanttemperatur von SignalK empfangen
auto* temp_cool_listener = new SKValueListener<float>(
    "sensors.temperature.Coolant.Temperature.0", 1000);

temp_cool_listener->connect_to(new LambdaConsumer<float>([](float temp_k) {
    // SignalK liefert Temperatur in Kelvin → Celsius
    float temp_c = temp_k - 273.15;
    
    // Bar-Wert setzen (Range in SquareLine anpassen: 0-120°C)
    lv_bar_set_value(ui_BarTempCoolant, (int)temp_c, LV_ANIM_ON);
    
    // Farbe je nach Temperatur
    lv_color_t color;
    if (temp_c >= 80) {
        color = lv_color_hex(0xFF0000);  // Rot
    } else if (temp_c >= 60) {
        color = lv_color_hex(0xFF8000);  // Orange
    } else {
        color = lv_color_hex(0x00FF00);  // Grün
    }
    lv_obj_set_style_bg_color(ui_BarTempCoolant, color, LV_PART_INDICATOR);
    
}));

event_loop()->onRepeat(5, [](){
    lv_timer_handler();
});



  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }