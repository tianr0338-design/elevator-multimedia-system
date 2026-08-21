# 基于 LVGL 的电梯多媒体系统

一个运行在 **ARM Linux 开发板** 上的电梯多媒体终端项目：设备端使用 LVGL 8.3 渲染 800×480 触摸屏界面（开机动画、登录、媒体播放、小游戏、广告推送、电梯状态展示），并通过网络从 **管理服务器** 拉取分组广告素材。

> 说明：本文件根据仓库实际配置（`lv_conf.h`、`main.c`、`Makefile`、`.cproject`、`server.c` 等）整理，非通用模板。

---

## 1. 系统架构

```
┌─────────────────────────────┐         TCP :8888 (JSON / cJSON)        ┌──────────────────────────────┐
│   电梯多媒体终端 (ARM Linux)  │  ◀───────────────────────────────────▶ │   管理服务器 (Linux PC)        │
│                             │                                          │                              │
│  LVGL 8.3  (fbdev 800×480)  │                                          │  lift_server/server.c        │
│   ├─ main.c  (LVGL+显示+输入) │                                          │  - epoll 高并发               │
│   ├─ lift_client/ (主界面)    │                                          │  - 在线设备表(device_t)        │
│   └─ project/  (备选界面)     │                                          │  - 按 group 推送 ads/          │
│                             │                                          │    group_1、group_2           │
│  资源: LVGL 文件系统 "A:"    │                                          │                              │
└─────────────────────────────┘                                          └──────────────────────────────┘
```

- **显示后端**：Linux framebuffer（`lv_drivers/display/fbdev.c`，`/dev/fb0`），分辨率 **800×480**。
- **输入后端**：Linux evdev（`lv_drivers/indev/evdev.c`，`/dev/input/event*`），支持触摸/鼠标，带光标。
- **资源访问**：图片/字体/GIF 通过 LVGL 文件系统抽象以盘符 `"A:"` 引用（如 `"A:logo.png"`、`"A:kaiji.gif"`），后端驱动在初始化阶段注册，指向设备上的素材目录。

---

## 2. 目录结构

```
基于lvgl的电梯多媒体系统/
├── .gitignore                  # 忽略编译产物 / IDE / 系统临时文件
├── README.md                   # 本文件
├── lift_server/                # 管理服务器端（PC / Linux）
│   ├── server.c                #   epoll 服务器，端口 8888，设备分组管理
│   ├── cJSON.c / cJSON.h       #   JSON 解析（通信协议）
│   └── ads/                    #   广告素材（group_1 / group_2）
└── lvgl8.3/                    # LVGL 8.3 工程（含 vendored 库 + 业务代码）
    ├── main.c                  #   启动入口：LVGL 初始化 + fbdev + evdev + app_init()
    ├── Makefile                #   交叉编译构建（输出 demo）
    ├── lv_conf.h               #   LVGL 全局配置
    ├── mouse_cursor_icon.c     #   鼠标光标图标
    ├── demo                    #   编译产物（已被 .gitignore 忽略）
    ├── build/                  #   编译中间文件（已被 .gitignore 忽略）
    ├── lvgl/                   #   LVGL 核心库（vendored）
    ├── lv_drivers/             #   显示/输入驱动（fbdev / evdev / sdl ...）
    ├── lv_demos/               #   LVGL 官方 demos（vendored）
    ├── lift_client/            #   设备端主界面（被 Makefile 实际编译）
    │   ├── src/  app.c advertisement.c elevator.c home.c style.c font.c
    │   ├── inc/  对应头文件
    │   └── asset/ 客户端素材
    ├── project/                #   另一套电梯多媒体界面（备选 / 未纳入默认 Makefile）
    │   ├── src/  project.c(auth/home/media/game/common)
    │   └── inc/  对应头文件（含 .bak 备份）
    ├── demo_ui/                #   GUI 设计器生成的 3D 打印机示例 UI（参考）
    │   ├── CMakeLists.txt      #   仅编译为 ui 静态库
    │   ├── ui.c / screens/ / components/ / images/ / fonts/
    ├── ex/                     #   学习示例（home/login/nav/setting/style/font）
    └── mycode/                 #   个人练习与素材（arc / ex01 / test / *.gif *.jpg）
```

---

## 3. 关键配置

### 3.1 LVGL（`lvgl8.3/lv_conf.h`）
| 配置项 | 值 | 说明 |
|---|---|---|
| `LV_COLOR_DEPTH` | `32` | 32 位色（ARGB8888），匹配 Linux fbdev |
| `LV_MEM_SIZE` | `8 * 1024 * 1024` | LVGL 动态内存 8 MB |
| `LV_DISP_DEF_REFR_PERIOD` | `30` (ms) | 默认屏幕刷新周期 |
| `LV_USE_USER_DATA` | `1` | 允许给 LVGL 对象挂载自定义数据 |
| `LV_USE_LOG` | `0` | 日志关闭 |
| GPU 加速 | 全部 `0` | STM32 DMA2D / NXP PXP / SDL 均关闭 → **纯软件渲染** |
| 字体 | Montserrat 14/24/26 + **SIMSUN_16_CJK** | 启用简体中文（宋体 16） |
| 默认字体 | `lv_font_montserrat_14` | |
| 主题 | default / basic / mono 均 `1` | |
| 官方 demos | 全部 `0` | 不使用内置 demo |

