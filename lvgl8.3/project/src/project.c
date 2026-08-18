#include "project.h"
#include "common.h"
#include "auth.h"
#include "home.h"
#include "media.h"
#include <signal.h>
#include <stdio.h>

static void on_boot_timer_finished(lv_timer_t * t)
{
    create_login_screen();
    lv_timer_del(t);
}

static void logo_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_t * gifobj = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(gifobj, lv_color_hex(0x020508), 0);
        lv_obj_t * gif = lv_gif_create(gifobj);
        lv_gif_set_src(gif, "A:kaiji.gif");
        lv_obj_center(gif);
        lv_scr_load_anim(gifobj, LV_SCR_LOAD_ANIM_NONE, 300, 0, true);
        lv_timer_create(on_boot_timer_finished, 4000, NULL);
    }
}

void qidong(void)
{
    signal(SIGPIPE, SIG_IGN);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x020508), 0);

    lv_obj_t * logo = lv_img_create(lv_scr_act());
    lv_img_set_src(logo, "A:logo.png");
    lv_obj_center(logo);

    lv_obj_set_style_img_recolor(logo, lv_color_white(), 0);
    lv_obj_set_style_img_recolor_opa(logo, LV_OPA_20, 0);

    lv_obj_add_flag(logo, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(logo, logo_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(logo, logo_event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(logo, logo_event_cb, LV_EVENT_CLICKED, NULL);
}
