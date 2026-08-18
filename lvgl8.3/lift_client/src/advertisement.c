#include "advertisement.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "cJSON.h"

#define SRV_PORT 8888
#define SRV_IP "192.168.10.169"
#define AD_MAX 10

/* 广告数据 */
static char g_titles[AD_MAX][128];
static char g_files[AD_MAX][64];
static int g_count = 0;
static int g_cur   = 0;

// UI 控件
static lv_obj_t * g_card    = NULL;
static lv_obj_t * g_label   = NULL;
static lv_obj_t * g_img     = NULL;
static lv_timer_t * g_timer = NULL;
static lv_timer_t * g_push  = NULL;
static int g_sockfd         = -1;
static char g_dev_id[64];
static int g_group = 1;

/* 天气信息（独立于广告服务器，直连外网 API） */
char g_weather[128]               = "";
char g_wtime[128]                 = "";
static lv_obj_t * g_weather_label = NULL;

void ad_bind_weather_label(lv_obj_t * label)
{
    g_weather_label = label;
}

void ad_bind_ui(lv_obj_t * card, lv_obj_t * title_label, lv_obj_t * img)
{
    g_card  = card;
    g_label = title_label;
    g_img   = img;
}

// 轮播回调
void cycle_cb(lv_timer_t * t)
{
    if(g_count == 0) {
        return;
    }

    g_cur = (g_cur + 1) % g_count;
    lv_label_set_text(g_label, g_titles[g_cur]);
    lv_img_set_src(g_img, g_files[g_cur]);
}

void ad_start_cycle(void)
{
    if(g_count == 0 || !g_label || !g_img) {
        return;
    }
    lv_label_set_text(g_label, g_titles[0]);
    lv_img_set_src(g_img, g_files[0]);
    g_timer = lv_timer_create(cycle_cb, 3000, NULL);
}

// 检查服务器推送
static void push_check_cb(lv_timer_t * t)
{
    if(g_sockfd == -1) {
        return;
    }

    char tmp[64];
    int n = recv(g_sockfd, tmp, sizeof(tmp), MSG_DONTWAIT);
    if(n <= 0) {
        return;
    }

    printf("[AD] 收到推送, 重连 (ID=%s group=%d)\n", g_dev_id, g_group);
    close(g_sockfd);
    g_sockfd = -1;
    if(ad_fetch(g_dev_id, g_group) >= 0) {
        if(g_timer) {
            lv_timer_del(g_timer);
            g_timer = NULL;
        }
        ad_start_cycle();
    }
}

// 直连 k780 API 获取天气（独立于广告服务器，不影响广告功能）

void ad_fetch_weather(void)
{
    struct hostent * ht = gethostbyname("api.k780.com");
    if(!ht) {
        printf("[WEA] DNS 失败\n");
        return;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) return;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(80);
    addr.sin_addr   = *(struct in_addr *)ht->h_addr_list[0];

    if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(fd);
        return;
    }

    /* ① 查归属地 */
    char req[8192], buf[4096];
    snprintf(req, sizeof(req),
             "GET /?app=ip.get&appkey=79457"
             "&sign=aa4b1cec794ebdbecd8f484af22014f0"
             "&format=json HTTP/1.1\r\n"
             "Host: api.k780.com\r\nConnection: close\r\n\r\n");
    send(fd, req, strlen(req), 0);

    int n = recv(fd, buf, sizeof(buf) - 1, 0);
    if(n <= 0) {
        close(fd);
        return;
    }
    buf[n] = '\0';

    char * start = strchr(buf, '{');
    char * end   = strrchr(buf, '}');
    if(!start || !end) {
        close(fd);
        return;
    }
    *(end + 1) = '\0';

    cJSON * root = cJSON_Parse(start);
    if(!root) {
        close(fd);
        return;
    }
    cJSON * result = cJSON_GetObjectItem(root, "result");
    cJSON * att    = result ? cJSON_GetObjectItem(result, "att") : NULL;
    if(!att || !att->valuestring) {
        cJSON_Delete(root);
        close(fd);
        return;
    }

    char att_str[128];
    strcpy(att_str, att->valuestring);
    strtok(att_str, ",");
    strtok(NULL, ",");
    char * city = strtok(NULL, ",");
    if(!city) {
        cJSON_Delete(root);
        close(fd);
        return;
    }
    cJSON_Delete(root);

    /* ② 查天气 */
    close(fd);
    fd = socket(AF_INET, SOCK_STREAM, 0);
    ht = gethostbyname("api.k780.com");
    if(!ht || fd == -1) return;
    addr.sin_addr = *(struct in_addr *)ht->h_addr_list[0];

    if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        close(fd);
        return;
    }

    snprintf(req, sizeof(req),
             "GET /?app=weather.today&cityNm=%s&appkey=79457"
             "&sign=aa4b1cec794ebdbecd8f484af22014f0&type=json"
             " HTTP/1.1\r\n"
             "Host: api.k780.com\r\nConnection: close\r\n\r\n",
             city);
    send(fd, req, strlen(req), 0);

    memset(buf, 0, sizeof(buf));
    n = recv(fd, buf, sizeof(buf) - 1, 0);
    if(n <= 0) {
        close(fd);
        return;
    }
    buf[n] = '\0';

    start = strchr(buf, '{');
    end   = strrchr(buf, '}');
    if(!start || !end) {
        close(fd);
        return;
    }
    *(end + 1) = '\0';

    root = cJSON_Parse(start);
    if(!root) {
        close(fd);
        return;
    }
    result = cJSON_GetObjectItem(root, "result");
    if(!result) {
        cJSON_Delete(root);
        close(fd);
        return;
    }

    cJSON * date    = cJSON_GetObjectItem(result, "days");
    cJSON * week    = cJSON_GetObjectItem(result, "week");
    cJSON * temp    = cJSON_GetObjectItem(result, "temperature_curr");
    cJSON * weather = cJSON_GetObjectItem(result, "weather_curr");

    if(date && week && temp && weather) {
        snprintf(g_weather, sizeof(g_weather), "%s %s %s", weather->valuestring, temp->valuestring, city);
        snprintf(g_wtime, sizeof(g_wtime), "%s %s", date->valuestring, week->valuestring);
        printf("[WEA] %s | %s\n", g_weather, g_wtime);
        /* 刷新界面 */
        if(g_weather_label) {
            char info[256];
            snprintf(info, sizeof(info), "%s | %s", g_weather, g_wtime);
            lv_label_set_text(g_weather_label, info);
        }
    }
    cJSON_Delete(root);
    close(fd);
}

