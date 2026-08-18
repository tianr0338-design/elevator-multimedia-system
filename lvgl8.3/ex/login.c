#include "login.h"
#include "style.h"   // 样式
#include "nav.h"

void login_event_cb(lv_event_t *e)
{
    lv_obj_t * target = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        //show_home();
        nav_switch_to(PAGE_HOME);
    }
}
lv_obj_t * show_login(void)
{
    lv_obj_t * src = lv_obj_create(NULL);
    lv_obj_set_size(src, 800, 480);
    lv_obj_set_style_bg_color(src, lv_color_hex(0xFF0000), 0);

    lv_obj_t * btn = lv_btn_create(src);
    lv_obj_center(btn);
    lv_obj_set_size(btn, 200, 100);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x00FF00), 0);

    lv_obj_t * label = lv_label_create(btn);
    lv_obj_center(label);
    lv_obj_add_style(label, style_get_label(), 0);
    lv_label_set_text(label, "登录");

    lv_obj_add_event_cb(btn, login_event_cb, LV_EVENT_CLICKED, NULL);
    return src;
}

