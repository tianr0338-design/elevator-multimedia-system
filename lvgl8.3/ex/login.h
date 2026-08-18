#ifndef LOGIN_H
#define LOGIN_H
#include "lvgl/lvgl.h"

// void show_login(void);
lv_obj_t * show_login(void);
void login_event_cb(lv_event_t * e); // 触发登录逻辑进入主页面

#endif
