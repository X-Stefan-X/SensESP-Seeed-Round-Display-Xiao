#include "ui.h"      
#include <lvgl.h>
#include <TFT_eSPI.h>

extern TFT_eSPI stats_display;

void my_disp_flush(lv_disp_t *disp, const lv_area_t *area, uint8_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    stats_display.startWrite();
    stats_display.setAddrWindow(area->x1, area->y1, w, h);
    stats_display.pushColors((uint16_t *)color_p, w * h, true);
    stats_display.endWrite();
    
    lv_disp_flush_ready(disp);
}

void setup_ui() {
    lv_init();
    
    stats_display.init();
    stats_display.setRotation(0);
    stats_display.fillScreen(TFT_BLACK);
    
    // LVGL v9 Buffer
    static lv_color_t buf[240 * 10];
    static lv_draw_buf_t draw_buf;
    lv_draw_buf_init(&draw_buf, 240, 10, LV_COLOR_FORMAT_RGB565, &buf);
    
    // LVGL v9 Display (KORREKT)
    lv_disp_t *disp = lv_disp_create(240, 240);
    lv_disp_set_flush_cb(disp, my_disp_flush);
    lv_disp_set_draw_buffers(disp, &draw_buf, NULL);
    
    ui_init();
    
    // Test: Roter Kreis in der Mitte
    lv_obj_t *test_circle = lv_arc_create(lv_screen_active());
    lv_arc_set_range(test_circle, 0, 100);
    lv_arc_set_value(test_circle, 75);
    lv_obj_set_size(test_circle, 100, 100);
    lv_obj_center(test_circle);
}
