#ifndef HOME_H
#define HOME_H
#include "lvgl/lvgl.h"

lv_obj_t * show_home(void);           //主页面（所有模块都在这一页）
void matrix_event_cb(lv_event_t * e); // 获取里面按键的楼层
#endif

