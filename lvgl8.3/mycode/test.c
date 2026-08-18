#include "mycode/test.h"

void test(void)
{
    lv_obj_t * obj = lv_obj_create(lv_scr_act()); // 在屏幕上铺一张画布，lv_scr_act()获取屏幕
    lv_obj_set_size(obj, 800, 480);               // 默认的画布大小很小，手动设置画布的大小
    lv_obj_set_align(obj, LV_ALIGN_CENTER);       // 让画布居中显示

    lv_obj_t * but1 = lv_btn_create(obj);       // 在画布上添加一个按钮
    lv_obj_set_size(but1, 200, 100);            // 设置按钮的大小
    lv_obj_set_align(but1, LV_ALIGN_RIGHT_MID); // 让按钮在画布的中间显示

    lv_obj_t * label = lv_label_create(but1); // 在按钮上设置一个标签
    // lv_obj_set_size(label, 80, 40);           // 设置标签的大小
    lv_label_set_text(label, "butten"); // 给标签添加，文本
                                        // lv_obj_set_align(label, LV_ALIGN_CENTER); // 让标签剧中
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_center(label); // 标签居中
    set_but_style1(but1); // 设置按钮样式

    lv_obj_t * btn2 = lv_btn_create(obj);      // 在添加一个新的按钮
    lv_obj_set_size(btn2, 200, 100);           // 设置大小
    lv_obj_set_align(btn2, LV_ALIGN_LEFT_MID); // 设置显示的位置
    lv_obj_t * label2 = lv_label_create(btn2); // 给按钮2也添加一个标签
    lv_label_set_text(label2, "butten");       // 给标签添加，文本
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_24, 0);
    lv_obj_center(label2);
    set_but_style2(btn2); // 设置按钮样式
}

static void set_but_style1(lv_obj_t * btn)
{
    static lv_style_t style_main;
    static lv_style_t style_pressed;
    static bool is_init = false;

    if(!is_init) {
        // --- 默认状态样式 ---
        lv_style_init(&style_main);

        // 核心背景：从优雅的海洋蓝到午夜蓝的垂直渐变
        lv_style_set_bg_color(&style_main, lv_color_hex(0x3498DB));      // 顶部色：明亮蓝
        lv_style_set_bg_grad_color(&style_main, lv_color_hex(0x2980B9)); // 底部色：深邃蓝
        lv_style_set_bg_grad_dir(&style_main, LV_GRAD_DIR_VER);
        lv_style_set_bg_opa(&style_main, 255);

        // 形状与边框
        lv_style_set_radius(&style_main, 12);
        lv_style_set_border_width(&style_main, 1);
        lv_style_set_border_color(&style_main, lv_color_hex(0x5DADE2)); // 浅蓝高光边框
        lv_style_set_border_opa(&style_main, 150);

        // 阴影：深色通透阴影，增加悬浮感
        lv_style_set_shadow_color(&style_main, lv_color_hex(0x1B4F72));
        lv_style_set_shadow_width(&style_main, 20);
        lv_style_set_shadow_opa(&style_main, 100);
        lv_style_set_shadow_ofs_y(&style_main, 8);

        // 文本：白色，并带有一点点字间距
        lv_style_set_text_color(&style_main, lv_color_white());
        lv_style_set_text_letter_space(&style_main, 1);

        // --- 按下状态样式 ---
        lv_style_init(&style_pressed);
        // 按下瞬间变色：深蓝色
        lv_style_set_bg_color(&style_pressed, lv_color_hex(0x21618C));
        // 点击下沉动画
        lv_style_set_translate_y(&style_pressed, 4);
        lv_style_set_shadow_width(&style_pressed, 5);
        lv_style_set_shadow_ofs_y(&style_pressed, 2);

        is_init = true;
    }

    lv_obj_add_style(btn, &style_main, LV_STATE_DEFAULT);
    lv_obj_add_style(btn, &style_pressed, LV_STATE_PRESSED);
}

static void set_but_style2(lv_obj_t * btn)
{
    {
        static lv_style_t style_main;
        static lv_style_t style_pressed;
        static bool is_init = false;

        if(!is_init) {
            // --- 默认状态：低调的深碳素黑 ---
            lv_style_init(&style_main);

            // 背景：使用非常深的灰色渐变，模拟磨砂金属质感
            lv_style_set_bg_color(&style_main, lv_color_hex(0x2C3E50));
            lv_style_set_bg_grad_color(&style_main, lv_color_hex(0x000000));
            lv_style_set_bg_grad_dir(&style_main, LV_GRAD_DIR_VER);
            lv_style_set_bg_opa(&style_main, 255);

            // 圆角：稍微方正一点，显得硬朗
            lv_style_set_radius(&style_main, 8);

            // 边框：这是灵魂！用极细的深蓝灰边框勾勒轮廓
            lv_style_set_border_width(&style_main, 1);
            lv_style_set_border_color(&style_main, lv_color_hex(0x34495E));

            // 阴影：使用纯黑阴影，且范围收紧，让按钮显得更重、更扎实
            lv_style_set_shadow_color(&style_main, lv_color_hex(0x000000));
            lv_style_set_shadow_width(&style_main, 15);
            lv_style_set_shadow_opa(&style_main, 150);
            lv_style_set_shadow_ofs_y(&style_main, 5);

            // 文本：淡淡的灰白色，不刺眼
            lv_style_set_text_color(&style_main, lv_color_hex(0xBDC3C7));

            // --- 按下状态：瞬间唤醒的霓虹绿 ---
            lv_style_init(&style_pressed);
            // 背景切换到深绿
            lv_style_set_bg_color(&style_pressed, lv_color_hex(0x27AE60));
            lv_style_set_bg_grad_color(&style_pressed, lv_color_hex(0x1E8449));
            // 字体变白变亮
            lv_style_set_text_color(&style_pressed, lv_color_white());
            // 增加发光效果 (通过改变阴影颜色实现)
            lv_style_set_shadow_color(&style_pressed, lv_color_hex(0x2ECC71));
            lv_style_set_shadow_width(&style_pressed, 20);
            // 轻微缩放效果
            lv_style_set_transform_zoom(&style_pressed, 250); // 略微缩小一点点 (256是原大)

            is_init = true;
        }

        lv_obj_add_style(btn, &style_main, LV_STATE_DEFAULT);
        lv_obj_add_style(btn, &style_pressed, LV_STATE_PRESSED);
    }
}