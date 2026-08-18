#include "media.h"
#include "common.h"
#include "home.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define MUSIC_FIFO "movie_fifo" // 定义音乐控制管道路径

// ---------------- 音乐模块全局变量 ----------------
static lv_obj_t * music_album_img = NULL;       // 黑胶唱片图片
static lv_anim_t music_anim;                    // 旋转动画对象
static bool is_music_playing           = false; // 播放状态
static lv_obj_t * music_play_btn_label = NULL;  // 播放按钮图标
static int music_fd                    = -1;    // 音乐管道文件描述符
static lv_obj_t * music_title_label    = NULL;  // 歌名标签
static lv_obj_t * music_artist_label   = NULL;  // 歌手标签
static lv_obj_t * spectrum_bars[12];            // 音乐频谱条数组
static lv_timer_t * spectrum_timer = NULL;      // 频谱定时器

// 音乐列表配置
#define MUSIC_TOTAL_COUNT 2 // 播放列表总数
// 音乐文件名数组
static const char * music_files[MUSIC_TOTAL_COUNT] = {"1.mp3", "2.mp3"};
// 音乐标题数组
static const char * music_titles[MUSIC_TOTAL_COUNT] = {"此生不换", "Visions"};
// 歌手名称数组
static const char * music_artists[MUSIC_TOTAL_COUNT] = {"青鸟飞鱼", "Unknown Artist"};
static int music_current_idx                         = 0; // 当前播放索引 (0 ~ MUSIC_TOTAL_COUNT-1)

// ---------------- 视频模块全局变量 ----------------
static lv_obj_t * video_play_btn_label = NULL;  // 视频播放按钮图标
static lv_obj_t * video_disp_area      = NULL;  // 视频显示区域对象
static bool is_video_playing           = false; // 视频播放状态

// 视频列表配置
#define VIDEO_TOTAL_COUNT 3
// 视频文件播放列表 (循环切换，初始索引 0)
static const char * video_files[VIDEO_TOTAL_COUNT] = {"1.avi", "2.avi", "3.avi"};
static int video_current_idx                       = 0;

// ---------------- 相册模块全局变量 ----------------
// 相册资源与状态
static const char * album_imgs[] = {
    "A:1.png", // 索引 0
    "A:2.png", // 索引 1
    "A:3.png", // 索引 2
    "A:4.png"  // 索引 3
};
#define ALBUM_TOTAL_IMGS (sizeof(album_imgs) / sizeof(album_imgs[0]))

static int album_current_idx       = 0;    // 当前显示的图片索引
static uint16_t album_zoom_val     = 256;  // 当前缩放倍数
static lv_obj_t * album_img_obj    = NULL; // 图片对象指针
static lv_obj_t * album_zoom_label = NULL; // 缩放比例显示标签指针
static int album_pending_idx   = 0;
static bool is_album_switching = false;

// ---------------- 内部函数声明 ----------------
static void send_mplayer_cmd(const char * cmd);
static void init_mplayer(void);
static void video_redraw_timer_cb(lv_timer_t * timer);
static void update_album_view(void);
static void anim_set_img_opa(void * obj, int32_t v);
static void album_fade_ready_cb(lv_anim_t * a);
static void Draw_Full_Background(void);

// ---------------- 实现代码 ----------------

static void Draw_Full_Background(void)
{
    // 1. 强制当前活动屏幕重绘
    lv_obj_invalidate(lv_scr_act());

    // 2. 显式重绘视频区域 (确保黑色背景覆盖)
    if(video_disp_area) {
        lv_obj_invalidate(video_disp_area);
    }
}

// 发送 mplayer 控制命令
static void send_mplayer_cmd(const char * cmd)
{
    if(music_fd < 0) {
        music_fd = open(MUSIC_FIFO, O_WRONLY | O_NONBLOCK);
    }
    if(music_fd > 0) {
        ssize_t n = write(music_fd, cmd, strlen(cmd));
        if(n <= 0 && errno == EPIPE) {
            close(music_fd);
            music_fd = -1;
        } else {
            printf("MPlayer CMD: %s\n", cmd);
        }
    } else {
        perror("Open FIFO failed");
    }
}

// 连续强制刷新回调
static void video_redraw_timer_cb(lv_timer_t * timer)
{
    lv_obj_invalidate(lv_scr_act());
    if(video_disp_area) {
        lv_obj_invalidate(video_disp_area);
    }
}

// 停止 mplayer 播放 (Public function implementation)
void stop_mplayer(void)
{
    system("killall mplayer > /dev/null 2>&1");
    if(music_fd > 0) {
        close(music_fd);
        music_fd = -1;
    }
    is_video_playing = false;
    Draw_Full_Background();
    lv_timer_t * timer = lv_timer_create(video_redraw_timer_cb, 50, NULL);
    lv_timer_set_repeat_count(timer, 10);
}

// 初始化并启动 mplayer
static void init_mplayer(void)
{
    if(access(MUSIC_FIFO, F_OK) == -1) {
        if(mkfifo(MUSIC_FIFO, 0777) != 0) {
            perror("mkfifo failed");
            return;
        }
    }
    char cmd[256];
    system("killall mplayer");
    snprintf(cmd, sizeof(cmd), "mplayer -idle -slave -input file=%s %s &", MUSIC_FIFO, music_files[music_current_idx]);
    system(cmd);
    is_music_playing = true;
}

