#include "ex01.h"

int count = 0;
static lv_style_t ui_btn_style;
void ui_style_btn_primary_init()
{
    lv_style_init(&ui_btn_style);
    lv_style_set_radius(&ui_btn_style, 10);
    lv_style_set_height(&ui_btn_style, 100);
    lv_style_set_width(&ui_btn_style, 200);
    lv_style_set_bg_opa(&ui_btn_style, LV_OPA_COVER);       // 透明度
    lv_style_set_bg_color(&ui_btn_style, lv_color_white()); // 背景颜色的设置
}
void btn_evnt_cb(lv_event_t * e)
{
    lv_obj_t * target    = lv_event_get_target(e);    // 获取触发事件的控件
    lv_event_code_t code = lv_event_get_code(e);      // 触发事件的类型，比如，点击，松开
    lv_obj_t * label     = lv_event_get_user_data(e); // 获取传过来的数据
    if(code == LV_EVENT_CLICKED) {
        count++;
        printf("%d\n", count);
    }
}
void ex()
{
    lv_obj_t * obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(obj, 800, 480);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000), LV_PART_MAIN);
    // lv_obj_t *img = lv_img_create(obj);
    // lv_obj_center(img);
    // lv_img_set_src(img,"A:1.png");               //添加图片
    lv_obj_t * gif = lv_gif_create(obj);
    lv_gif_set_src(gif, "A:kaiji.gif");
    // lv_obj_center(gif);

    lv_obj_t * btn1 = lv_btn_create(obj);
    ui_style_btn_primary_init();
    lv_obj_add_style(btn1, &ui_btn_style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(btn1);

    lv_obj_t * label = lv_label_create(btn1);

    // lv_label_set_text(label,"btn");
    lv_obj_center(label);

    lv_obj_add_event_cb(btn1, btn_evnt_cb, LV_EVENT_ALL, label); // 绑定事件
}

void set_font(void)
{
    static lv_ft_info_t info;           // 创建字体结构体
    info.name   = "./simfang.ttf";      // 字体的路径
    info.weight = 30;                   // 字体的大小
    info.style  = FT_FONT_STYLE_NORMAL; // 字体的样式----正常
    info.mem    = NULL;                 // 不使用内存字体
    if(!lv_ft_font_init(&info)) {
        printf("字体初始化失败！请检查 ttf 文件路径\n");
    } else {
        printf("字体初始化成功\n");
        printf("font = %p\n", info.font);
    }

    static lv_style_t ft_style;
    lv_style_init(&ft_style);
    lv_style_set_text_font(&ft_style, info.font); // 使用字体样式

    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_obj_set_style_bg_color(label, lv_color_hex(0x00FF00), 0); // 绿色背景
    lv_obj_set_style_bg_opa(label, LV_OPA_COVER, 0);             // 不透明
    lv_obj_add_style(label, &ft_style, 0);
    lv_label_set_text(label, "上課JFK啥的JFK盛大積分");
    lv_obj_center(label);
}

void ta_event_cd(lv_event_t * e)
{
    lv_event_t * target  = lv_event_get_target(e);    // 获取触发事件的对象
    lv_event_code_t code = lv_event_get_code(e);      // 获取触发事件的类型
    lv_obj_t * kb        = lv_event_get_user_data(e); // 获取事件传递过来的参数
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, target);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN); // 让键盘显示出来
    }
    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL); // 取消关联
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_READY)
    {
        printf("确定\n");
        char *p = lv_textarea_get_text(target);
        puts(p);
    }
}

void test_keyboard(void)
{
    lv_obj_t * text = lv_textarea_create(lv_scr_act()); // 在屏幕上创建一个文本框
    lv_obj_clear_flag(text, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(text, 200, 40);
    lv_obj_center(text);
    lv_obj_t * keybodard = lv_keyboard_create(lv_scr_act()); // 创建一个键盘
    lv_obj_add_flag(keybodard, LV_OBJ_FLAG_HIDDEN);          // 先隐藏
    lv_obj_add_event_cb(text, ta_event_cd, LV_EVENT_ALL, keybodard);
}
