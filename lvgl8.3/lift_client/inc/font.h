#ifndef FONT_H
#define FONT_H

#include "lvgl/lvgl.h"

void font_init(void);               // 中文字体的样式设置
lv_font_t * font_get_default(void); // 获取默认字体指针
lv_font_t * font_get_floo(void); // 楼层显示
#endif