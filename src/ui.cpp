#include "ui.h"  // SquareLine UI
#include "sensesp.h"

extern tft stats_display;  // Dein bestehender Display aus main.cpp

void setup_ui() {
    lv_init();
    stats_display.lvgl_driver_init();  // Nutze DEINEN bestehenden Display-Treiber
    
    static lv_color_t buf[stats_display.height() * 10];  // LVGL Buffer
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, stats_display.height() * 10);
    
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = stats_display.lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.hor_res = stats_display.width();
    disp_drv.ver_res = stats_display.height();
    stats_display.lvgl_register_disp_driver(&disp_drv);
    
    ui_init();  // SquareLine Screens initialisieren
    
    // UI-Update Timer (500ms)
    static lv_timer_t * ui_timer = lv_timer_create([](lv_timer_t *){
        // Hier später deine SensESP-Werte:
        // lv_arc_set_value(ui_arc_bottom, battery_soc);
        // lv_label_set_text_fmt(ui_label_percent, "%d%%", soc);
    }, 500, NULL);
}
