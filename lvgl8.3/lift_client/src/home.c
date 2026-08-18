#include "home.h"
#include "style.h"
#include "elevator.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "advertisement.h"
// 动画回调：渐变动画透明度
static void anim_opa_cb(void * obj, int32_t v)
{
    lv_obj_set_style_img_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

static lv_obj_t * g_btnm = NULL; // 按钮矩阵引用

// 电梯到达某楼层 → 清除对应按钮高亮
static void on_floor_done(int floor)
{
    if(!g_btnm) {
        return;
    }
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", floor);
    // 遍历所有按钮（5行×3列=15个），找到匹配文字则清除高亮
    for(uint16_t i = 0; i < 15; i++) {
        const char * txt = lv_btnmatrix_get_btn_text(g_btnm, i);
        if(txt && strcmp(txt, buf) == 0) {
            lv_btnmatrix_clear_btn_ctrl(g_btnm, i, LV_BTNMATRIX_CTRL_CHECKED);
            {
                return;
            }
        }
    }
}

// 按键回调
void matrix_event_cb(lv_event_t * e)
{
    if(lv_event_get_code(e) != LV_EVENT_CLICKED) {
        return;
    }

    lv_obj_t * mat = lv_event_get_target(e);
    // 获取点击按钮下标（0开始）
    uint16_t btn_idx = lv_btnmatrix_get_selected_btn(mat);

    // 获取按钮上的字符串
    const char * str = lv_btnmatrix_get_btn_text(mat, btn_idx);
    if(str[0] == '\0') {
        return;
    }
    // 功能键
    if(strcmp(str, "开门") == 0) {
        elevator_door_extend(); // 门开着就延长，没开就忽略
        return;
    }
    if(strcmp(str, "关门") == 0) {
        elevator_door_close(); // 门开着就立即清等待时间
        return;
    }
    if(strcmp(str, "警报") == 0) {
        ad_send_alarm();
        return;
    }

    // 数字楼层：当前楼层不高亮
    int f = atoi(str);
    if(f != elevator_get_floor()) {
        elevator_add_request(f);
        lv_btnmatrix_set_btn_ctrl(mat, btn_idx, LV_BTNMATRIX_CTRL_CHECKED);
    }
}

lv_obj_t * show_home(void)
{
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_size(scr, 800, 480);
    lv_obj_t * bag = lv_img_create(scr);
    lv_img_set_src(bag, "A:bag.png");

    // 广告部分
    lv_obj_t * ad_card = lv_obj_create(scr);
    lv_obj_add_style(ad_card, style_get_card(), 0);
    lv_obj_set_size(ad_card, 280, 360);
    lv_obj_set_pos(ad_card, 30, 40);

    // 广告标题：紧贴顶部居中
    lv_obj_t * ad_title = lv_label_create(ad_card);
    lv_obj_add_style(ad_title, style_get_label(), 0);
    lv_obj_align(ad_title, LV_ALIGN_TOP_MID, 0, 2);

    // 广告图片：居中，等比缩放
    lv_obj_t * ad_img = lv_img_create(ad_card);
    lv_obj_align(ad_img, LV_ALIGN_CENTER, 0, 10);
    lv_img_set_zoom(ad_img, 350); // 进行缩放 256=1x, 350≈1.37x */

    /* 天气信息：广告卡片下方 */
    lv_obj_t *wea_label = lv_label_create(scr);
    lv_obj_add_style(wea_label, style_get_label(), 0);
    lv_obj_align_to(wea_label, ad_card, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
    lv_label_set_long_mode(wea_label, LV_LABEL_LONG_DOT);
    lv_label_set_text(wea_label, "加载天气...");

    /* 绑定给广告模块 */
    ad_bind_ui(ad_card, ad_title, ad_img);
    ad_bind_weather_label(wea_label);

    // 楼层显示部分
    lv_obj_t * floor_card = lv_obj_create(scr);
    lv_obj_add_style(floor_card, style_get_card(), 0);
    lv_obj_set_size(floor_card, 180, 320);
    lv_obj_set_pos(floor_card, 330, 60);
    // 垂直 flex 布局：上箭头 → 楼层号 → 下箭头，三均分
    lv_obj_clear_flag(floor_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(floor_card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(floor_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(floor_card, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 上行箭头
    lv_obj_t * upimg = lv_img_create(floor_card);
    lv_img_set_src(upimg, "A:up.png");

    // 楼层号
    lv_obj_t * labelfloo = lv_label_create(floor_card);
    char num[8];
    sprintf(num, "%d", init_floor());
    lv_label_set_text(labelfloo, num);
    lv_obj_add_style(labelfloo, style_get_floo(), 0);

    // 下行箭头
    lv_obj_t * donwimg = lv_img_create(floor_card);
    lv_img_set_src(donwimg, "A:donw.png");

    // 门状态文字
    lv_obj_t * door_label = lv_label_create(floor_card);
    lv_obj_add_style(door_label, style_get_label(), 0);
    lv_label_set_text(door_label, "停靠中");

    // 绑定 UI：电梯状态变化自动刷新
    elevator_bind_ui(labelfloo, upimg, donwimg, door_label);

    // 上行箭头呼吸动画
    lv_anim_t a_up;
    lv_anim_init(&a_up);
    lv_anim_set_var(&a_up, upimg);
    lv_anim_set_exec_cb(&a_up, anim_opa_cb);
    lv_anim_set_values(&a_up, LV_OPA_10, LV_OPA_COVER);
    lv_anim_set_time(&a_up, 500);
    lv_anim_set_playback_time(&a_up, 500);
    lv_anim_set_repeat_count(&a_up, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a_up);

    // 下行箭头呼吸动画，延迟 300ms 交替
    lv_anim_t a_down;
    lv_anim_init(&a_down);
    lv_anim_set_var(&a_down, donwimg);
    lv_anim_set_exec_cb(&a_down, anim_opa_cb);
    lv_anim_set_values(&a_down, LV_OPA_10, LV_OPA_COVER);
    lv_anim_set_time(&a_down, 500);
    lv_anim_set_playback_time(&a_down, 500);
    lv_anim_set_repeat_count(&a_down, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_delay(&a_down, 250);
    lv_anim_start(&a_down);
    // 右边
    lv_obj_t * btn_card = lv_obj_create(scr);
    lv_obj_add_style(btn_card, style_get_card(), 0);
    lv_obj_set_size(btn_card, 240, 360);
    lv_obj_set_pos(btn_card, 530, 10);
    lv_obj_clear_flag(btn_card, LV_OBJ_FLAG_SCROLLABLE);

    // 电梯按钮
    static const char * btnm_map[] = {"10", "11", "12", "\n", "7", "8",  "9",    "\n",   "4",    "5",
                                      "6",  "\n", "1",  "2",  "3", "\n", "开门", "关门", "警报", ""};

    lv_obj_t * btnm = lv_btnmatrix_create(btn_card);
    lv_btnmatrix_set_map(btnm, btnm_map);

    lv_obj_set_size(btnm, lv_pct(100), lv_pct(100));
    lv_obj_center(btnm);
    lv_obj_set_style_bg_opa(btnm, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(btnm, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btnm, 10, LV_PART_MAIN);
    lv_obj_set_style_pad_row(btnm, 15, LV_PART_MAIN);
    lv_obj_set_style_pad_column(btnm, 15, LV_PART_MAIN);
    // 绑定事件
    lv_obj_add_event_cb(btnm, matrix_event_cb, LV_EVENT_CLICKED, NULL);

    // 浅底深字
    lv_obj_add_style(btnm, style_get_btn(), LV_PART_ITEMS);
    lv_obj_add_style(btnm, style_get_label(), LV_PART_ITEMS);

    // 按下：亮蓝
    lv_obj_add_style(btnm, style_get_pr(), LV_PART_ITEMS | LV_STATE_PRESSED);

    // 等待中（CHECKED）：橙色
    lv_obj_add_style(btnm, style_get_wait(), LV_PART_ITEMS | LV_STATE_CHECKED);

    g_btnm = btnm;
    elevator_on_floor_served(on_floor_done); // 触发回调,进行消除橙色

    return scr;
}
