#ifndef ADVERTISEMENT_H
#define ADVERTISEMENT_H
#include "lvgl/lvgl.h"

// 连接服务器并下载广告，返回 0 成功
int ad_fetch(const char * dev_id, int group);

// 绑定广告UI控件：卡片、标题label、图片
void ad_bind_ui(lv_obj_t *card, lv_obj_t *title_label, lv_obj_t *img);

// 开始轮播（每 3 秒切一条
void ad_start_cycle(void);

void get_time(void);           // 服务器获取日期
void ad_send_alarm(void);                   // 发送警报
void ad_fetch_weather(void);                 // 获取天气（直连外网API）
void ad_bind_weather_label(lv_obj_t *label); // 绑定天气显示标签

#endif
