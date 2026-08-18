#ifndef MEDIA_H
#define MEDIA_H

#include "lvgl/lvgl.h"

// Public functions
void create_music_screen(void);//创造音乐界面
void create_video_screen(void);//创造视频界面
void create_album_screen(void); // 创建相册界面（淡出/切换/淡入动画，支持缩放）
void stop_mplayer(void); // 停止 MPlayer 播放并刷新显示
void reset_media_state(void); // 重置媒体状态与指针

#endif
