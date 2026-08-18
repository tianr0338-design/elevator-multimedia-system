#include "home.h"
#include "nav.h"
#include "style.h"
void home_back_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        nav_switch_to(PAGE_LOGIN);
    }
}

lv_obj_t * show_home(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, 800, 480);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t * label = lv_label_create(scr);
    lv_obj_add_style(label, style_get_label(), 0);
    lv_obj_center(label);
    lv_label_set_text(label, "主页面");

    // 返回按钮
    lv_obj_t * btn       = lv_btn_create(scr);
    lv_obj_t * btn_label = lv_label_create(btn);
    lv_obj_add_style(btn_label, style_get_label(), 0);
    lv_label_set_text(btn_label, "返回");
    lv_obj_add_event_cb(btn, home_back_cb, LV_EVENT_CLICKED, NULL);

    return scr;
}