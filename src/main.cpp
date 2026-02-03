// Signal K application template file.
//
// This application demonstrates core SensESP concepts in a very
// concise manner. You can build and upload the application as is
// and observe the value changes on the serial port monitor.
//
// You can use this source file as a basis for your own projects.
// Remove the parts that are not relevant to you, and add your own code
// for external hardware libraries.

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
#include "displays/battery_ring_display.h"


using namespace sensesp;

BatteryRingDisplay* display = nullptr; 
TFT_eSPI tft = TFT_eSPI();  

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
                    ->set_sk_server("demo.signalk.org", 8)
                    ->get_app();

tft.begin();
// Nach TFT.begin() in setup():
display = new BatteryRingDisplay(&tft);

int listendelay = 1000;

auto* soc_listener = new SKValueListener<float>("electrical.batteries.1.current", listendelay, "electrical/batteries/current");
soc_listener->connect_to(new LambdaConsumer<float>([](float input){
    display->update_soc(input);
}));

auto* bat_listener = new SKValueListener<float>("electrical.batteries.1.voltage", listendelay, "electrical/batteries/voltage");
bat_listener->connect_to(new LambdaConsumer<float>([](float input){
    display->update_volt(input);
}));

event_loop()->onRepeat(1000, [](){
    if (display != nullptr) {
        display->draw();
    }
});



  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
