#include "setting.h"
#include "style.h"
#include "nav.h"

lv_obj_t * show_setting(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, 800, 480);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xEEEEEE), 0);

    lv_obj_t * label = lv_label_create(scr);
    lv_obj_add_style(label, style_get_label(), 0);
    lv_label_set_text(label, "设置页面");

    return scr;
}
