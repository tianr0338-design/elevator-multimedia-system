#ifndef NAV_H
#define NAV_H
#include "lvgl/lvgl.h"

typedef enum {
    PAGE_LOGIN,
    PAGE_HOME,
    PAGE_SETTING,
} page_id_t;

void nav_switch_to(page_id_t id);

#endif
