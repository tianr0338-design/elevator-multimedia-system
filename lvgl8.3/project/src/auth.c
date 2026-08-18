#include "auth.h"
#include "common.h"
#include "home.h"
#include <stdio.h>
#include <string.h>

// 登录界面的输入框和键盘对象
static lv_obj_t * login_username_ta;
static lv_obj_t * login_password_ta;
static lv_obj_t * login_keyboard;

// 注册界面的输入框和键盘对象
static lv_obj_t * register_username_ta;
static lv_obj_t * register_password_ta;
static lv_obj_t * register_keyboard;

// 函数声明
static void create_register_screen(void);

// 获取用户文件路径
static void get_user_filepath(const char * username, char * filepath_buf)
{
    sprintf(filepath_buf, "user_%s.txt", username);
}

// 检查用户是否存在
static bool check_user_exists(const char * username)
{
    char filepath[128];
    get_user_filepath(username, filepath);

    FILE * fp = fopen(filepath, "r");
    if(fp) {
        fclose(fp);
        return true;
    }
    return false;
}

// 保存新用户
static bool save_new_user(const char * username, const char * password)
{
    char filepath[128];
    get_user_filepath(username, filepath);

    FILE * fp = fopen(filepath, "w");
    if(fp) {
        fprintf(fp, "%s\n%s", username, password);
        fclose(fp);
        return true;
    }
    return false;
}

// 验证登录
static bool verify_login(const char * username, const char * password)
{
    if(!check_user_exists(username)) {
        return false;
    }

    char filepath[128];
    get_user_filepath(username, filepath);

    FILE * fp = fopen(filepath, "r");
    if(fp) {
        char file_user[32] = {0};
        char file_pass[32] = {0};

        fscanf(fp, "%s", file_user);
        fscanf(fp, "%s", file_pass);
        fclose(fp);

        if(strcmp(password, file_pass) == 0) {
            return true;
        }
    }
    return false;
}

// 确保管理员存在
void ensure_admin_exists(void)
{
    if(!check_user_exists("admin")) {
        save_new_user("admin", "123456");
    }
}

// 注册提交事件回调
static void register_submit_event_cb(lv_event_t * e)
{
    const char * user = lv_textarea_get_text(register_username_ta);
    const char * pass = lv_textarea_get_text(register_password_ta);

    if(strlen(user) == 0 || strlen(pass) == 0) {
        show_toast("用户名或密码不能为空", lv_color_hex(0xFF4444));
        return;
    }

    if(check_user_exists(user)) {
        show_toast("用户已存在", lv_color_hex(0xFF4444));
        return;
    }

    if(save_new_user(user, pass)) {
        show_toast("注册成功", lv_color_hex(0x44FF44));
        create_login_screen();
    } else {
        show_toast("注册失败", lv_color_hex(0xFF4444));
    }
}

static void back_to_login_event_cb(lv_event_t * e)
{
    create_login_screen();
}

static void login_btn_event_cb(lv_event_t * e)
{
    const char * user = lv_textarea_get_text(login_username_ta);
    const char * pass = lv_textarea_get_text(login_password_ta);

    if(strlen(user) == 0 || strlen(pass) == 0) {
        show_toast("用户名或密码不能为空", lv_color_hex(0xFF4444));
        return;
    }

    if(verify_login(user, pass)) {
        show_toast("登录成功", lv_color_hex(0x44FF44));
        create_main_dashboard();
    } else {
        show_toast("用户名或密码错误", lv_color_hex(0xFF4444));
    }
}

static void to_register_btn_event_cb(lv_event_t * e)
{
    create_register_screen();
}

