#include "app.h"
#include "font.h"
#include "style.h"
#include "home.h"
#include "elevator.h"
#include <stdlib.h>
#include <time.h>
#include "advertisement.h"

void app_init(void)
{
    srand(time(NULL));
    font_init();
    style_init();
    show_home();

    //下载广告，成功就轮播，失败保留"等待广告
    if(ad_fetch("LIFT001", 1) >= 0) {
        ad_start_cycle();
    }

    lv_timer_create((lv_timer_cb_t)elevator_task_tick, 30, NULL);

    /* 获取天气（直连外网，独立于广告服务器） */
    ad_fetch_weather();
}
