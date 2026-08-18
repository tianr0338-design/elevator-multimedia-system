#include "home.h"
#include "common.h"
#include "media.h"
#include "game.h"
#include "auth.h"
#include <time.h>
#include <stdio.h>

// 状态栏时钟标签指针
static lv_obj_t * g_clock_label = NULL;
// 更新时间的定时器（每秒触发一次）
static lv_timer_t * timer_update_time = NULL;

static void update_time_cb(lv_timer_t * t)
{
    // 更新时间显示为 HH:MM:SS
    if(g_clock_label) {
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        lv_label_set_text_fmt(g_clock_label, "%02d:%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }
}

static void show_home_content(void);

static void update_sidebar_highlight(int active_idx)
{
    // 根据 active_idx 高亮侧边栏按钮，其余恢复为普通态
    for(int i = 0; i < 6; i++) {
        if(g_sidebar_btns[i] == NULL) continue;

        lv_obj_t * btn   = g_sidebar_btns[i];
        lv_obj_t * img   = lv_obj_get_child(btn, 0);
        lv_obj_t * label = lv_obj_get_child(btn, 1);

        if(i == active_idx) {
            lv_obj_set_style_bg_opa(btn, LV_OPA_20, 0);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x00D2FF), 0);
            if(img) lv_obj_set_style_img_recolor_opa(img, LV_OPA_0, 0);
            if(label) lv_obj_set_style_text_color(label, lv_color_white(), 0);
        } else {
            lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
            if(img) lv_obj_set_style_img_recolor_opa(img, LV_OPA_0, 0);
            if(label) lv_obj_set_style_text_color(label, lv_color_hex(0xE0E0E0), 0);
        }
    }
}

static void sidebar_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    intptr_t idx = (intptr_t)lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        // 若点击的是当前页面，直接返回
        if(current_page_idx == (int)idx) return;

        // 离开音乐/视频/游戏/退出等页面前，重置媒体状态，避免残留资源
        if(current_page_idx == 2 || idx == 2 || current_page_idx == 1 || idx == 4 || idx == 5) {
             reset_media_state();
        }

        current_page_idx = (int)idx;
        update_sidebar_highlight((int)idx);

        if(g_content_area) {
            lv_obj_clean(g_content_area);
        }

        if(idx != 0) {
            // 非主页时关闭时钟定时器并清空相关指针
            if(timer_update_time) {
                lv_timer_del(timer_update_time);
                timer_update_time = NULL;
            }
            g_clock_label = NULL;
            g_content_area = NULL; 
        }

        // 页面索引：
        // 0 主页 | 1 音乐 | 2 视频 | 3 相册 | 4 游戏 | 5 退出到登录
        switch(idx) {
            case 0: show_home_content(); break;
            case 1: create_music_screen(); break;
            case 2: create_video_screen(); break;
            case 3: create_album_screen(); break;
            case 4: create_game_screen(); break;
            case 5: create_login_screen(); break;
        }
    }
}

