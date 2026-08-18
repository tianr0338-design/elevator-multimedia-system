#include "elevator.h"
#include <stdlib.h>
#include <stdio.h>

#define FLOOR_MIN 1
#define FLOOR_MAX 12
#define MOVE_MS 3000 // 每层移动 3 秒

elevator_data_t g_elev; // 电梯的全局状态
static FloorList g_queue            = NULL;
static uint16_t g_move_tick         = MOVE_MS;
static void (*on_floor_served)(int) = NULL; // 到达楼层回调

// 等待刷新的UI
static lv_obj_t * ui_floor = NULL; // 当前楼层
static lv_obj_t * ui_up    = NULL; // 上行箭头
static lv_obj_t * ui_down  = NULL; // 下行箭头
static lv_obj_t * ui_door  = NULL; // 门状态文字

// UI 刷新
static void floor_ui_refresh(void)
{
    if(!ui_floor) {
        return;
    }

    char buf[8];
    snprintf(buf, sizeof(buf), "%d", g_elev.current_floor);
    lv_label_set_text(ui_floor, buf);

    // 箭头：移动时显示方向，空闲时隐藏，开门时隐藏
    if(g_elev.is_door_open || g_elev.dir == DIR_NONE) {
        lv_obj_add_flag(ui_up, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_down, LV_OBJ_FLAG_HIDDEN);
    } else if(g_elev.dir == DIR_UP) {
        lv_obj_clear_flag(ui_up, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ui_down, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(ui_up, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(ui_down, LV_OBJ_FLAG_HIDDEN);
    }

    // 门状态文字
    if(ui_door) {
        if(g_elev.is_door_open) {
            lv_label_set_text(ui_door, "开门中");
        } else if(g_elev.dir != DIR_NONE) {
            lv_label_set_text(ui_door, "运行中");
        } else {
            lv_label_set_text(ui_door, "停靠中");
        }
    }
}

void elevator_bind_ui(lv_obj_t * floor_label, lv_obj_t * up_arrow, lv_obj_t * down_arrow, lv_obj_t * door_label)
{
    ui_floor = floor_label;
    ui_up    = up_arrow;
    ui_down  = down_arrow;
    ui_door  = door_label;
    // 默认状态
    lv_obj_add_flag(ui_up, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_down, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(ui_door, "停靠中");
}


// 找当前方向最近的目标楼层，没有则返回
static int find_target(elevator_dir_t dir)
{
    FloorNode * p;
    int target;

    if(dir == DIR_UP) {
        target = FLOOR_MAX + 1;
        for(p = g_queue; p; p = p->next) {
            if(p->floor > g_elev.current_floor && p->floor < target) { // 比当前楼层打，而且是最靠近当前的楼层的
                target = p->floor;
            }
        }
        if(target == FLOOR_MAX + 1) {
            return -1;
        } else {
            return target;
        }
    }

    if(dir == DIR_DOWN) {
        target = FLOOR_MIN - 1;
        for(p = g_queue; p; p = p->next) {
            if(p->floor < g_elev.current_floor && p->floor > target) { // 比当前楼层小，而且是最靠近当前的楼层的
                target = p->floor;
            }
        }
        if(target == FLOOR_MIN - 1) {
            return -1;
        } else {
            return target;
        }
    }

    return -1;
}

// 统计某方向还有多少请求
static int count_dir(elevator_dir_t d)
{
    int n = 0;
    FloorNode * p;
    for(p = g_queue; p; p = p->next) {
        if(d == DIR_UP && p->floor > g_elev.current_floor) { // 上行
            n++;
        }
        if(d == DIR_DOWN && p->floor < g_elev.current_floor) { // 下行
            n++;
        }
    }
    return n;
}

// 从队列中删除某个楼层
static void remove_floor(int floor)
{
    FloorNode *prev = NULL, *p = g_queue;
    while(p) {
        if(p->floor == floor) {
            if(prev) {
                prev->next = p->next;
            } else {
                g_queue = p->next;
            }
            free(p);
            // 通知 UI：该楼层已到达
            if(on_floor_served) {
                on_floor_served(floor);
            }
            return;
        }
        prev = p;
        p    = p->next;
    }
}

void elevator_on_floor_served(void (*cb)(int floor))
{
    on_floor_served = cb;
}

elevator_data_t get_elev(void)
{
    return g_elev;
}

void elevator_door_extend(void)
{
    if(g_elev.is_door_open) {
        g_elev.stay_tick = g_elev.stay_fixed;  // 重置倒计时，延长开门
    }
}

void elevator_door_close(void)
{
    if(g_elev.is_door_open) {
        g_elev.is_door_open = false;
        g_elev.stay_tick    = 0;
        floor_ui_refresh();
    }
}

// 对运行状态进行初始化
int init_floor(void)
{
    int cur              = (rand() % (FLOOR_MAX - FLOOR_MIN + 1)) + FLOOR_MIN;
    g_elev.current_floor = cur;
    g_elev.dir           = DIR_NONE;
    g_elev.stay_fixed    = 3000;
    g_elev.stay_tick     = 0;
    g_elev.is_door_open  = false;
    return cur;
}

int elevator_get_floor(void)
{
    return g_elev.current_floor;
}

// 楼层按钮被按下
void elevator_add_request(int floor)
{
    if(floor < FLOOR_MIN || floor > FLOOR_MAX) {
        return;
    }
    if(floor == g_elev.current_floor) {
        return;
    } // 就在这层，不开门

    FloorNode * p;
    for(p = g_queue; p; p = p->next) // 去重
    {
        if(p->floor == floor) {
            return;
        }
    }

    FloorNode * n = malloc(sizeof(FloorNode)); // 进入新楼层
    n->floor      = floor;
    n->next       = g_queue;
    g_queue       = n;
    printf("[ELEV] 请求: %d 楼  待处理: %d\n", floor, count_dir(DIR_UP) + count_dir(DIR_DOWN));
}

// 状态机核心：每 30ms 调用一次
void elevator_task_tick(void)
{
    // 状态 ①：开门中，倒计时关门
    if(g_elev.is_door_open) {
        if(g_elev.stay_tick > 0) {
            g_elev.stay_tick -= 30;
            if(g_elev.stay_tick <= 0) {
                g_elev.stay_tick    = 0;
                g_elev.is_door_open = false;
                floor_ui_refresh();
                printf("[ELEV] 关门\n");
            }
        }
        return; // 门开着，不移动
    }

    // 状态 ②：队列空 状态机停止
    if(g_queue == NULL) {
        g_elev.dir = DIR_NONE;
        return;
    }

    // 状态 ③：空闲时决定方向
    if(g_elev.dir == DIR_NONE) {
        // 只有单向请求
        int up_n   = count_dir(DIR_UP);
        int down_n = count_dir(DIR_DOWN);

        if(up_n == 0 && down_n == 0) {
            return;
        } // 队列异常（全是当前楼层）
        if(up_n > 0 && down_n == 0) {
            g_elev.dir = DIR_UP;
        } else if(down_n > 0 && up_n == 0) {
            g_elev.dir = DIR_DOWN;
        } else {
            // 两边都有请求，分别找最近目标，选距离近的方向
            int up_target   = find_target(DIR_UP);
            int down_target = find_target(DIR_DOWN);
            if(up_target == -1) {
                g_elev.dir = DIR_DOWN;
            } else if(down_target == -1) {
                g_elev.dir = DIR_UP;
            } else {
                int up_dist   = up_target - g_elev.current_floor;
                int down_dist = g_elev.current_floor - down_target;
                if(up_dist <= down_dist) {
                    g_elev.dir = DIR_UP;
                } else {
                    g_elev.dir = DIR_DOWN;
                }
            }
        }

        if(g_elev.dir == DIR_UP) {
            printf("[ELEV] 方向: 上行\n");
        } else if(g_elev.dir == DIR_DOWN) {
            printf("[ELEV] 方向: 下行\n");
        } else {
            printf("[ELEV] 方向: 空闲\n");
        }
        g_move_tick = MOVE_MS; // 新方向启动，重置冷却
        floor_ui_refresh();
    }

    // 状态 ④：到目标楼层了吗？（先于冷却，一到达立即开门）
    {
        FloorNode * p;
        for(p = g_queue; p; p = p->next) {
            if(p->floor == g_elev.current_floor) {
                printf("[ELEV] 到达 %d 楼，开门\n", g_elev.current_floor);
                g_elev.is_door_open = true;
                g_elev.stay_tick    = g_elev.stay_fixed; // 开门时间
                remove_floor(g_elev.current_floor);
                g_move_tick = MOVE_MS; // 下次移动前也要冷却
                floor_ui_refresh();

                if(count_dir(g_elev.dir) == 0) {
                    elevator_dir_t opp; // 进行折反
                    if(g_elev.dir == DIR_UP) {
                        opp = DIR_DOWN;
                    } else {
                        opp = DIR_UP;
                    }
                    if(count_dir(opp) > 0) {
                        g_elev.dir = opp;
                    } else {
                        g_elev.dir = DIR_NONE;
                    }
                }
                return;
            }
        }
    }

    // 状态 ⑤：移动倒计时，没到时间不换层(减速器)
    if(g_move_tick > 0) {
        g_move_tick -= 30;
        return;
    }

    // 状态 ⑥：没有到达,继续移动
    if(g_elev.dir == DIR_UP) {
        g_elev.current_floor++;
    } else if(g_elev.dir == DIR_DOWN) {
        g_elev.current_floor--;
    }
    g_move_tick = MOVE_MS;
    floor_ui_refresh();
    printf("[ELEV] → %d 楼\n", g_elev.current_floor);
}
