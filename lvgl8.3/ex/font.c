#include "font.h"


static lv_ft_info_t info; // 先创建字体结构体

void font_init(void)
{
    // 相关的设置
    info.name   = "./simfang.ttf";
    info.style  = FT_FONT_STYLE_NORMAL;
    info.weight = 30;
    info.mem    = NULL;
    lv_ft_font_init(&info);  // 加载 TTF 字体文件
}

lv_font_t * font_get_default(void)
{
    return info.font;
}