// 发送警报给服务器
void ad_send_alarm(void)
{
    if(g_sockfd == -1) {
        return;
    }
    send(g_sockfd, "===ALARM===\n", 12, 0);
    printf("[AD] 已发送警报\n");
}

// 连接服务器 + 下载广告

// 流式状态机
enum { ST_WAIT, ST_TITLE, ST_IMG_SIZE, ST_IMG_DATA };

int ad_fetch(const char * dev_id, int group)
{
    // 存下 ID 和分组，避免自己拷自己
    if(dev_id != g_dev_id) {
        snprintf(g_dev_id, sizeof(g_dev_id), "%s", dev_id);
    }
    g_group = group;
    printf("[AD] 开始连接...\n");
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        printf("[AD] socket failed\n");
        return -1;
    }
    // 开始连接服务器
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(SRV_PORT);
    addr.sin_addr.s_addr = inet_addr(SRV_IP);
    if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        printf("[AD] connect failed\n");
        close(fd);
        return -1; // 失败跑路
    }

    char hand[128];
    snprintf(hand, sizeof(hand), "===ID %s GROUP %d===\n", dev_id, group);
    printf("[AD] 握手: %s", hand);
    send(fd, hand, strlen(hand), 0);

    // 流式状态机
    char buf[4096];           // 接受图片数据缓冲区
    int buf_len    = 0;       // 接受数据的长度
    int state      = ST_WAIT; // 当去状态空闲
    int img_remain = 0;       // 还需读多少字节图片数据
    FILE * img_fp  = NULL;

    g_count = 0; // 当前读取的广告数量

    while(g_count < AD_MAX) {
        int n = recv(fd, buf + buf_len, sizeof(buf) - buf_len, 0); // 开始接受
        if(n <= 0) {
            break;
        }
        buf_len += n; // 更新当前长度

        char * p = buf;
        while(p < buf + buf_len && g_count < AD_MAX) {
            if(state == ST_IMG_DATA) {
                int consume;
                if(p + img_remain > buf + buf_len) {
                    consume = buf + buf_len - p;
                } else {
                    consume = img_remain;
                }
                if(img_fp) {
                    fwrite(p, 1, consume, img_fp);
                }
                p += consume;
                img_remain -= consume;
                if(img_remain == 0) {
                    if(img_fp) {
                        fclose(img_fp);
                        img_fp = NULL;
                    }
                    state = ST_WAIT;
                    printf("[AD] 图片%d 保存完成\n", g_count);
                    g_count++;
                }
                continue;
            }

            // 扫 marker
            if(strncmp(p, "===END===", 9) == 0) {
                goto done;
            }
            if(strncmp(p, "===TITLE ", 9) == 0) {
                p += 9;
                char * e = strstr(p, "===\n");
                if(!e) {
                    break;
                } // 数据不全，等下一包
                int len = e - p;
                if(len > 127) {
                    len = 127;
                }
                memcpy(g_titles[g_count], p, len);
                g_titles[g_count][len] = '\0';
                printf("[AD] 标题: %s\n", g_titles[g_count]);
                p     = e + 4;
                state = ST_WAIT;
                continue;
            }
            if(strncmp(p, "===IMG ", 7) == 0) {
                int size = 0;
                sscanf(p, "===IMG %d===", &size);
                char * e = strstr(p + 7, "===");
                if(!e) {
                    break;
                }
                p          = e + 3;
                img_remain = size;
                // 直接开文件写
                char rp[128];
                snprintf(rp, sizeof(rp), "/lift_cilent/ad_%d.png", g_count);
                snprintf(g_files[g_count], sizeof(g_files[0]), "A:ad_%d.png", g_count);
                img_fp = fopen(rp, "wb");
                printf("[AD] 开始收图片, %d 字节\n", size);
                state = ST_IMG_DATA;
                continue;
            }
            p++;
        }
        // 把未处理的字节移到 buffer 开头
        int left = buf + buf_len - p;
        if(left > 0 && p != buf) {
            memmove(buf, p, left);
        }
        buf_len = left;
    }

done:
    if(img_fp) {
        fclose(img_fp);
        img_fp = NULL;
    }
    printf("[AD] 下载完成, 共 %d 条\n", g_count);
    g_sockfd = fd;

    // 排空 TCP 残留碎片（TCP 分包可能导致 \n 遗留）
    {
        char drain[64];
        while(recv(fd, drain, sizeof(drain), MSG_DONTWAIT) > 0) {
        }
    }

    // 启动推送监听，每 2 秒检查一次
    if(g_sockfd != -1 && !g_push) {
        g_push = lv_timer_create(push_check_cb, 2000, NULL);
    }

    if(g_count > 0 && fd != -1) {
        return fd;
    } else {
        return -1;
    }
}
