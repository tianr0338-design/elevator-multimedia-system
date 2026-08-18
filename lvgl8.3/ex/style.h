#ifndef STYLE_H
#define STYLE_H
#include "lvgl/lvgl.h"

void style_init(void); // 统一获取所有的样式
lv_style_t * style_get_label(void); // 获取标签样式
#endif