// 音乐专辑封面旋转动画回调
static void music_anim_cb(void * var, int32_t v)
{
    lv_img_set_angle(var, (int16_t)v);
}

// 音乐控制按钮事件回调
static void music_btn_event_cb(lv_event_t * e)
{
    intptr_t code = (intptr_t)lv_event_get_user_data(e);

    if(code == 0 || code == 2) { // Prev/Next
        if(code == 0) {
            music_current_idx--;
            if(music_current_idx < 0) music_current_idx = MUSIC_TOTAL_COUNT - 1;
        } else {
            music_current_idx++;
            if(music_current_idx >= MUSIC_TOTAL_COUNT) music_current_idx = 0;
        }

        char cmd[256];
        snprintf(cmd, sizeof(cmd), "loadfile %s\n", music_files[music_current_idx]);
        send_mplayer_cmd(cmd);

        if(music_title_label) lv_label_set_text(music_title_label, music_titles[music_current_idx]);
        if(music_artist_label) lv_label_set_text(music_artist_label, music_artists[music_current_idx]);

        if(!is_music_playing) {
            is_music_playing = true;
            if(music_play_btn_label) lv_label_set_text(music_play_btn_label, LV_SYMBOL_PAUSE);
            
            lv_anim_init(&music_anim);
            lv_anim_set_var(&music_anim, music_album_img);
            lv_anim_set_exec_cb(&music_anim, music_anim_cb);
            lv_anim_set_time(&music_anim, 10000);
            lv_anim_set_values(&music_anim, 0, 3600);
            lv_anim_set_repeat_count(&music_anim, LV_ANIM_REPEAT_INFINITE);
            lv_anim_start(&music_anim);
        }
    } else if(code == 1) { // Play/Pause
        is_music_playing = !is_music_playing;
        if(is_music_playing) {
            send_mplayer_cmd("pause\n");
            if(music_play_btn_label) lv_label_set_text(music_play_btn_label, LV_SYMBOL_PAUSE);
            
            lv_anim_init(&music_anim);
            lv_anim_set_var(&music_anim, music_album_img);
            lv_anim_set_exec_cb(&music_anim, music_anim_cb);
            lv_anim_set_time(&music_anim, 10000);
            lv_anim_set_values(&music_anim, 0, 3600);
            lv_anim_set_repeat_count(&music_anim, LV_ANIM_REPEAT_INFINITE);
            lv_anim_start(&music_anim);
        } else {
            send_mplayer_cmd("pause\n");
            if(music_play_btn_label) lv_label_set_text(music_play_btn_label, LV_SYMBOL_PLAY);
            lv_anim_del(music_album_img, music_anim_cb);
        }
    }
}

// 频谱动画回调
static void spectrum_update_cb(lv_timer_t * t)
{
    if(!is_music_playing) {
        for(int i = 0; i < 12; i++) {
            if(lv_bar_get_value(spectrum_bars[i]) > 0) {
                lv_bar_set_value(spectrum_bars[i], 0, LV_ANIM_ON);
            }
        }
        return;
    }
    for(int i = 0; i < 12; i++) {
        int val = rand() % 90 + 10;
        lv_bar_set_value(spectrum_bars[i], val, LV_ANIM_ON);
    }
}

// 音乐屏幕销毁清理回调
static void music_scr_del_cb(lv_event_t * e)
{
    if(spectrum_timer) {
        lv_timer_del(spectrum_timer);
        spectrum_timer = NULL;
    }
}

// 音量控制回调
static void music_volume_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int val           = (int)lv_slider_get_value(slider);
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "volume %d 1\n", val);
    send_mplayer_cmd(cmd);
}

// 返回主页事件回调
static void media_back_event_cb(lv_event_t * e)
{
    // 清理全屏界面的全局指针
    music_album_img      = NULL;
    music_play_btn_label = NULL;
    music_title_label    = NULL;
    music_artist_label   = NULL;
    album_img_obj        = NULL;
    album_zoom_label     = NULL;
    video_disp_area      = NULL;
    video_play_btn_label = NULL;

    // 如果正在播放视频，停止
    if(is_video_playing) {
        stop_mplayer();
        is_video_playing = false;
    }

    create_main_dashboard();
}

