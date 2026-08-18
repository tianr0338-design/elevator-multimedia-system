#include "arc.h"
void lv_ex_arc(void)
{
    lv_obj_t * obj = lv_obj_create(lv_scr_act()); // 在屏幕上创建一个画布
    lv_obj_set_size(obj, 800, 480);               // 屏幕大小
    lv_obj_center(obj);                           // 居中

    // lv_obj_t * arc = lv_arc_create(obj); // 在画布上创建一个弧
    // lv_obj_set_size(arc, 150, 150);
    // lv_arc_set_rotation(arc, 0);     //        //弧口的朝向
    // lv_arc_set_bg_angles(arc, 0, 180); // 背景弧也设为 0°~0°（隐藏背景）
    // lv_arc_set_range(arc, 0, 100);   // 设置值范围
    // lv_arc_set_value(arc, 0);        // 起始的值
    // lv_obj_center(arc);

    lv_obj_t * label = lv_label_create(obj);
    // 必须设置固定宽度（例如 100 像素）
    lv_obj_set_width(label, 100); // 设置标签的
    lv_obj_center(label);
    // char text[] = "hello world apple tree";
    // lv_label_set_text(label,text);
    // lv_label_set_text_fmt(label, "value:%s", "hello world");
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL); // 设置标签里面的内容进行滚动
    // lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
}

void obj(void)
{
    static lv_style_t style;
    lv_style_init(&style);

    lv_obj_t * obj = lv_obj_create(lv_scr_act()); // 一个容器
    lv_obj_center(obj);
    lv_style_set_bg_color(&style, lv_color_make(255, 0, 0));
    lv_style_set_radius(&style, 10);
    lv_obj_add_style(obj, &style, 0);
}

void fix(void)
{
    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 700, 320);
    lv_obj_center(obj);
    lv_obj_t * btn1 = lv_btn_create(obj);
    lv_obj_t * btn2 = lv_btn_create(obj);
    lv_obj_t * btn3 = lv_btn_create(obj);

    lv_obj_t * btn4 = lv_btn_create(obj);
    lv_obj_t * btn5 = lv_btn_create(obj);
    lv_obj_set_size(btn1, 200, 100);
    lv_obj_set_size(btn2, 200, 100);
    lv_obj_set_size(btn3, 200, 100);
    lv_obj_set_size(btn4, 200, 100);
    lv_obj_set_size(btn5, 200, 100);
    lv_obj_set_layout(obj, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP);
    // lv_obj_set_style_pad_all(obj,0,0);
    // lv_obj_set_style_pad_column(obj,0,0);
}

void lv_example_slider(void)
{
    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 800, 480);
    lv_obj_t * slider = lv_slider_create(obj);
    lv_obj_set_size(slider, 300, 30);
    lv_obj_center(slider);
}

void lv_example_switch(void)
{
    lv_obj_t * sw = lv_switch_create(lv_scr_act());
    lv_obj_center(sw);
}

void lv_example_textarea_simple(void)
{
    // 创建一个文本框
    lv_obj_t * textarea = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(textarea, 200, 100);
    lv_obj_set_align(textarea, LV_ALIGN_TOP_LEFT);
    lv_textarea_set_placeholder_text(textarea, "password...."); // 设置框里面的提示词
    lv_textarea_set_password_mode(textarea, true);              // 开启密码模式
    lv_textarea_set_one_line(textarea, true);                   // 开启单行模式

    // 創建一个键盘
    lv_obj_t * keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_height(keyboard, LV_PCT(50));           // 高度占屏幕50%（240px）
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0); // 对齐屏幕底部

    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN); // 初始化藏起来

    lv_keyboard_set_textarea(keyboard, textarea); // 关联键盘和文本框
    lv_obj_add_event_cb(textarea, textarea_click_event_cb, LV_EVENT_ALL, keyboard);
}

lv_example_event(void)
{
    lv_obj_t * obj = lv_obj_create(lv_scr_act()); // 在屏幕上创建一个画布
    lv_obj_set_size(obj, 800, 480);               // 屏幕大小

    lv_obj_t * btn = lv_btn_create(obj);
    lv_obj_set_size(btn, 300, 150);
    lv_obj_center(btn);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "0");
    lv_obj_center(label);

    // 绑定事件
    lv_obj_add_event_cb(btn, event_btn_cd, LV_EVENT_CLICKED, (void *)label);
}

void event_btn_cd(lv_event_t * e)
{
    static int count     = 0;
    lv_obj_t * label     = (lv_obj_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e); // 判断事件类型
    if(code == LV_EVENT_CLICKED) {
        count++;
        lv_label_set_text_fmt(label, "%d", count);
    }
}

