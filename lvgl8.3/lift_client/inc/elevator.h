#ifndef ELEVATOR_H
#define ELEVATOR_H
#include "lvgl/lvgl.h"

typedef enum { DIR_NONE, DIR_UP, DIR_DOWN } elevator_dir_t;

typedef struct
{
    int current_floor;
    elevator_dir_t dir;
    uint16_t stay_tick;  // 楼层等待时间
    uint16_t stay_fixed; // 等待倒计时
    bool is_door_open;
} elevator_data_t;

typedef struct list_floor
{
    struct list_floor * next;
    int floor;
} FloorNode, *FloorList;

int init_floor(void);                 // 初始化楼层，返回当前楼层号
int elevator_get_floor(void);         // 获取当前楼层
void elevator_add_request(int floor); // 按楼层按钮
void elevator_task_tick(void);        // 每 30ms 调一次，驱动电梯状态机
// 绑定 UI
void elevator_bind_ui(lv_obj_t * floor_label, lv_obj_t * up_arrow, lv_obj_t * down_arrow, lv_obj_t * door_label);
// 到达楼层回调（用于清除按钮高亮）
void elevator_on_floor_served(void (*cb)(int floor));
elevator_data_t get_elev(void);  // 获取电梯全局状态
void elevator_door_extend(void); // 开门键：延长开门时间
void elevator_door_close(void);  // 关门键,把等待时间清0

#endif
