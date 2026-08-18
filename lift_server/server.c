#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "cJSON.h"
#define PORT "8888"
#define AD_DIR "ads"
#define MAX_CLI 1024

// 设备信息
typedef struct
{
    int  fd;
    char id[64];
    int  group;
    char ip[32];
} device_t;

device_t g_devs[MAX_CLI];   // 在线设备表

void dev_add(int fd, const char *ip)
{
    for (int i = 0; i < MAX_CLI; i++)
        if (g_devs[i].fd == -1)
        {
            g_devs[i].fd = fd;
            snprintf(g_devs[i].ip, sizeof(g_devs[i].ip), "%s", ip);
            return;
        }
}

void dev_del(int fd) // 断开连接就删除
{
    for (int i = 0; i < MAX_CLI; i++)
        if (g_devs[i].fd == fd)
        {
            memset(&g_devs[i], 0, sizeof(device_t));
            g_devs[i].fd = -1;
            return;
        }
}

device_t *dev_find(int fd) // 寻找设备
{
    for (int i = 0; i < MAX_CLI; i++)
        if (g_devs[i].fd == fd)
        {
            return &g_devs[i];
        }
    return NULL;
}

void dev_list(void) // 监控在线设备
{
    printf("\n===== 在线设备 =====\n");
    int n = 0;
    for (int i = 0; i < MAX_CLI; i++)
    {
        if (g_devs[i].fd != -1)
        {
            printf("  [%d] fd=%d  ID=%s  分组=%d  IP=%s\n",
                   n++, g_devs[i].fd, g_devs[i].id,
                   g_devs[i].group, g_devs[i].ip);
        }
    }
    printf("共 %d 台在线\n\n", n);
}

// 给客服端发送信息
void send_str(int fd, const char *s) { send(fd, s, strlen(s), 0); }

void send_ads(int fd, int group)
{
    char list_path[256];
    snprintf(list_path, sizeof(list_path), AD_DIR "/group_%d/ads.txt", group); // 拼接路径

    FILE *fp = fopen(list_path, "r");
    if (!fp)
    {
        send_str(fd, "===END===\n");
        printf("[SRV] 分组 %d 没有广告\n", group); // 没有这个组,不发送
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) // 逐行读取文件内容
    {
        line[strcspn(line, "\r\n")] = '\0'; //\r\ns是windows系统的换行标志
        if (line[0] == '\0')                // 没有读到就跳过
        {
            continue;
        }

        char *title = strtok(line, "|"); // 得到标题
        char *file = strtok(NULL, "|");  // 得到图片名字
        if (!title || !file)
        {
            continue; // 只要其中一个没有读取到就跑路
        }

        char img_path[256];
        snprintf(img_path, sizeof(img_path), AD_DIR "/group_%d/%s", group, file); // 拼接路径

        struct stat st;
        if (stat(img_path, &st) != 0) // 获取图片信息
        {
            continue;
        }
        FILE *ifp = fopen(img_path, "rb"); // 以二进制的方式进行读取
        if (!ifp)
        {
            continue;
        }
        char *img = malloc(st.st_size); // 按照图片信息的大小进行分配内存
        fread(img, 1, st.st_size, ifp); // 读取完成
        fclose(ifp);

        char head[64];
        snprintf(head, sizeof(head), "===TITLE %s===\n", title);
        send_str(fd, head); // 发送标题

        snprintf(head, sizeof(head), "===IMG %ld===", (long)st.st_size);
        send_str(fd, head);           // 发送图片名
        send(fd, img, st.st_size, 0); // 发送图片数据
        send_str(fd, "\n");
        free(img);
    }
    fclose(fp);

    send_str(fd, "===END===\n");
    printf("[SRV] → 分组%d 广告已发送\n", group);
}

// 向某个分组的所有设备推送广告
void push_group(int group)
{
    int n = 0;
    for (int i = 0; i < MAX_CLI; i++)
    {
        if (g_devs[i].fd != -1 && g_devs[i].group == group)
        {
            send_ads(g_devs[i].fd, group);
            n++;
        }
    }
    printf("[SRV] 分组%d 推送完成, %d 台设备\n", group, n);
}