// 创建音乐播放界面 (Public function)
void create_music_screen(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(scr, music_scr_del_cb, LV_EVENT_DELETE, NULL);

    lv_obj_t * bg_img = lv_img_create(scr);
    lv_img_set_src(bg_img, "A:zhuyebak.png");
    lv_obj_center(bg_img);

    lv_obj_t * main_cont = lv_obj_create(scr);
    lv_obj_set_size(main_cont, 760, 420);
    lv_obj_align(main_cont, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);

    lv_obj_set_layout(main_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(main_cont, 0, 0);
    lv_obj_set_style_pad_column(main_cont, 0, 0);

    // Left Container
    lv_obj_t * left_cont = lv_obj_create(main_cont);
    lv_obj_set_height(left_cont, LV_PCT(100));
    lv_obj_set_flex_grow(left_cont, 45);
    lv_obj_set_style_bg_opa(left_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(left_cont, 0, 0);
    lv_obj_clear_flag(left_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * glow = lv_obj_create(left_cont);
    lv_obj_set_size(glow, 260, 260);
    lv_obj_align(glow, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_radius(glow, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(glow, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(glow, LV_OPA_20, 0);
    lv_obj_set_style_shadow_width(glow, 50, 0);
    lv_obj_set_style_shadow_color(glow, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_shadow_spread(glow, 10, 0);

    lv_obj_t * album_wrap = lv_obj_create(left_cont);
    lv_obj_set_size(album_wrap, 240, 240);
    lv_obj_align(album_wrap, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_radius(album_wrap, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_clip_corner(album_wrap, true, 0);
    lv_obj_set_style_bg_opa(album_wrap, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(album_wrap, 0, 0);

    music_album_img = lv_img_create(album_wrap);
    lv_img_set_src(music_album_img, "A:rmus.png");
    lv_obj_center(music_album_img);

    lv_obj_t * center_dot = lv_obj_create(album_wrap);
    lv_obj_set_size(center_dot, 60, 60);
    lv_obj_center(center_dot);
    lv_obj_set_style_radius(center_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(center_dot, lv_color_hex(0x202020), 0);
    lv_obj_set_style_border_width(center_dot, 2, 0);
    lv_obj_set_style_border_color(center_dot, lv_color_hex(0x505050), 0);

    lv_obj_t * center_highlight = lv_obj_create(center_dot);
    lv_obj_set_size(center_highlight, 10, 10);
    lv_obj_center(center_highlight);
    lv_obj_set_style_radius(center_highlight, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(center_highlight, lv_color_hex(0xAAAAAA), 0);

    lv_obj_t * spectrum_cont = lv_obj_create(left_cont);
    lv_obj_set_size(spectrum_cont, 240, 60);
    lv_obj_align(spectrum_cont, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_opa(spectrum_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(spectrum_cont, 0, 0);
    lv_obj_set_layout(spectrum_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(spectrum_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(spectrum_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(spectrum_cont, 0, 0);
    lv_obj_set_style_pad_column(spectrum_cont, 2, 0);

    for(int i = 0; i < 12; i++) {
        spectrum_bars[i] = lv_bar_create(spectrum_cont);
        lv_obj_set_size(spectrum_bars[i], 12, 50);
        lv_bar_set_range(spectrum_bars[i], 0, 100);
        lv_bar_set_value(spectrum_bars[i], 10, LV_ANIM_OFF);
        lv_obj_set_style_bg_color(spectrum_bars[i], lv_color_hex(0x00D2FF), LV_PART_INDICATOR);
        lv_obj_set_style_bg_grad_color(spectrum_bars[i], lv_color_hex(0x0050FF), LV_PART_INDICATOR);
        lv_obj_set_style_bg_grad_dir(spectrum_bars[i], LV_GRAD_DIR_VER, LV_PART_INDICATOR);
        lv_obj_set_style_radius(spectrum_bars[i], 2, LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa(spectrum_bars[i], LV_OPA_TRANSP, LV_PART_MAIN);
    }

    spectrum_timer = lv_timer_create(spectrum_update_cb, 100, NULL);

    init_mplayer();

    // Right Container
    lv_obj_t * right_cont = lv_obj_create(main_cont);
    lv_obj_set_height(right_cont, LV_PCT(100));
    lv_obj_set_flex_grow(right_cont, 55);
    lv_obj_set_style_bg_opa(right_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(right_cont, 0, 0);
    lv_obj_clear_flag(right_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_layout(right_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(right_cont, 20, 0);

    lv_obj_t * info_box = lv_obj_create(right_cont);
    lv_obj_set_size(info_box, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(info_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(info_box, 0, 0);
    lv_obj_set_flex_flow(info_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    music_title_label = lv_label_create(info_box);
    lv_label_set_text(music_title_label, music_titles[music_current_idx]);
    lv_obj_set_style_text_font(music_title_label, get_chinese_font(), 0);
    lv_obj_set_style_text_color(music_title_label, lv_color_white(), 0);

    music_artist_label = lv_label_create(info_box);
    lv_label_set_text(music_artist_label, music_artists[music_current_idx]);
    lv_obj_set_style_text_font(music_artist_label, get_chinese_font(), 0);
    lv_obj_set_style_text_color(music_artist_label, lv_color_hex(0xAAAAAA), 0);

    lv_obj_t * progress_box = lv_obj_create(right_cont);
    lv_obj_set_size(progress_box, LV_PCT(90), 40);
    lv_obj_set_style_bg_opa(progress_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(progress_box, 0, 0);
    lv_obj_set_layout(progress_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(progress_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(progress_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * time_curr = lv_label_create(progress_box);
    lv_label_set_text(time_curr, "00:00");
    lv_obj_set_style_text_color(time_curr, lv_color_white(), 0);

    lv_obj_t * slider = lv_slider_create(progress_box);
    lv_obj_set_width(slider, 200);
    lv_slider_set_value(slider, 30, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x00D2FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_white(), LV_PART_KNOB);

    lv_obj_t * time_total = lv_label_create(progress_box);
    lv_label_set_text(time_total, "03:45");
    lv_obj_set_style_text_color(time_total, lv_color_white(), 0);

    lv_obj_t * ctrl_box = lv_obj_create(right_cont);
    lv_obj_set_size(ctrl_box, LV_PCT(100), 80);
    lv_obj_set_style_bg_opa(ctrl_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_box, 0, 0);
    lv_obj_set_layout(ctrl_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(ctrl_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ctrl_box, 30, 0);

    lv_obj_t * btn_prev = lv_btn_create(ctrl_box);
    lv_obj_set_size(btn_prev, 60, 60);
    lv_obj_set_style_radius(btn_prev, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_prev, lv_color_hex(0x202020), 0);
    lv_obj_set_style_bg_opa(btn_prev, LV_OPA_60, 0);
    lv_obj_set_style_border_width(btn_prev, 1, 0);
    lv_obj_set_style_border_color(btn_prev, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_border_opa(btn_prev, LV_OPA_40, 0);
    lv_obj_t * lbl_prev = lv_label_create(btn_prev);
    lv_label_set_text(lbl_prev, LV_SYMBOL_PREV);
    lv_obj_set_style_text_color(lbl_prev, lv_color_hex(0x00D2FF), 0);
    lv_obj_center(lbl_prev);
    lv_obj_add_event_cb(btn_prev, music_btn_event_cb, LV_EVENT_CLICKED, (void *)0);

    lv_obj_t * btn_play = lv_btn_create(ctrl_box);
    lv_obj_set_size(btn_play, 80, 80);
    lv_obj_set_style_radius(btn_play, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_play, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_bg_grad_color(btn_play, lv_color_hex(0x0050FF), 0);
    lv_obj_set_style_bg_grad_dir(btn_play, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_shadow_width(btn_play, 30, 0);
    lv_obj_set_style_shadow_color(btn_play, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_shadow_spread(btn_play, 5, 0);
    lv_obj_set_style_border_width(btn_play, 2, 0);
    lv_obj_set_style_border_color(btn_play, lv_color_white(), 0);
    lv_obj_set_style_border_opa(btn_play, LV_OPA_30, 0);

    music_play_btn_label = lv_label_create(btn_play);
    lv_label_set_text(music_play_btn_label, LV_SYMBOL_PAUSE);
    lv_obj_set_style_text_font(music_play_btn_label, lv_font_default(), 0);
    lv_obj_set_style_text_color(music_play_btn_label, lv_color_white(), 0);
    lv_obj_center(music_play_btn_label);
    lv_obj_add_event_cb(btn_play, music_btn_event_cb, LV_EVENT_CLICKED, (void *)1);

    lv_anim_init(&music_anim);
    lv_anim_set_var(&music_anim, music_album_img);
    lv_anim_set_exec_cb(&music_anim, music_anim_cb);
    lv_anim_set_time(&music_anim, 10000);
    lv_anim_set_values(&music_anim, 0, 3600);
    lv_anim_set_repeat_count(&music_anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&music_anim);

    lv_obj_t * btn_next = lv_btn_create(ctrl_box);
    lv_obj_set_size(btn_next, 60, 60);
    lv_obj_set_style_radius(btn_next, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_next, lv_color_hex(0x202020), 0);
    lv_obj_set_style_bg_opa(btn_next, LV_OPA_60, 0);
    lv_obj_set_style_border_width(btn_next, 1, 0);
    lv_obj_set_style_border_color(btn_next, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_border_opa(btn_next, LV_OPA_40, 0);
    lv_obj_t * lbl_next = lv_label_create(btn_next);
    lv_label_set_text(lbl_next, LV_SYMBOL_NEXT);
    lv_obj_set_style_text_color(lbl_next, lv_color_hex(0x00D2FF), 0);
    lv_obj_center(lbl_next);
    lv_obj_add_event_cb(btn_next, music_btn_event_cb, LV_EVENT_CLICKED, (void *)2);

    lv_obj_t * vol_box = lv_obj_create(right_cont);
    lv_obj_set_size(vol_box, LV_PCT(90), 50);
    lv_obj_set_style_bg_opa(vol_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vol_box, 0, 0);
    lv_obj_set_layout(vol_box, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(vol_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(vol_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(vol_box, 15, 0);

    lv_obj_t * vol_icon = lv_label_create(vol_box);
    lv_label_set_text(vol_icon, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_color(vol_icon, lv_color_hex(0xAAAAAA), 0);

    lv_obj_t * vol_slider = lv_slider_create(vol_box);
    lv_obj_set_width(vol_slider, 200);
    lv_slider_set_range(vol_slider, 0, 100);
    lv_slider_set_value(vol_slider, 80, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(vol_slider, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(vol_slider, lv_color_hex(0x00D2FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(vol_slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_add_event_cb(vol_slider, music_volume_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * btn_mode = lv_btn_create(vol_box);
    lv_obj_set_size(btn_mode, 40, 40);
    lv_obj_set_style_radius(btn_mode, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_mode, lv_color_hex(0x303030), 0);
    lv_obj_t * lbl_mode = lv_label_create(btn_mode);
    lv_label_set_text(lbl_mode, LV_SYMBOL_LOOP);
    lv_obj_center(lbl_mode);

    lv_obj_t * btn_back = lv_btn_create(scr);
    lv_obj_set_size(btn_back, 40, 40);
    lv_obj_set_style_radius(btn_back, 20, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x808080), 0);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_80, 0);
    lv_obj_set_pos(btn_back, 20, 20);

    lv_obj_t * label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, LV_SYMBOL_HOME);
    lv_obj_center(label_back);

    lv_obj_add_event_cb(btn_back, media_back_event_cb, LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

void reset_media_state(void)
{
    stop_mplayer();
    is_music_playing = false;
    is_video_playing = false;
    
    // Stop music animation if exists
    if(music_album_img) {
        lv_anim_del(music_album_img, music_anim_cb);
    }
    
    // Reset global pointers
    music_album_img      = NULL;
    music_play_btn_label = NULL;
    music_title_label    = NULL;
    music_artist_label   = NULL;
    video_play_btn_label = NULL;
    video_disp_area      = NULL;
    album_img_obj        = NULL;
    album_zoom_label     = NULL;
}

// ---------------- 视频功能模块 ----------------

static void video_btn_event_cb(lv_event_t * e)
{
    intptr_t code = (intptr_t)lv_event_get_user_data(e);

    if(code == 1) { // Play/Pause
        if(is_video_playing) {
            send_mplayer_cmd("pause\n");
            is_video_playing = false;
            if(video_play_btn_label) lv_label_set_text(video_play_btn_label, LV_SYMBOL_PLAY);
        } else {
            send_mplayer_cmd("pause\n");
            if(music_fd < 0) {
                char cmd[512];
                system("killall mplayer > /dev/null 2>&1");
                
                int w = 680 - 8;
                int h = 320 - 8;
                int x = 110 + 4;
                int y = 95 + 4;

                if(video_disp_area) {
                    lv_obj_update_layout(video_disp_area);
                    lv_area_t coords;
                    lv_obj_get_coords(video_disp_area, &coords);
                    x = coords.x1 + 4;
                    y = coords.y1 + 4;
                    w = lv_area_get_width(&coords) - 8;
                    h = lv_area_get_height(&coords) - 8;
                }

                if(w <= 0 || h <= 0) { w=672; h=312; x=114; y=99; }
                w = w & ~1;
                h = h & ~1;

                snprintf(cmd, sizeof(cmd),
                         "mplayer -idle -slave -input file=%s -geometry %dx%d+%d+%d -zoom -x %d -y %d %s &",
                         MUSIC_FIFO, w, h, x, y, w, h, video_files[video_current_idx]);
                system(cmd);
            }
            is_video_playing = true;
            if(video_play_btn_label) lv_label_set_text(video_play_btn_label, LV_SYMBOL_PAUSE);
        }
    } else if(code == 2) { // Stop
        stop_mplayer();
        is_video_playing = false;
        if(video_play_btn_label) lv_label_set_text(video_play_btn_label, LV_SYMBOL_PLAY);
    } else if(code == 3) {
        send_mplayer_cmd("volume 100\n");
    } else if(code == 4) {
        send_mplayer_cmd("volume -100\n");
    } else if(code == 5) {
        send_mplayer_cmd("mute 1\n");
    } else if(code == 6) {
        send_mplayer_cmd("mute 0\n");
    } else if(code == 7) {
        send_mplayer_cmd("seek 10\n");
    } else if(code == 8) {
        send_mplayer_cmd("seek -10\n");
    }
}

static void video_nav_event_cb(lv_event_t * e)
{
    intptr_t code = (intptr_t)lv_event_get_user_data(e);
    if(code == 0) {
        video_current_idx--;
        if(video_current_idx < 0) video_current_idx = VIDEO_TOTAL_COUNT - 1;
    } else if(code == 1) {
        video_current_idx++;
        if(video_current_idx >= VIDEO_TOTAL_COUNT) video_current_idx = 0;
    }

    const char * path = video_files[video_current_idx];

    if(is_video_playing) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "loadfile %s\n", path);
        send_mplayer_cmd(cmd);
        if(video_play_btn_label) lv_label_set_text(video_play_btn_label, LV_SYMBOL_PAUSE);
        is_video_playing = true;
    } else {
        if(access(MUSIC_FIFO, F_OK) == -1) mkfifo(MUSIC_FIFO, 0777);
        char cmd[512];
        system("killall mplayer > /dev/null 2>&1");
        
        int w = 680 - 8;
        int h = 320 - 8;
        int x = 110 + 4;
        int y = 95 + 4;

        if(video_disp_area) {
            lv_obj_update_layout(video_disp_area);
            lv_area_t coords;
            lv_obj_get_coords(video_disp_area, &coords);
            x = coords.x1 + 4;
            y = coords.y1 + 4;
            w = lv_area_get_width(&coords) - 8;
            h = lv_area_get_height(&coords) - 8;
        }

        if(w <= 0 || h <= 0) { w=672; h=312; x=114; y=99; }
        w = w & ~1;
        h = h & ~1;

        snprintf(cmd, sizeof(cmd),
                 "mplayer -idle -slave -input file=%s -geometry %dx%d+%d+%d -zoom -x %d -y %d %s &", MUSIC_FIFO, w,
                 h, x, y, w, h, path);
        system(cmd);
        is_video_playing = true;
        if(video_play_btn_label) lv_label_set_text(video_play_btn_label, LV_SYMBOL_PAUSE);
    }
}

static void video_volume_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    int val           = (int)lv_slider_get_value(slider);
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "volume %d 1\n", val);
    send_mplayer_cmd(cmd);
}

static void video_drag_redraw_event_cb(lv_event_t * e)
{
    if(!is_video_playing) {
        Draw_Full_Background();
    }
}

// 创建视频全屏界面
void create_video_screen(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * bg_img = lv_img_create(scr);
    lv_img_set_src(bg_img, "A:zhuyebak.png");
    lv_obj_center(bg_img);

    lv_obj_t * main_cont = lv_obj_create(scr);
    lv_obj_set_size(main_cont, 760, 420);
    lv_obj_align(main_cont, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(main_cont, 0, 0);

    lv_obj_set_layout(main_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(main_cont, 0, 0);
    lv_obj_set_style_pad_gap(main_cont, 0, 0);

    video_disp_area = lv_obj_create(main_cont);
    lv_obj_set_width(video_disp_area, 680);
    lv_obj_set_flex_grow(video_disp_area, 1);
    lv_obj_set_style_bg_color(video_disp_area, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(video_disp_area, LV_OPA_COVER, 0);

    lv_obj_add_event_cb(video_disp_area, video_drag_redraw_event_cb, LV_EVENT_PRESSING, NULL);

    lv_obj_set_style_border_width(video_disp_area, 2, 0);
    lv_obj_set_style_border_color(video_disp_area, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_border_opa(video_disp_area, LV_OPA_80, 0);
    lv_obj_set_style_radius(video_disp_area, 10, 0);
    lv_obj_set_style_shadow_width(video_disp_area, 20, 0);
    lv_obj_set_style_shadow_color(video_disp_area, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_shadow_opa(video_disp_area, LV_OPA_40, 0);
    lv_obj_clear_flag(video_disp_area, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * tips = lv_label_create(video_disp_area);
    lv_label_set_text(tips, "点击播放按钮开始播放视频");
    lv_obj_set_style_text_color(tips, lv_color_hex(0x808080), 0);
    lv_obj_set_style_text_font(tips, get_chinese_font(), 0);
    lv_obj_center(tips);

    lv_obj_t * btn_prev = lv_btn_create(scr);
    lv_obj_set_size(btn_prev, 40, 40);
    lv_obj_set_style_radius(btn_prev, 20, 0);
    lv_obj_set_style_bg_color(btn_prev, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(btn_prev, LV_OPA_80, 0);
    lv_obj_align(btn_prev, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_t * label_prev = lv_label_create(btn_prev);
    lv_label_set_text(label_prev, LV_SYMBOL_LEFT);
    lv_obj_center(label_prev);
    lv_obj_add_event_cb(btn_prev, video_nav_event_cb, LV_EVENT_CLICKED, (void *)0);

    lv_obj_t * btn_next = lv_btn_create(scr);
    lv_obj_set_size(btn_next, 40, 40);
    lv_obj_set_style_radius(btn_next, 20, 0);
    lv_obj_set_style_bg_color(btn_next, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(btn_next, LV_OPA_80, 0);
    lv_obj_align(btn_next, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_t * label_next = lv_label_create(btn_next);
    lv_label_set_text(label_next, LV_SYMBOL_RIGHT);
    lv_obj_center(label_next);
    lv_obj_add_event_cb(btn_next, video_nav_event_cb, LV_EVENT_CLICKED, (void *)1);

    lv_obj_t * ctrl_bar = lv_obj_create(main_cont);
    lv_obj_set_size(ctrl_bar, 750, 60);
    lv_obj_set_style_bg_opa(ctrl_bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_bar, 0, 0);
    lv_obj_clear_flag(ctrl_bar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_layout(ctrl_bar, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(ctrl_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * btn_back = lv_btn_create(ctrl_bar);
    lv_obj_set_size(btn_back, 40, 40);
    lv_obj_set_style_radius(btn_back, 20, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x808080), 0);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_80, 0);
    lv_obj_t * label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, LV_SYMBOL_HOME);
    lv_obj_center(label_back);
    lv_obj_add_event_cb(btn_back, media_back_event_cb, LV_EVENT_CLICKED, NULL);

    // Create other control buttons (simplified from original for brevity, but keeping logic)
    // Seek -10
    lv_obj_t * btn_seek_n = lv_btn_create(ctrl_bar);
    lv_obj_set_size(btn_seek_n, 40, 40);
    lv_obj_set_style_radius(btn_seek_n, 20, 0);
    lv_obj_set_style_bg_color(btn_seek_n, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(btn_seek_n, LV_OPA_80, 0);
    lv_obj_t * lbl_sn = lv_label_create(btn_seek_n);
    lv_label_set_text(lbl_sn, LV_SYMBOL_PREV);
    lv_obj_center(lbl_sn);
    lv_obj_add_event_cb(btn_seek_n, video_btn_event_cb, LV_EVENT_CLICKED, (void *)8);

    // Play/Pause
    lv_obj_t * btn_play2 = lv_btn_create(ctrl_bar);
    lv_obj_set_size(btn_play2, 50, 50);
    lv_obj_set_style_radius(btn_play2, 25, 0);
    lv_obj_set_style_bg_color(btn_play2, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_bg_opa(btn_play2, LV_OPA_80, 0);
    video_play_btn_label = lv_label_create(btn_play2);
    lv_label_set_text(video_play_btn_label, is_video_playing ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
    lv_obj_center(video_play_btn_label);
    lv_obj_add_event_cb(btn_play2, video_btn_event_cb, LV_EVENT_CLICKED, (void *)1);

    // Stop
    lv_obj_t * btn_stop = lv_btn_create(ctrl_bar);
    lv_obj_set_size(btn_stop, 40, 40);
    lv_obj_set_style_radius(btn_stop, 20, 0);
    lv_obj_set_style_bg_color(btn_stop, lv_color_hex(0xFF4040), 0);
    lv_obj_set_style_bg_opa(btn_stop, LV_OPA_80, 0);
    lv_obj_t * label_stop = lv_label_create(btn_stop);
    lv_label_set_text(label_stop, LV_SYMBOL_STOP);
    lv_obj_center(label_stop);
    lv_obj_add_event_cb(btn_stop, video_btn_event_cb, LV_EVENT_CLICKED, (void *)2);

    // Seek +10
    lv_obj_t * btn_seek_p = lv_btn_create(ctrl_bar);
    lv_obj_set_size(btn_seek_p, 40, 40);
    lv_obj_set_style_radius(btn_seek_p, 20, 0);
    lv_obj_set_style_bg_color(btn_seek_p, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(btn_seek_p, LV_OPA_80, 0);
    lv_obj_t * lbl_sp = lv_label_create(btn_seek_p);
    lv_label_set_text(lbl_sp, LV_SYMBOL_NEXT);
    lv_obj_center(lbl_sp);
    lv_obj_add_event_cb(btn_seek_p, video_btn_event_cb, LV_EVENT_CLICKED, (void *)7);

    // Volume
    lv_obj_t * vol_cont = lv_obj_create(ctrl_bar);
    lv_obj_set_size(vol_cont, 220, 50);
    lv_obj_set_style_bg_opa(vol_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vol_cont, 0, 0);
    lv_obj_set_layout(vol_cont, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(vol_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(vol_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(vol_cont, 0, 0);
    lv_obj_set_style_pad_gap(vol_cont, 10, 0);

    lv_obj_t * vol_icon = lv_label_create(vol_cont);
    lv_label_set_text(vol_icon, LV_SYMBOL_VOLUME_MAX);
    lv_obj_set_style_text_color(vol_icon, lv_color_hex(0xAAAAAA), 0);

    lv_obj_t * vol_slider = lv_slider_create(vol_cont);
    lv_obj_set_width(vol_slider, 140);
    lv_slider_set_range(vol_slider, 0, 100);
    lv_slider_set_value(vol_slider, 80, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(vol_slider, lv_color_hex(0x404040), LV_PART_MAIN);
    lv_obj_set_style_bg_color(vol_slider, lv_color_hex(0x00D2FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(vol_slider, lv_color_white(), LV_PART_KNOB);
    lv_obj_add_event_cb(vol_slider, video_volume_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

// ---------------- 相册功能模块 ----------------

static void anim_set_img_opa(void * obj, int32_t v)
{
    lv_obj_set_style_img_opa((lv_obj_t *)obj, v, 0);
}

static void album_fade_ready_cb(lv_anim_t * a)
{
    album_current_idx = album_pending_idx;
    album_zoom_val    = 256;
    update_album_view();

    lv_anim_t fade_in;
    lv_anim_init(&fade_in);
    lv_anim_set_var(&fade_in, album_img_obj);
    lv_anim_set_values(&fade_in, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&fade_in, 300);
    lv_anim_set_exec_cb(&fade_in, (lv_anim_exec_xcb_t)anim_set_img_opa);
    lv_anim_set_ready_cb(&fade_in, NULL);
    is_album_switching = false;
    lv_anim_start(&fade_in);
}

static void update_album_view(void)
{
    if(!album_img_obj) return;

    lv_img_set_src(album_img_obj, album_imgs[album_current_idx]);
    lv_img_set_zoom(album_img_obj, album_zoom_val);

    if(album_zoom_label) {
        lv_label_set_text_fmt(album_zoom_label, "%d%%", (int)(album_zoom_val * 100 / 256));
    }
}

static void album_btn_event_cb(lv_event_t * e)
{
    intptr_t code = (intptr_t)lv_event_get_user_data(e);

    if(is_album_switching && (code == 0 || code == 1)) return;

    switch(code) {
        case 0: // Prev
            is_album_switching = true;
            if(album_current_idx > 0) album_pending_idx = album_current_idx - 1;
            else album_pending_idx = ALBUM_TOTAL_IMGS - 1;
            {
                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, album_img_obj);
                lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
                lv_anim_set_time(&a, 300);
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)anim_set_img_opa);
                lv_anim_set_ready_cb(&a, album_fade_ready_cb);
                lv_anim_start(&a);
            }
            break;
        case 1: // Next
            is_album_switching = true;
            if(album_current_idx < ALBUM_TOTAL_IMGS - 1) album_pending_idx = album_current_idx + 1;
            else album_pending_idx = 0;
            {
                lv_anim_t a;
                lv_anim_init(&a);
                lv_anim_set_var(&a, album_img_obj);
                lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
                lv_anim_set_time(&a, 300);
                lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)anim_set_img_opa);
                lv_anim_set_ready_cb(&a, album_fade_ready_cb);
                lv_anim_start(&a);
            }
            break;
        case 2: // Zoom In
            if(album_zoom_val < 1024) {
                album_zoom_val += 64;
                update_album_view();
            }
            break;
        case 3: // Zoom Out
            if(album_zoom_val > 64) {
                album_zoom_val -= 64;
                update_album_view();
            }
            break;
    }
}

void create_album_screen(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * bg_img = lv_img_create(scr);
    lv_img_set_src(bg_img, "A:zhuyebak.png");
    lv_obj_center(bg_img);

    lv_obj_t * img_cont = lv_obj_create(scr);
    lv_obj_set_size(img_cont, 700, 350);
    lv_obj_align(img_cont, LV_ALIGN_CENTER, 0, -20);
    lv_obj_set_style_bg_opa(img_cont, LV_OPA_TRANSP, 0);

    lv_obj_set_style_border_width(img_cont, 2, 0);
    lv_obj_set_style_border_color(img_cont, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_border_opa(img_cont, LV_OPA_80, 0);
    lv_obj_set_style_radius(img_cont, 10, 0);
    lv_obj_set_style_shadow_width(img_cont, 20, 0);
    lv_obj_set_style_shadow_color(img_cont, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_shadow_opa(img_cont, LV_OPA_40, 0);
    lv_obj_clear_flag(img_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(img_cont, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    album_img_obj = lv_img_create(img_cont);
    lv_obj_center(album_img_obj);

    lv_obj_t * btn_prev = lv_btn_create(scr);
    lv_obj_set_size(btn_prev, 50, 50);
    lv_obj_set_style_radius(btn_prev, 25, 0);
    lv_obj_set_style_bg_color(btn_prev, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(btn_prev, LV_OPA_80, 0);
    lv_obj_align(btn_prev, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_t * label_prev = lv_label_create(btn_prev);
    lv_label_set_text(label_prev, LV_SYMBOL_LEFT);
    lv_obj_center(label_prev);
    lv_obj_add_event_cb(btn_prev, album_btn_event_cb, LV_EVENT_CLICKED, (void *)0);

    lv_obj_t * btn_next = lv_btn_create(scr);
    lv_obj_set_size(btn_next, 50, 50);
    lv_obj_set_style_radius(btn_next, 25, 0);
    lv_obj_set_style_bg_color(btn_next, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(btn_next, LV_OPA_80, 0);
    lv_obj_align(btn_next, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_t * label_next = lv_label_create(btn_next);
    lv_label_set_text(label_next, LV_SYMBOL_RIGHT);
    lv_obj_center(label_next);
    lv_obj_add_event_cb(btn_next, album_btn_event_cb, LV_EVENT_CLICKED, (void *)1);

    album_zoom_label = NULL;
    album_current_idx = 0;
    album_zoom_val = 256;
    update_album_view();

    lv_obj_t * btn_zoom_in = lv_btn_create(scr);
    lv_obj_set_size(btn_zoom_in, 50, 50);
    lv_obj_set_style_radius(btn_zoom_in, 25, 0);
    lv_obj_set_style_bg_color(btn_zoom_in, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(btn_zoom_in, LV_OPA_80, 0);
    lv_obj_align(btn_zoom_in, LV_ALIGN_BOTTOM_MID, 40, -20);
    lv_obj_t * label_zoom_in = lv_label_create(btn_zoom_in);
    lv_label_set_text(label_zoom_in, LV_SYMBOL_PLUS);
    lv_obj_center(label_zoom_in);
    lv_obj_add_event_cb(btn_zoom_in, album_btn_event_cb, LV_EVENT_CLICKED, (void *)2);

    lv_obj_t * btn_zoom_out = lv_btn_create(scr);
    lv_obj_set_size(btn_zoom_out, 50, 50);
    lv_obj_set_style_radius(btn_zoom_out, 25, 0);
    lv_obj_set_style_bg_color(btn_zoom_out, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(btn_zoom_out, LV_OPA_80, 0);
    lv_obj_align(btn_zoom_out, LV_ALIGN_BOTTOM_MID, -40, -20);
    lv_obj_t * label_zoom_out = lv_label_create(btn_zoom_out);
    lv_label_set_text(label_zoom_out, LV_SYMBOL_MINUS);
    lv_obj_center(label_zoom_out);
    lv_obj_add_event_cb(btn_zoom_out, album_btn_event_cb, LV_EVENT_CLICKED, (void *)3);

    lv_obj_t * btn_back = lv_btn_create(scr);
    lv_obj_set_size(btn_back, 40, 40);
    lv_obj_set_style_radius(btn_back, 20, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x808080), 0);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_80, 0);
    lv_obj_set_pos(btn_back, 20, 20);
    lv_obj_t * label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, LV_SYMBOL_HOME);
    lv_obj_center(label_back);
    lv_obj_add_event_cb(btn_back, media_back_event_cb, LV_EVENT_CLICKED, NULL);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}
