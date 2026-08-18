#include "game.h"
#include "common.h"
#include "home.h"
#include "media.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void game_back_event_cb(lv_event_t * e)
{
    create_main_dashboard();
}

static void game1_event_cb(lv_event_t * e)
{
    reset_media_state();
    system("killall InfoNES > /dev/null 2>&1");
    usleep(200000);

    printf("Starting Super Mario...\n");
    int ret = system("./InfoNES mario.nes");
    printf("Game exited with code: %d\n", ret);

    lv_obj_invalidate(lv_scr_act());
}

static void game2_event_cb(lv_event_t * e)
{
    reset_media_state();
    system("killall InfoNES > /dev/null 2>&1");
    usleep(200000);

    printf("Starting Contra (Super_Contra@US.nes)...\n");
    int ret = system("./InfoNES Super_Contra@US.nes");
    printf("Game exited with code: %d\n", ret);

    lv_obj_invalidate(lv_scr_act());
}

static void game3_event_cb(lv_event_t * e)
{
    reset_media_state();
    system("killall InfoNES > /dev/null 2>&1");
    usleep(200000);

    printf("Starting TMNT (Teenage_Mutant_Ninja_Turtles_-_Tournament_Fighters@US.nes)...\n");
    int ret = system("./InfoNES Teenage_Mutant_Ninja_Turtles_-_Tournament_Fighters@US.nes");
    printf("Game exited with code: %d\n", ret);

    lv_obj_invalidate(lv_scr_act());
}

void create_game_screen(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * bg_img = lv_img_create(scr);
    lv_img_set_src(bg_img, "A:zhuyebak.png");
    lv_obj_center(bg_img);

    lv_obj_t * title = lv_label_create(scr);
    lv_label_set_text(title, "????");
    lv_obj_set_style_text_font(title, get_chinese_font(), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t * game_grid = lv_obj_create(scr);
    lv_obj_set_size(game_grid, 600, 200);
    lv_obj_align(game_grid, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(game_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(game_grid, 0, 0);
    lv_obj_clear_flag(game_grid, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_layout(game_grid, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(game_grid, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(game_grid, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * btn_game1 = create_sidebar_btn(game_grid, "A:game1.png", "超级玛丽");
    lv_obj_t * img1 = lv_obj_get_child(btn_game1, 0);
    lv_obj_set_style_radius(img1, 16, 0);
    lv_obj_set_style_clip_corner(img1, true, 0);
    lv_obj_add_event_cb(btn_game1, game1_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_game2 = create_sidebar_btn(game_grid, "A:game2.png", "魂斗罗");
    lv_obj_t * img2 = lv_obj_get_child(btn_game2, 0);
    lv_obj_set_style_radius(img2, 16, 0);
    lv_obj_set_style_clip_corner(img2, true, 0);
    lv_obj_add_event_cb(btn_game2, game2_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_game3 = create_sidebar_btn(game_grid, "A:game3.png", "忍者神龟");
    lv_obj_t * img3 = lv_obj_get_child(btn_game3, 0);
    lv_obj_set_style_radius(img3, 16, 0);
    lv_obj_set_style_clip_corner(img3, true, 0);
    lv_obj_add_event_cb(btn_game3, game3_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_back = lv_btn_create(scr);
    lv_obj_set_size(btn_back, 40, 40);
    lv_obj_set_style_radius(btn_back, 20, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x808080), 0);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_80, 0);
    lv_obj_set_pos(btn_back, 20, 20);

    lv_obj_t * label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, LV_SYMBOL_HOME);
    lv_obj_center(label_back);

    lv_obj_add_event_cb(btn_back, game_back_event_cb, LV_EVENT_CLICKED, NULL);
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}