int main(void)
{
    // 初始化设备表
    for (int i = 0; i < MAX_CLI; i++)
    {
        g_devs[i].fd = -1;
    }

    int socfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socfd == -1)
    {
        perror("socket");
        return -1;
    }
    // 允许端口重用
    int val = 1;
    setsockopt(socfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(PORT));
    addr.sin_addr.s_addr = INADDR_ANY;
    // 绑定信息
    bind(socfd, (struct sockaddr *)&addr, sizeof(addr));
    listen(socfd, 100);
    printf("电梯广告管理服务器启动, 端口 %s\n", PORT);
    printf("命令: list(查看设备) push N(推送分组N) help\n\n");
    // 用epoll进行多路复用
    int epfd = epoll_create(1000);
    struct epoll_event evt = {.events = EPOLLIN};

    // 监听 socket
    evt.data.fd = socfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, socfd, &evt);

    // 监听标准输入（管理命令）
    evt.data.fd = STDIN_FILENO;
    evt.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &evt);

    struct epoll_event evt_arr[1024]; // 初始化

    while (1)
    {
        int n = epoll_wait(epfd, evt_arr, 1024, -1);
        for (int i = 0; i < n; i++)
        {
            int fd = evt_arr[i].data.fd;

            // 管理命令
            if (fd == STDIN_FILENO)
            {
                char cmd[256] = {}, *p;
                read(STDIN_FILENO, cmd, sizeof(cmd));
                cmd[strcspn(cmd, "\r\n")] = '\0'; // 去掉\r\n

                if (strcmp(cmd, "list") == 0)
                {
                    dev_list();
                }
                else if (strncmp(cmd, "push ", 5) == 0)
                {
                    int g = atoi(cmd + 5); // 跳过push 得到组号
                    printf("[SRV] 推送分组 %d 广告\n", g);
                    push_group(g);
                }
                else if (strcmp(cmd, "help") == 0)
                {
                    printf("  list   - 查看在线设备\n");
                    printf("  push N - 推送广告给分组N\n");
                }
                else
                {
                    printf("未知命令, 输入 help\n");
                }
                continue;
            }

            // 新连接
            if (fd == socfd)
            {
                struct sockaddr_in cilent;
                socklen_t len = sizeof(cilent);
                int cfd = accept(socfd, (struct sockaddr *)&cilent, &len);
                if (cfd == -1)
                {
                    continue;
                }

                evt.data.fd = cfd;
                evt.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &evt);
                dev_add(cfd, inet_ntoa(cilent.sin_addr));

                printf("[SRV] 新设备 fd=%d (%s)\n",
                       cfd, inet_ntoa(cilent.sin_addr));
                continue;
            }

            // 已有客户端消息
            char buf[256] = {};
            int r = recv(fd, buf, sizeof(buf) - 1, 0);
            if (r <= 0)
            {
                device_t *d = dev_find(fd);
                printf("[SRV] 设备断开 fd=%d", fd);
                if (d && d->id[0])
                    printf(" ID=%s", d->id);
                printf("\n");
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL); // 断开的设备不要监控
                close(fd);
                dev_del(fd);
                continue;
            }

            // 解析握手命令
            device_t *d = dev_find(fd);
            if (!d)
            {
                continue;
            }

            if (strncmp(buf, "===ID ", 6) == 0)
            {
                sscanf(buf, "===ID %63s GROUP %d===", d->id, &d->group);
                printf("[SRV] 设备注册: ID=%s 分组=%d\n", d->id, d->group);
                send_ads(fd, d->group);
            }
            else if(strncmp(buf, "===ALARM===", 11) == 0)
            {
                printf("[SRV] !!! 警报 !!! 设备 %s 分组 %d IP %s\n",
                       d->id[0] ? d->id : "?", d->group, d->ip);
            }
            else
            {
                printf("[SRV] fd=%d 消息: %s\n", fd, buf);
            }
        }
    }
    close(socfd);
    return 0;
}
