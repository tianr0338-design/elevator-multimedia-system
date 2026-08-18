#ifndef EX01_H
#define EX01_H
#include "lvgl/lvgl.h"
#include <stdio.h>
void ex(void);                        // 练习
void btn_evnt_cb(lv_event_t * e);     // 回调函数
void ui_style_btn_primary_init(void); // 按钮的样式函数
void set_font(void);                  // 中文字体设置
void test_keyboard(void);             // 键盘和文本框
void ta_event_cd(lv_event_t * e);     // 绑定文本框和键盘
#endif                                // !EX01_H