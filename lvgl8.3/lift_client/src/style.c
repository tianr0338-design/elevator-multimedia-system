#include "style.h"
#include "font.h"

static lv_style_t style_label;
static lv_style_t style_card;
static lv_style_t style_btn;
static lv_style_t style_floo;
static lv_style_t style_btn_pr;
static lv_style_t style_wait;
void style_init(void)
{
    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label, font_get_default()); // 字体样式

    lv_style_init(&style_floo);
    lv_style_set_text_font(&style_floo, font_get_floo()); // 楼层显示
    lv_style_init(&style_card);                           // 3个卡片的样式
    lv_style_set_bg_opa(&style_card, LV_OPA_70);          // 设置 70% 透明度，透出背景
    lv_style_set_radius(&style_card, 15);                 // 15px 圆角
    lv_style_set_border_width(&style_card, 0);

    // 浅底深字
    lv_style_set_bg_color(&style_btn, lv_color_hex(0xF0F0F0));   // 浅灰底
    lv_style_set_text_color(&style_btn, lv_color_hex(0x333333)); // 深灰字
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_border_width(&style_btn, 1);
    lv_style_set_border_color(&style_btn, lv_color_hex(0xCCCCCC));
    lv_style_set_text_font(&style_btn, font_get_default());
    // 按下的样式
    lv_style_init(&style_btn_pr);
    lv_style_set_bg_color(&style_btn_pr, lv_color_hex(0x2196F3));
    lv_style_set_text_color(&style_btn_pr, lv_color_hex(0xFFFFFF));

    lv_style_init(&style_wait);
    lv_style_set_bg_color(&style_wait, lv_color_hex(0xFF9800));
    lv_style_set_text_color(&style_wait, lv_color_hex(0xFFFFFF));
}

lv_style_t * style_get_label(void)
{
    return &style_label;
}
lv_style_t * style_get_card(void)
{
    return &style_card;
}
lv_style_t * style_get_btn(void)
{
    return &style_btn;
}

lv_style_t * style_get_pr(void)
{
    return &style_btn_pr;
}

lv_style_t * style_get_floo(void)
{
    return &style_floo;
}

lv_style_t * style_get_wait(void)
{
    return &style_wait;
}