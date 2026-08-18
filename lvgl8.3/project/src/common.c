#include "common.h"

// 全局变量定义
lv_obj_t * g_content_area = NULL;
lv_obj_t * g_sidebar_btns[6] = {0};
int current_page_idx = 0;

static lv_font_t * g_chinese_font = NULL;

// 声明内置字体
// LV_FONT_DECLARE(lv_font_simsun_16_cjk);

// 获取中文字体
lv_font_t * get_chinese_font(void)
{
    // 使用 FreeType 加载外部字体
    if(g_chinese_font == NULL) {
        static lv_ft_info_t info;
        // 尝试加载中文字体，优先尝试 SourceHanSansSC-VF.ttf，如果没有则尝试 simkai.ttf
        if(access("./SourceHanSansSC-VF.ttf", F_OK) == 0) {
             info.name = "./SourceHanSansSC-VF.ttf";
        } else {
             // 备用字体路径，用户可能使用了其他名称
             info.name = "./simkai.ttf"; 
        }
        
        // 如果用户有特定路径，请在此处修改
        // info.name = "/path/to/your/font.ttf";

        info.weight = 20;
        info.style  = FT_FONT_STYLE_NORMAL;
        info.mem    = NULL;

        if(!lv_ft_font_init(&info)) {
            LV_LOG_ERROR("Chinese font initialization failed");
            // 如果加载失败，返回默认字体作为后备，避免崩溃
            return LV_FONT_DEFAULT;
        }

        g_chinese_font = info.font;
    }
    return g_chinese_font;
}

// 通用键盘事件回调
void common_kb_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta        = lv_event_get_target(e);
    lv_obj_t * kb        = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(kb);
    } else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}


// 创建侧边栏按钮
lv_obj_t * create_sidebar_btn(lv_obj_t * parent, const char * img_path, const char * text)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_set_size(obj, 100, 100);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_set_layout(obj, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(obj, 5, 0);

    lv_obj_t * img = lv_img_create(obj);
    lv_img_set_src(img, img_path);

    lv_obj_t * label = lv_label_create(obj);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, get_chinese_font(), LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xE0E0E0), LV_PART_MAIN);

    return obj;
}

// 显示 Toast 提示信息
void show_toast(const char * text, lv_color_t color)
{
    lv_obj_t * toast = lv_label_create(lv_scr_act());
    lv_label_set_text(toast, text);
    lv_obj_set_style_text_font(toast, get_chinese_font(), 0);
    lv_obj_set_style_text_color(toast, color, 0);

    lv_obj_set_style_bg_color(toast, lv_color_hex(0x222222), 0);
    lv_obj_set_style_bg_opa(toast, LV_OPA_90, 0);
    lv_obj_set_style_pad_all(toast, 10, 0);
    lv_obj_set_style_radius(toast, 5, 0);

    lv_obj_center(toast);
    lv_obj_del_delayed(toast, 2000);
}