void show_jpg(void)
{
    lv_obj_t * obj = lv_obj_create(lv_scr_act()); // 在屏幕上创建一个画布
    lv_obj_set_size(obj, 800, 480);               // 屏幕大小

    // lv_obj_t *img = lv_img_create(obj);
    // lv_img_set_src(img,"A:1.bmp");
    // lv_obj_align(img,LV_ALIGN_CENTER,0,0);
    lv_obj_t * gif = lv_gif_create(obj);
    lv_gif_set_src(gif, "A:2.gif");
    lv_obj_center(gif);
    // 关键：检查是否真的加载成功
    if(lv_img_get_src(gif) == NULL) {
        printf("❌ GIF 加载失败！文件未找到或格式不支持\n");
        // 可以创建一个红色方块作为占位符
        lv_obj_set_style_bg_color(gif, lv_color_make(255,0,0), 0);
        lv_obj_set_style_bg_opa(gif, LV_OPA_COVER, 0);
    } else {
        printf("✅ GIF 加载成功！\n");
    }
}

void show_chinese(void)
{
    // 1.先创建字体信息结构体
    static lv_ft_info_t info;
    info.name   = "./simfang.ttf";      // 字体的路径
    info.weight = 24;                   // 设置字号的大小
    info.style  = FT_FONT_STYLE_NORMAL; // 字体样式正常
    info.mem    = NULL;                 // 不使用内存字体

    // 2.初始化字体
    if(!(lv_ft_font_init(&info))) {
        LV_LOG_ERROR("creat failed");
    }

    // 设置字体样式
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, info.font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER); // 设置文本居中对齐

    /*创建应用新样式的标签*/
    lv_obj_t * label = lv_label_create(lv_scr_act()); // 在当前屏幕创建标签
    lv_obj_add_style(label, &style, 0);               // 为标签应用样式
    // lv_label_set_text(label, "你好\n我也不知道这是什么字体。 \n123456789！\n"); // 设置标签文本，包含中文、数字和符号
    // lv_obj_center(label);                                                       // 将标签居中显示
}

char admin[50]         = {0};
char password[50]      = {0};
char admin_os[]        = "admin";
char password_os[]     = "123456"; // ✅ 修正拼写
lv_obj_t * username_ta = NULL;
lv_obj_t * password_ta = NULL;
lv_obj_t * keyboard    = NULL;

void login(void)
{
    // 1.先创建字体信息结构体
    static lv_ft_info_t info;
    info.name   = "./simfang.ttf";      // 字体的路径
    info.weight = 24;                   // 设置字号的大小
    info.style  = FT_FONT_STYLE_NORMAL; // 字体样式正常
    info.mem    = NULL;                 // 不使用内存字体

    // 2.初始化字体
    if(!(lv_ft_font_init(&info))) {
        LV_LOG_ERROR("creat failed");
    }

    // 设置字体样式
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_text_font(&style, info.font);
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER); // 设置文本居中对齐

    lv_obj_t * obj1 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj1, 800, 480);
    lv_obj_clear_flag(obj1, LV_OBJ_FLAG_SCROLLABLE); // ✅ 禁用滚动

    lv_obj_t * img = lv_img_create(obj1); // ✅ 使用 obj1 而不是未定义的 obj
    lv_img_set_src(img, "A:bak.jpg");
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    // 创建用户名文本框
    username_ta = lv_textarea_create(img);
    lv_obj_set_size(username_ta, 200, 40);
    lv_obj_align(username_ta, LV_ALIGN_TOP_MID, 0, 50);
    lv_textarea_set_placeholder_text(username_ta, "用户名");
    lv_textarea_set_one_line(username_ta, true);
    lv_obj_add_style(username_ta, &style, LV_PART_MAIN);

    // 创建密码文本框
    password_ta = lv_textarea_create(img);
    lv_obj_set_size(password_ta, 200, 40); // ✅ 添加尺寸设置
    lv_obj_align(password_ta, LV_ALIGN_TOP_MID, 0, 100);
    lv_textarea_set_placeholder_text(password_ta, "密码");
    lv_textarea_set_one_line(password_ta, true);
    lv_textarea_set_password_mode(password_ta, true);
    lv_obj_add_style(password_ta, &style, LV_PART_MAIN);

    // 创建登录按钮
    lv_obj_t * login_btn = lv_btn_create(img);
    lv_obj_set_size(login_btn, 100, 40);
    lv_obj_align(login_btn, LV_ALIGN_TOP_MID, 0, 160);
    lv_obj_t * btn_label = lv_label_create(login_btn);
    lv_obj_add_style(btn_label, &style, 0);
    lv_label_set_text(btn_label, "登录");
    lv_obj_center(btn_label);
    lv_obj_add_event_cb(login_btn, login_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建键盘
    keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_height(keyboard, LV_PCT(50));
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);

    // 初始关联到用户名框（会在事件中动态切换）
    lv_keyboard_set_textarea(keyboard, username_ta);

    // 添加事件回调
    lv_obj_add_event_cb(username_ta, textarea_click_event_cb, LV_EVENT_FOCUSED, keyboard);
    lv_obj_add_event_cb(username_ta, textarea_click_event_cb, LV_EVENT_DEFOCUSED, keyboard);
    lv_obj_add_event_cb(password_ta, textarea_click_event_cb, LV_EVENT_FOCUSED, keyboard);
    lv_obj_add_event_cb(password_ta, textarea_click_event_cb, LV_EVENT_DEFOCUSED, keyboard);
}

