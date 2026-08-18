#ifndef COMMON_H
#define COMMON_H

#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// 全局变量声明
extern lv_obj_t * g_content_area;//内容区域
extern lv_obj_t * g_sidebar_btns[6];//侧边栏按钮
extern int current_page_idx;//当前页面索引

// 公共辅助函数
lv_font_t * get_chinese_font(void);//获取中文字体
lv_obj_t * create_sidebar_btn(lv_obj_t * parent, const char * img_path, const char * text);//创建侧边栏按钮
void show_toast(const char * text, lv_color_t color);//显示提示信息
void common_kb_event_cb(lv_event_t * e);//通用键盘事件回调

#endif
