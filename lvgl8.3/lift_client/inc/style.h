#ifndef STYLE_H
#define STYLE_H
#include "lvgl/lvgl.h"

void style_init(void);              // 统一获取所有的样式
lv_style_t * style_get_label(void); // 获取标签样式
lv_style_t * style_get_card(void);  // 获取卡片样式
lv_style_t * style_get_btn(void);   // 获取按钮样式
lv_style_t * style_get_pr(void);    // 按下的的瞬间变化样式
lv_style_t * style_get_wait(void);    // 等待的过程中
lv_style_t * style_get_floo(void);  // 楼层显示
#endif