static void textarea_click_event_cb(lv_event_t * e)
{
    lv_obj_t * ta        = lv_event_get_target(e);
    lv_obj_t * kb        = (lv_obj_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta); // ✅ 动态切换关联
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    } else if(code == LV_EVENT_DEFOCUSED) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

static void login_btn_event_cb(lv_event_t * e)
{
    // 获取输入的用户名和密码
    strcpy(admin, lv_textarea_get_text(username_ta));
    strcpy(password, lv_textarea_get_text(password_ta));

    // ✅ 修正逻辑：strcmp == 0 表示相等
    if(strcmp(admin, admin_os) == 0 && strcmp(password, password_os) == 0) {
        printf("登录成功！\n");
        // 创建新屏幕
        lv_obj_t * new_screen = lv_obj_create(NULL);
        lv_obj_set_size(new_screen, 800, 480);

        lv_obj_t * gif = lv_gif_create(new_screen);
        lv_gif_set_src(gif, "A:1.gif");
        lv_obj_center(gif);

        lv_scr_load(new_screen); // 切换到新屏幕
    } else {
        printf("用户名或密码错误！\n");
    }
}

void ta_event_cb1(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta        = lv_event_get_target(e);
    lv_obj_t * kb        = lv_event_get_user_data(e);

    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        }
    } else if(code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta); /*To forget the last clicked object to make it focusable again*/
    }
}

void test_pinyin(void)
{
    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 800, 480); // 画布原始大小很小，所以需要我们手动去延展

    lv_obj_t * pinyin_ime = lv_ime_pinyin_create(obj);
    /*创建字体信息结构体*/
    static lv_ft_info_t info;
    /*FreeType使用C标准文件系统，因此不需要驱动器号*/
    info.name   = "./simfang.ttf";      // 字体文件路径
    info.weight = 36;                   // 字体大小
    info.style  = FT_FONT_STYLE_NORMAL; // 字体样式：正常
    info.mem    = NULL;                 // 不使用内存字体

    /*初始化FreeType字体*/
    if(!lv_ft_font_init(&info)) {
        LV_LOG_ERROR("create failed."); // 初始化失败时记录错误
    }

    /*创建使用新字体的样式*/
    static lv_style_t style;
    lv_style_init(&style);                                 // 初始化样式
    lv_style_set_text_font(&style, info.font);             // 设置样式使用新字体
    lv_style_set_text_align(&style, LV_TEXT_ALIGN_CENTER); // 设置文本居中对齐

    /*创建应用新样式的标签*/
    lv_obj_add_style(pinyin_ime, &style, 0); // 为标签应用样式

    /* ta1 */
    lv_obj_t * ta1 = lv_textarea_create(obj);
    lv_textarea_set_one_line(ta1, true);
    lv_obj_add_style(ta1, &style, 0); // 为标签应用样式
    lv_obj_align(ta1, LV_ALIGN_TOP_LEFT, 0, 0);

    /*Create a keyboard and add it to ime_pinyin*/
    lv_obj_t * kb = lv_keyboard_create(obj);
    lv_ime_pinyin_set_keyboard(pinyin_ime, kb);
    lv_keyboard_set_textarea(kb, ta1);

    lv_obj_add_event_cb(ta1, ta_event_cb1, LV_EVENT_ALL, kb);

    /*Get the cand_panel, and adjust its size and position*/
    lv_obj_t * cand_panel = lv_ime_pinyin_get_cand_panel(pinyin_ime);
    lv_obj_set_size(cand_panel, LV_PCT(100), LV_PCT(10));
    lv_obj_align_to(cand_panel, kb, LV_ALIGN_OUT_TOP_MID, 0, 0);

    // /*Try using ime_pinyin to output the Chinese below in the ta1 above*/
    // lv_obj_t * cz_label = lv_label_create(obj);
    // lv_label_set_text(cz_label,
    //                   "嵌入式系统（Embedded
    //                   System），\n是一种嵌入机械或电气系统内部、具有专一功能和实时计算性能的计算机系统。");
    // lv_obj_add_style(cz_label, &style, 0);                                 // 为标签应用样式
    // lv_obj_set_width(cz_label, 310);
    // lv_obj_align_to(cz_label, ta1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
}