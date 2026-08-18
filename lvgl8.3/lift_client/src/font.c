#include "font.h"

static lv_ft_info_t info; // 先创建字体结构体
static lv_ft_info_t info1;
void font_init(void)
{
    // 相关的设置
    info.name   = "./simfang.ttf";
    info.style  = FT_FONT_STYLE_NORMAL;
    info.weight = 25;
    info.mem    = NULL;
    lv_ft_font_init(&info); // 加载 TTF 字体文件

    info1.name   = "./simfang.ttf";
    info1.style  = FT_FONT_STYLE_BOLD;
    info1.weight = 130;
    info1.mem    = NULL;
    lv_ft_font_init(&info1); // 加载 TTF 字体文件
}

lv_font_t * font_get_default(void)
{
    return info.font;
}

lv_font_t * font_get_floo(void)
{
    return info1.font;
}