static void show_home_content(void)
{
    // 构建主页内容区域：左侧速度仪表，右侧预留信息面板
    if(!g_content_area) return;

    lv_obj_set_layout(g_content_area, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(g_content_area, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_content_area, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(g_content_area, 10, 0);
    lv_obj_set_style_pad_gap(g_content_area, 20, 0);

    lv_obj_t * speed_box = lv_obj_create(g_content_area);
    lv_obj_set_size(speed_box, 360, 360);
    lv_obj_set_style_bg_opa(speed_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(speed_box, 0, 0);
    lv_obj_clear_flag(speed_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * speed_bg = lv_img_create(speed_box);
    lv_img_set_src(speed_bg, "A:yibiaopan.png");
    lv_obj_align(speed_bg, LV_ALIGN_CENTER, -30, -30);

    // 速度数值与单位
    lv_obj_t * label_speed_val = lv_label_create(speed_box);
    lv_label_set_text(label_speed_val, "65");
    lv_obj_set_style_text_font(label_speed_val, get_chinese_font(), 0);
    lv_obj_set_style_text_color(label_speed_val, lv_color_white(), 0);
    lv_obj_align(label_speed_val, LV_ALIGN_CENTER, -10, -50);
    lv_obj_move_foreground(label_speed_val);

    lv_obj_t * label_speed_unit = lv_label_create(speed_box);
    lv_label_set_text(label_speed_unit, "km/h");
    lv_obj_set_style_text_font(label_speed_unit, get_chinese_font(), 0);
    lv_obj_set_style_text_color(label_speed_unit, lv_color_white(), 0);
    lv_obj_align_to(label_speed_unit, label_speed_val, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_move_foreground(label_speed_unit);

    lv_obj_t * right_panel = lv_obj_create(g_content_area);
    lv_obj_set_size(right_panel, 300, 380);
    lv_obj_set_style_bg_opa(right_panel, LV_OPA_0, 0);
    lv_obj_set_style_border_width(right_panel, 0, 0);
    lv_obj_clear_flag(right_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * car_img = lv_img_create(right_panel);
    lv_img_set_src(car_img, "A:bakright.png");
    lv_obj_center(car_img);
}

void create_main_dashboard(void)
{
    // 构建主页主屏幕：背景、状态栏、侧边栏、内容区域
    current_page_idx = 0;

    lv_obj_t * main_scr = lv_obj_create(NULL);
    lv_obj_clear_flag(main_scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * bg_img = lv_img_create(main_scr);
    lv_img_set_src(bg_img, "A:zhuyebak.png");
    lv_obj_center(bg_img);

    // 顶部状态栏（时间/天气/图标）
    lv_obj_t * status_bar = lv_obj_create(main_scr);
    lv_obj_set_size(status_bar, 800, 80);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_opa(status_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * left_box = lv_obj_create(status_bar);
    lv_obj_set_size(left_box, 300, 80);
    lv_obj_align(left_box, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_opa(left_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left_box, 0, 0);
    lv_obj_set_layout(left_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(left_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(left_box, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(left_box, 10, 0);
    lv_obj_set_style_pad_column(left_box, 15, 0);

    g_clock_label = lv_label_create(left_box);
    lv_label_set_text(g_clock_label, "00:00:00");
    lv_obj_set_style_text_color(g_clock_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(g_clock_label, get_chinese_font(), 0);

    lv_obj_t * weather_icon = lv_label_create(left_box);
    lv_label_set_text(weather_icon, "☀");
    lv_obj_set_style_text_color(weather_icon, lv_color_hex(0xFFD700), 0);
    lv_obj_set_style_text_font(weather_icon, get_chinese_font(), 0);

    lv_obj_t * temp_label = lv_label_create(left_box);
    lv_label_set_text(temp_label, "26°C");
    lv_obj_set_style_text_color(temp_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(temp_label, get_chinese_font(), 0);

    lv_obj_t * center_box = lv_obj_create(status_bar);
    lv_obj_set_size(center_box, 200, 80);
    lv_obj_align(center_box, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(center_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(center_box, 0, 0);
    lv_obj_set_layout(center_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(center_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(center_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(center_box, 20, 0);

    lv_obj_t * usb_icon = lv_label_create(center_box);
    lv_label_set_text(usb_icon, LV_SYMBOL_USB);
    lv_obj_set_style_text_color(usb_icon, lv_color_hex(0xAAAAAA), 0);

    lv_obj_t * msg_icon = lv_label_create(center_box);
    lv_label_set_text(msg_icon, LV_SYMBOL_NEW_LINE);
    lv_obj_set_style_text_color(msg_icon, lv_color_hex(0xAAAAAA), 0);

    // 右侧无线/音量状态
    lv_obj_t * right_box = lv_obj_create(status_bar);
    lv_obj_set_size(right_box, 300, 80);
    lv_obj_align(right_box, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_opa(right_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_box, 0, 0);
    lv_obj_set_layout(right_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right_box, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_right(right_box, 20, 0);
    lv_obj_set_style_pad_column(right_box, 15, 0);

    lv_obj_t * bt_icon = lv_label_create(right_box);
    lv_label_set_text(bt_icon, LV_SYMBOL_BLUETOOTH);
    lv_obj_set_style_text_color(bt_icon, lv_color_hex(0x00D2FF), 0);

    lv_obj_t * wifi_icon = lv_label_create(right_box);
    lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_icon, lv_color_white(), 0);

    lv_obj_t * vol_icon = lv_label_create(right_box);
    lv_label_set_text(vol_icon, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_color(vol_icon, lv_color_white(), 0);

    // 主页时钟定时器（1s刷新）
    if(timer_update_time) lv_timer_del(timer_update_time);
    timer_update_time = lv_timer_create(update_time_cb, 1000, NULL);
    update_time_cb(NULL);

    // 左侧侧边栏（6 个入口）
    lv_obj_t * side_bar = lv_obj_create(main_scr);
    lv_obj_set_size(side_bar, 100, 390);
    lv_obj_align(side_bar, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_set_style_bg_opa(side_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(side_bar, 0, 0);
    lv_obj_set_style_radius(side_bar, 0, 0);
    lv_obj_set_style_pad_all(side_bar, 0, 0);
    lv_obj_set_style_pad_row(side_bar, 20, 0);

    lv_obj_set_layout(side_bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(side_bar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(side_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    g_sidebar_btns[0] = create_sidebar_btn(side_bar, "A:zhuye.png", "主页");
    lv_obj_add_event_cb(g_sidebar_btns[0], sidebar_event_cb, LV_EVENT_CLICKED, (void *)0);

    g_sidebar_btns[1] = create_sidebar_btn(side_bar, "A:mus.png", "音乐");
    lv_obj_add_event_cb(g_sidebar_btns[1], sidebar_event_cb, LV_EVENT_CLICKED, (void *)1);

    g_sidebar_btns[2] = create_sidebar_btn(side_bar, "A:shiping.png", "视频");
    lv_obj_add_event_cb(g_sidebar_btns[2], sidebar_event_cb, LV_EVENT_CLICKED, (void *)2);

    g_sidebar_btns[3] = create_sidebar_btn(side_bar, "A:xiangche.png", "相册");
    lv_obj_add_event_cb(g_sidebar_btns[3], sidebar_event_cb, LV_EVENT_CLICKED, (void *)3);

    g_sidebar_btns[4] = create_sidebar_btn(side_bar, "A:game.png", "游戏");
    lv_obj_add_event_cb(g_sidebar_btns[4], sidebar_event_cb, LV_EVENT_CLICKED, (void *)4);

    g_sidebar_btns[5] = create_sidebar_btn(side_bar, "A:exit.png", "退出");
    lv_obj_add_event_cb(g_sidebar_btns[5], sidebar_event_cb, LV_EVENT_CLICKED, (void *)5);

    // 右侧内容区域（主页布局）
    g_content_area = lv_obj_create(main_scr);
    lv_obj_set_size(g_content_area, 700, 390);
    lv_obj_align(g_content_area, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_style_bg_opa(g_content_area, LV_OPA_0, 0);
    lv_obj_set_style_border_width(g_content_area, 0, 0);
    lv_obj_clear_flag(g_content_area, LV_OBJ_FLAG_SCROLLABLE);

    show_home_content();
    update_sidebar_highlight(0);

    // 加载主屏幕（销毁旧屏幕）
    lv_scr_load_anim(main_scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}