// 创建注册界面
static void create_register_screen(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * bg = lv_img_create(scr);
    lv_img_set_src(bg, "A:bak.png");
    lv_obj_center(bg);
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

    register_keyboard = lv_keyboard_create(scr);
    lv_obj_add_flag(register_keyboard, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * panel = lv_obj_create(scr);
    lv_obj_set_size(panel, 400, 320);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1A1C23), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_90, 0);
    lv_obj_set_style_radius(panel, 20, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x00D200), 0);
    lv_obj_set_style_border_opa(panel, LV_OPA_50, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "用户注册");
    lv_obj_set_style_text_font(title, get_chinese_font(), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    register_username_ta = lv_textarea_create(panel);
    lv_textarea_set_one_line(register_username_ta, true);
    lv_textarea_set_placeholder_text(register_username_ta, "请输入用户名");
    lv_obj_set_width(register_username_ta, 280);
    lv_obj_align(register_username_ta, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_text_font(register_username_ta, get_chinese_font(), 0);
    lv_obj_add_event_cb(register_username_ta, common_kb_event_cb, LV_EVENT_ALL, register_keyboard);

    register_password_ta = lv_textarea_create(panel);
    lv_textarea_set_one_line(register_password_ta, true);
    lv_textarea_set_placeholder_text(register_password_ta, "请输入密码");
    lv_obj_set_width(register_password_ta, 280);
    lv_obj_align(register_password_ta, LV_ALIGN_TOP_MID, 0, 110);
    lv_obj_set_style_text_font(register_password_ta, get_chinese_font(), 0);
    lv_obj_add_event_cb(register_password_ta, common_kb_event_cb, LV_EVENT_ALL, register_keyboard);

    lv_obj_t * btn_main = lv_btn_create(panel);
    lv_obj_set_size(btn_main, 280, 45);
    lv_obj_align(btn_main, LV_ALIGN_TOP_MID, 0, 170);
    lv_obj_set_style_bg_color(btn_main, lv_color_hex(0x00D200), 0);
    lv_obj_add_event_cb(btn_main, register_submit_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * label_main = lv_label_create(btn_main);
    lv_label_set_text(label_main, "立即注册");
    lv_obj_set_style_text_font(label_main, get_chinese_font(), 0);
    lv_obj_center(label_main);

    lv_obj_t * btn_sub = lv_btn_create(panel);
    lv_obj_set_size(btn_sub, 280, 40);
    lv_obj_align(btn_sub, LV_ALIGN_TOP_MID, 0, 225);
    lv_obj_set_style_bg_opa(btn_sub, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_sub, 0, 0);
    lv_obj_add_event_cb(btn_sub, back_to_login_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * label_sub = lv_label_create(btn_sub);
    lv_label_set_text(label_sub, "返回登录");
    lv_obj_set_style_text_font(label_sub, get_chinese_font(), 0);
    lv_obj_set_style_text_color(label_sub, lv_color_hex(0x888888), 0);
    lv_obj_center(label_sub);

    lv_obj_move_foreground(register_keyboard);
}

// 创建登录界面
void create_login_screen(void)
{
    lv_obj_t * scr = lv_obj_create(NULL);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t * bg = lv_img_create(scr);
    lv_img_set_src(bg, "A:bak.png");
    lv_obj_center(bg);
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

    login_keyboard = lv_keyboard_create(scr);
    lv_obj_add_flag(login_keyboard, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t * panel = lv_obj_create(scr);
    lv_obj_set_size(panel, 400, 320);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x1A1C23), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_90, 0);
    lv_obj_set_style_radius(panel, 20, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(0x00D2FF), 0);
    lv_obj_set_style_border_opa(panel, LV_OPA_50, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "系统登录");
    lv_obj_set_style_text_font(title, get_chinese_font(), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    login_username_ta = lv_textarea_create(panel);
    lv_textarea_set_one_line(login_username_ta, true);
    lv_textarea_set_placeholder_text(login_username_ta, "请输入用户名");
    lv_obj_set_width(login_username_ta, 280);
    lv_obj_align(login_username_ta, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_text_font(login_username_ta, get_chinese_font(), 0);
    lv_obj_add_event_cb(login_username_ta, common_kb_event_cb, LV_EVENT_ALL, login_keyboard);

    login_password_ta = lv_textarea_create(panel);
    lv_textarea_set_one_line(login_password_ta, true);
    lv_textarea_set_password_mode(login_password_ta, true);
    lv_textarea_set_placeholder_text(login_password_ta, "请输入密码");
    lv_obj_set_width(login_password_ta, 280);
    lv_obj_align(login_password_ta, LV_ALIGN_TOP_MID, 0, 110);
    lv_obj_set_style_text_font(login_password_ta, get_chinese_font(), 0);
    lv_obj_add_event_cb(login_password_ta, common_kb_event_cb, LV_EVENT_ALL, login_keyboard);

    lv_obj_t * btn_main = lv_btn_create(panel);
    lv_obj_set_size(btn_main, 280, 45);
    lv_obj_align(btn_main, LV_ALIGN_TOP_MID, 0, 170);
    lv_obj_set_style_bg_color(btn_main, lv_color_hex(0x00D2FF), 0);
    lv_obj_add_event_cb(btn_main, login_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * label_main = lv_label_create(btn_main);
    lv_label_set_text(label_main, "立即登录");
    lv_obj_set_style_text_font(label_main, get_chinese_font(), 0);
    lv_obj_center(label_main);

    lv_obj_t * btn_sub = lv_btn_create(panel);
    lv_obj_set_size(btn_sub, 280, 40);
    lv_obj_align(btn_sub, LV_ALIGN_TOP_MID, 0, 225);
    lv_obj_set_style_bg_opa(btn_sub, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_sub, 0, 0);
    lv_obj_add_event_cb(btn_sub, to_register_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * label_sub = lv_label_create(btn_sub);
    lv_label_set_text(label_sub, "注册新账号");
    lv_obj_set_style_text_font(label_sub, get_chinese_font(), 0);
    lv_obj_set_style_text_color(label_sub, lv_color_hex(0x888888), 0);
    lv_obj_center(label_sub);

    lv_obj_move_foreground(login_keyboard);
}
