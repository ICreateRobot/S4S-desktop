# CLAUDE.md

本文件为 Claude Code (claude.ai/code) 提供针对本代码仓库的工作指导。

## 项目概览

这是一个基于 PlatformIO 的 Arduino 固件项目，目标板为 **Arduino UNO R4 WiFi**（`renesas-ra` 平台）。该固件作为一个串口控制的扩展板（Hat），通过 UART 与上位机通信，进而控制主控板及各类外设。

- **协议文档**：`doc/serial_communication.md`
- **PlatformIO 项目根目录**：`src/`

## 常用命令

所有 PlatformIO 命令均应在 `src/` 目录下执行：

```bash
cd src

# 编译
pio run

# 编译并上传至开发板
pio run -v -t upload

# 格式化代码（使用 src/.clang-format）
clang-format -i src/**/*.cpp src/**/*.h
```

此外，`src/test/` 下还有不依赖 PlatformIO 的原生 CMake 测试（针对 `parse_value` 和 `udc` 逻辑），可独立编译运行。

## 高层架构

该固件采用三层架构，并配有一个轻量级任务调度器：

```
main.cpp
  └── zst_task_handler()  [zs_tools 协作式任务调度器]
        └── ptask_protocol
              └── udc_pack_task()  [解析串口数据包]
                    └── udc_event_receive_finsh()
                          └── function_map_exec()  [分发至设备处理函数]
```

### 1. 接口层（`src/interface/`）

封装底层总线访问：
- `if_uart.cpp/h` —— 与上位机进行 UART 通信
- `if_i2c.cpp/h` —— 通过 I2C 总线与外设通信

### 2. 硬件层（`src/hardware/`）

C++ 类，用于封装物理设备和第三方 Arduino 库。每个设备位于独立的子目录中（例如 `hardware/main_board/` 中的 `hw_main_board_c`，`hardware/esp_oled/` 中的 `hw_esp_oled`）。这些类实例化为全局对象，并在 `hardware_init()` 中初始化。

关键本地库（`src/lib/` 下的 git 子模块）：
- `arduino_s4sMainBoard` —— 主控板电机/舵机/RTC 控制
- `arduino_k210` —— AI 摄像头模块
- `music_i2sPlayer` —— 音频播放
- `udcheck` —— UDC2 数据包协议库
- `zs_tools` —— 轻量级任务/事件/定时器框架

外部 PlatformIO 依赖（`platformio.ini`）：
- `adafruit/Adafruit SH110X`
- `adafruit/Adafruit QMC5883P Library`
- `seeed-studio/Seeed Arduino LSM6DS3`

### 3. 设备/协议层（`src/device/protocol/`）

将接收到的串口命令映射为硬件操作。核心流程如下：

- **UDC2 协议** (`udcheck`) 帧格式：帧头 `0xAA` | 长度 | 数据对象 | 校验 `0x55`
- 接收到的数据包被解析为对象。对象 ID `10`（目标设备名）和 `11`（函数名）被提取为字符串。
- `function_map_exec()` 在 `function_map_collection` 中查找对应的处理函数（一个 device-name → function-name → handler 的 `std::map`）。
- 每个处理函数接收一个 `udc_pack_t*`，并返回一个 `fmap_result_t`（定义于 `dev_protocol_common.h`）。

添加新协议命令通常意味着：
1. 在对应的 `dev_protocol_*.cpp` 中添加处理函数
2. 在该文件的 `*_init()` 函数中通过 `function_map_t` 注册该命令
3. 确保对应的硬件类暴露了所需的方法

### 任务系统 (`zs_tools`)

项目使用 `zs_tools`（配置于 `include/zst_conf.h`）实现协作式多任务：
- `zst_init()` 初始化框架
- `ptask_root_create()` / `ptask_create()` 创建任务根节点和任务
- `zst_task_handler()` 在 `loop()` 中运行调度器
- 协议任务 (`ptask_protocol`) 周期性地调用 `udc_pack_task()` 来处理接收到的字节

### 代码风格

- `src/` 下的 `.clang-format` 定义了格式化规则
- 列宽限制：100
- 缩进宽度：4 个空格
- 自定义花括号换行（函数和控制语句在花括号前换行）
- 指针对齐：右对齐
