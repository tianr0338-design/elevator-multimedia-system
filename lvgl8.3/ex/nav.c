#include "nav.h"
#include "login.h"
#include "home.h"
#include "setting.h"

static lv_obj_t * screens[3] = {NULL};

void nav_switch_to(page_id_t id)
{
    if (screens[id] == NULL) {
        switch (id) {
            case PAGE_LOGIN:
                screens[id] = show_login();
                break;
            case PAGE_HOME:
                screens[id] = show_home();
                break;
            case PAGE_SETTING:
                screens[id] = show_setting();
                break;
        }
    }
    lv_scr_load(screens[id]);
}
