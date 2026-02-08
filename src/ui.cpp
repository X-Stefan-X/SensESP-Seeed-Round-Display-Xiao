#include "ui.h"
#include <TFT_eSPI.h>
#include <lvgl.h>

#define USE_TFT_ESPI_LIBRARY
#include "lv_xiao_round_screen.h"

extern "C" {
  #include "../ui/ui.h"
}

void setup_ui() {
  lv_init();
  lv_tick_set_cb(millis);
  lv_xiao_disp_init();
  lv_xiao_touch_init();
  ui_init();
}