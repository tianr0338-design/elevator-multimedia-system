#include "app.h"
#include "font.h"
#include "style.h"
#include "nav.h"

void app_init(void)
{
    font_init();  // 第一步：加载底层字体资源
    style_init(); // 第二步：基于字体创建样式
    nav_switch_to(PAGE_LOGIN);
}
