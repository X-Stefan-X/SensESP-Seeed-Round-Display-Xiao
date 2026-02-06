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

//BatteryRingDisplay* display = nullptr; 
TFT_eSPI stats_display = TFT_eSPI();

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
auto* bat_listener = new SKValueListener<float>("electrical.batteries.1.voltage", listendelay, "electrical/batteries/voltage");
bat_listener->connect_to(new LambdaConsumer<float>([](float input){
    int arc_value = (int)(input / 14.4*4095);
    lv_arc_set_value(ui_ARCbatLevel, arc_value); 

    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f V", input);
    lv_label_set_text(ui_LBLunten, buf);
}));


//ARC oben Throttle
auto* throttle_out = new SKOutput<float>("propulsion.port.throttle", "propulsion/port/throttle");

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