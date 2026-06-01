# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 在本仓库中工作时提供指导。

## 构建

交叉编译目标为 ARM 平台（RV1103/RV1106 Luckfox Pico），需要 Luckfox SDK 工具链。

```bash
# 先设置 SDK 路径（必须使用绝对路径）
export LUCKFOX_SDK_PATH=/path/to/luckfox-pico

# 构建
./build.sh        # 创建 build/ 目录，执行 cmake + make install

# 清理
./build.sh clean  # 删除 build/ 和 install/

# 手动构建
mkdir build && cd build && cmake .. && make install
```

工具链路径硬编码在 `CMakeLists.txt` 的 `SDK_PATH` 变量中，需根据本地环境修改。

编译输出位于 `install/rkipc_terminal_demo/`。

## 架构

基于瑞芯微 RV1106/RV1103 SoC 的嵌入式 IPC（网络摄像头）应用，运行于 Buildroot 系统。通过 RKMPI 采集视频、编码，经 RTSP 推流，使用 RKNN 运行 YOLOv5 推理，同时驱动 LCD 显示。

### 核心设计模式

- **信号/槽** (`code/signal_slot.h`)：线程安全的观察者模式，用于模块间通信。模板类 `Signal<Args...>`，提供 `connect()`/`emit()` 接口。
- **抽象工厂** (`code/factory/`)：模块实例化采用工厂模式 — `baseVideoFactory`、`displayFactory`、`ledFactory`、`ptzFactory`。
- **观察者/发布者** (`code/observer/`)：基于信号/槽之上的事件发布系统。

### 模块布局 (`code/`)

| 模块 | 功能 |
|------|------|
| `video/` | 视频管线 — RTSP推流、LCD输出、YOLO推理、OSD叠加 |
| `display/` | LCD framebuffer 显示 |
| `net/` | 网络客户端 |
| `tcp/` | TCP 服务器 + UART 串口通信 |
| `control/` | 设备控制逻辑 |
| `ptz/` | 云台控制（PWM驱动） |
| `led/` | GPIO 控制 LED |
| `osd/` | OSD 字符/图形叠加（基于 freetype） |
| `param/` | INI 配置文件解析（`code/rkipc.ini`） |
| `onvif/` | ONVIF 协议服务器（基于 SOAP） |

### 视频管线类继承 (`code/video/`)

- `videoBase.h` — 基类
- `videoRTSP` — RTSP 推流（主输出）
- `videoLCD` — LCD 显示流
- `videoYOLO` — RKNN YOLOv5 推理 + OSD 标注

### 依赖库

静态/动态库位于 `lib/` — OpenCV、RGA、RKAIQ（ISP）、RKNN runtime、rockit、rockchip_mpp。头文件位于 `include/`。

## 语言

C/C++ 混合代码库。底层模块使用 C（RKMPI、framebuffer、GPIO、INI解析），高层模块使用 C++（视频管线、工厂、信号槽、观察者）。

## 部署

将 `install/` 输出文件夹通过 ADB/SSH 部署到 Luckfox Pico 开发板。运行前先在板端执行 `RkLunch-stop.sh` 释放摄像头占用，然后运行可执行文件。RTSP 流地址：`rtsp://<板端IP>/live/0`。
