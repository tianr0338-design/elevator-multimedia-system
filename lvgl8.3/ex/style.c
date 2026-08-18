#include "style.h"
#include "font.h"

static lv_style_t style_label; // static，外面看不见
void style_init(void)
{
    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label, font_get_default());
}

lv_style_t * style_get_label(void)
{
    return &style_label;
}