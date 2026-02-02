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

#include "I2C_BM8563.h"
#include <lvgl.h>

using namespace sensesp;


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
                    //->set_sk_server("192.168.10.3", 80)
                    ->get_app();



    // SignalK Battery & Voltage Producer
float battery_soc = 0.0;
float system_voltage = 0.0;

auto battery_producer = new SKPathProducer<float>(
    "electrical.batteries.1.capacity.stateOfCharge",
    &battery_soc, "Battery SoC %",
    new Linear(0, 100));

auto voltage_producer = new SKPathProducer<float>(
    "electrical.batteries.1.voltage",
    &system_voltage, "Battery Voltage V",
    new Linear(0, 15));

// Battery Ring Display hinzufügen
auto battery_display = new BatteryRingDisplay(
    0, 0, 240, 240,  // Vollbild
    &battery_soc, &system_voltage);

ui.addElement(battery_display);

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