> 屏幕分辨率 **800×480** 不在 `lv_conf.h` 中，而是在 `main.c` 的显示驱动里硬编码（`disp_drv.hor_res = 800; disp_drv.ver_res = 480`）。显示缓冲 `DISP_BUF_SIZE = 4 MB`。

### 3.2 构建与运行（`lvgl8.3/Makefile` / `main.c`）
| 项 | 值 |
|---|---|
| 交叉编译器 | `arm-linux-gcc` |
| 目标平台 | ARM Linux 开发板（framebuffer 屏） |
| 主入口 | `main.c` → `lv_init()` → `fbdev_init()` → `evdev_init()` → `app_init()`（位于 `lift_client`） |
| 额外模块 | `lv_extra_init()` 启用 PNG / BMP / **FreeType** 等 |
| 链接依赖 | `-lm`、`-lfreetype`（FreeType 2.13.3，绝对路径 `/freetype-2.13.3/...`） |
| 输出 | `demo`（位于 `lvgl8.3/`，已被忽略） |
| 构建目录 | `build/` |

### 3.3 服务器（`lift_server/server.c`）
- 采用 `epoll` 实现高并发，监听 **端口 8888**。
- 维护在线设备表 `device_t`（fd / id / group / ip），设备按 `group` 分组。
- 广告素材按组存放于 `ads/group_1/`、`ads/group_2/`，通过 **cJSON** 封装的 JSON 协议下发。
- 纯 Linux C（`<sys/epoll.h>` 等），用 `gcc` 即可编译。

---

## 4. 构建与运行

### 4.1 设备端（LVGL 终端）
前置依赖：
- 交叉编译工具链 `arm-linux-gcc`
- **FreeType 2.13.3**（头文件与库路径需在 `Makefile` 的 `CFLAGS` / `LDFLAGS` 中配置，当前写死为 `/freetype-2.13.3/...`）

```bash
cd lvgl8.3
make            # 产物：lvgl8.3/demo
# 拷贝到开发板后运行（需 /dev/fb0 与 /dev/input/event* 就绪）
./demo
```

> 也可用 **Eclipse CDT**（`.cproject` / `.project`，GNU cross 工具链）导入 `lvgl8.3` 工程进行图形化构建/调试。

### 4.2 管理服务器
```bash
cd lift_server
gcc server.c cJSON.c -o server -lpthread
./server        # 启动后监听 8888，管理在线设备并下发 ads/ 素材
```

---

## 5. 模块说明

- **lift_client（设备端主界面）**：`Makefile` 实际编译的业务代码。`app.c` 的 `app_init()` 为界面总入口；`advertisement.c` 负责广告展示，`elevator.c` 负责电梯状态，`home.c` 首页，`style.c` / `font.c` 负责样式与 FreeType 字体。
- **project（备选电梯多媒体界面）**：另一套实现，含开机动画 `qidong()`（logo + `kaiji.gif`）、登录 `auth`、首页 `home`、媒体 `media`、小游戏 `game`、公共组件 `common`。**注意**：当前 `Makefile` 的 `CSRCS` 只包含 `lift_client/src/*.c`，未包含 `project/src/*.c`，因此它未进入默认 `make` 产物，可能通过 Eclipse 工程单独构建或作为备选 UI。
- **demo_ui**：GUI 设计器（如 SquareLine / EEZ Studio）导出的 3D 打印机示例界面，仅 `add_library(ui ...)` 编译为静态库，作为 LVGL UI 开发参考。
- **ex**：零散学习示例（`home/login/nav/setting/style/font`），用于功能验证。
- **mycode**：个人练习与临时素材（`arc`、`ex01`、`test`、若干 `.gif/.jpg`）。
- **lift_server**：后台管理服务器，见 §3.3。

---

## 6. Git 与忽略规则

- 仓库已 `git init`，默认分支 `main`，`core.autocrlf=false`。
- 根目录 `.gitignore` 忽略：系统文件、IDE 临时文件（`.vscode/`、`.settings/`、`.project`、`.cproject`、`.clang-format`）、编译产物（`*.o` / `*.a` / `*.so` / `*.exe` 等）、构建目录（`build/`、`lvgl8.3/build/`）、以及二进制产物（`lvgl8.3/demo`、`lvgl8.3/project/bin/`）。
- `lvgl8.3/` 内自带的 `lvgl` / `lv_drivers` / `lv_demos` 原以 **git submodule** 方式引入（见 `lvgl8.3/.gitmodules`），当前已整体 vendored 进本仓库并纳入追踪。若希望精简仓库体积，可改回 submodule 或在 `.gitignore` 中排除这两个库目录。

---

## 7. 后续优化方向

1. **统一活动界面**：`Makefile` 当前默认编译 `lift_client`（正式主界面），`project/` 作为备选 UI，后续计划合并或移除，减少重复维护。
2. **FreeType 路径参数化**：当前在 `Makefile` 中按 `/freetype-2.13.3/...` 绝对路径链接，计划改为相对路径或环境变量，提升跨环境可移植性。
3. **素材路径配置化**：`"A:"` 文件系统盘符的挂载点（素材根目录）在初始化代码中注册，部署时需保证该路径存在且包含 `logo.png`、`kaiji.gif` 等资源。
