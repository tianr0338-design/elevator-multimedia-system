#ifndef ARC_H
#define ARC_H
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string.h>
void lv_ex_arc(void);                                // hu
void obj(void);                                      // 容器
void fix(void);                                      // 弹性布局
void lv_example_slider(void);                        // 滑块
void lv_example_switch(void);                        // 开关
void lv_example_textarea_simple(void);               // 键盘和文本框
void event_btn_cd(lv_event_t * e);                   // 事件
static void textarea_click_event_cb(lv_event_t * e); // 点击显示键盘
void show_jpg(void);                                 // 图片的显示
void show_chinese(void);                             // 显示中文
void login(void);
static void login_btn_event_cb(lv_event_t * e);
void test_pinyin(void);// 拼音键盘
void ta_event_cb1(lv_event_t * e);
#endif