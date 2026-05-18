# S4S Hat Arduino Firmware — 项目知识库

本文档汇总了项目的关键信息、构建系统、架构设计和常见问题，供 AI 在后续对话中快速理解上下文。

---

## 一、项目概况

| 项目 | 内容 |
|------|------|
| 目标板 | Arduino UNO R4 WiFi |
| 功能 | 串口控制的扩展板（Hat），通过 UART 与上位机通信，控制主控板及外设 |
| 协议 | UDC2 自定义协议（帧头 `0xAA` \| 长度 \| 数据对象 \| 校验 `0x55`） |
| 框架 | PlatformIO (可选) + arduino-cli (当前主力) |
| 项目根 | `src/`（所有源码、配置、构建工具都在此目录下） |

---

## 二、构建系统

### 2.1 工具链

- **arduino-cli.exe v1.3.0** — 放在 `src/` 根目录
- **arduino-cli.yaml** — 本地配置，路径相对于 CWD
- **FQBN**: `arduino:renesas_uno:unor4wifi`
- **Platform**: `arduino:renesas_uno@1.5.3`
- **Toolchain**: `arm-none-eabi-gcc 7-2017q4`

### 2.2 目录结构

```
src/
├── arduino-cli.exe          # Arduino CLI
├── arduino-cli.yaml         # 本地配置
├── build.bat                # 一键构建脚本
├── src.ino                  # Arduino 入口文件（调用 prepare.h）
├── prepare.h / prepare.cpp  # Arduino setup()/loop() 定义
├── arduino_data/            # 核心和工具链（已下载的 renesas_uno 等）
├── arduino_user/            # 第三方库（已下载的 Adafruit SH110X 等）
├── include/                 # 配置文件（FreeRTOSConfig.h, zst_conf.h 等）
├── lib/                     # git 子模块库
│   ├── arduino_s4sMainBoard/  # 主控板电机/舵机/RTC 控制
│   ├── arduino_k210/          # AI 摄像头模块
│   ├── music_i2sPlayer/       # 音频播放
│   ├── udcheck/               # UDC2 协议库
│   └── zs_tools/              # 轻量级任务/事件/定时器框架
├── src/                     # 项目源码（main.cpp → main.cpp_ 以避冲突）
│   ├── interface/           # UART/I2C 通信层
│   ├── hardware/            # 硬件抽象层
│   └── device/              # 设备/协议层
└── test/                    # CMake 原生测试（独立编译）
```

### 2.3 arduino-cli.yaml

```yaml
board_manager:
    additional_urls: []
directories:
    user: ./arduino_user
    data: ./arduino_data
```

### 2.4 src.ino

```cpp
#include "prepare.h"

void app_setup(void) { }
void app_loop(void) { }
```

`setup()` 和 `loop()` 定义在 `prepare.cpp` 中，`app_setup()`/`app_loop()` 是用户自定义钩子（原名写在 `main.cpp` 中，现为 `main.cpp_`）。

---

## 三、build.bat 命令参考

所有命令在 `src/` 目录下执行。

| 命令 | 说明 |
|------|------|
| `build.bat init` | 安装核心 & 第三方库，自动创建 src.ino，文件管理 |
| `build.bat clear` | 清除编译缓存（删除 `arduino_data/staging/`） |
| `build.bat build [-v] [-d MACRO...]` | 编译，-v 显示详细输出，-d 添加宏定义 |
| `build.bat burn PORT [BAUD] [-v] [-d MACRO...]` | 编译+烧录到指定串口 |

### 示例

```bash
build.bat build                        # 仅编译
build.bat build -v                     # 显示详细编译过程
build.bat build -d NOT_CUSTOM_BUILD    # 带宏编译
build.bat build -v -d FOO -d BAR       # 详细+多宏
build.bat burn COM99                   # 编译+烧录
build.bat burn COM99 115200            # 指定波特率烧录
build.bat burn COM99 -d FOO -d BAR     # 烧录+宏
```

### 参数说明

- `-v`：启用详细输出（显示正在编译的文件）
- `-d MACRO`：添加编译宏（等效于 `-D MACRO`），可重复使用
- 位置参数：PORT 为串口号（如 COM99），BAUDRATE 为可选波特率

### 文件管理自动化

`build` 和 `burn` 命令在执行编译前会自动处理：
1. 将 `src/main.cpp` 重命名为 `src/main.cpp_`（避免与 .ino 冲突）
2. 将 `lib/zs_tools/src/component/cc/example/` 中的 `.c` 文件重命名为 `.c_`（避免编译测试文件）

---

## 四、外部依赖

### 4.1 第三方库（arduino-cli lib install）

| 库名 | 用途 |
|------|------|
| Adafruit BusIO | I2C/SPI 总线抽象 |
| Adafruit GFX Library | 图形绘制 |
| Adafruit QMC5883P Library | 磁力计 |
| Adafruit SH110X | OLED 显示屏驱动 |
| Seeed Arduino LSM6DS3 | 六轴陀螺仪 |

### 4.2 子模块库（`lib/` 目录，git submodule）

每个子模块需要 `library.properties` 文件才能被 arduino-cli 识别，`architectures=*`（从具体架构改为通用）。

---

## 五、架构概览

### 三层架构

```
src.ino → prepare.cpp → app_setup()/app_loop()
  └── zst_task_handler()  [zs_tools 协作式任务调度器]
        └── ptask_protocol
              └── udc_pack_task()  [解析串口数据包]
                    └── udc_event_receive_finsh()
                          └── function_map_exec()  [分发至设备处理函数]
```

1. **接口层** (`interface/`) — 封装 UART / I2C 总线访问
2. **硬件层** (`hardware/`) — 各类外设的 C++ 封装类，全局对象在 `hardware_init()` 中初始化
3. **设备/协议层** (`device/protocol/`) — 串口命令 → 硬件操作的映射，使用 `function_map_collection`（`std::map` 套 `std::map`）进行路由

### 添加新协议命令

1. 在对应的 `dev_protocol_*.cpp` 中添加处理函数
2. 在该文件的 `*_init()` 中通过 `function_map_t` 注册函数
3. 确保对应的硬件类暴露了所需方法

### 任务调度

- 基于 `zs_tools` 的协作式调度器
- `zst_init()` → `ptask_root_create()` → `ptask_create()` → `zst_task_handler()`（在 `loop()` 中运行）

---

## 六、关键注意事项

### 6.1 `-iprefix` 问题

**症状**：`bits/os_defines.h: No such file or directory`
**原因**：Arduino UNO R4 WiFi 的 platform.txt 中 `-iprefix{runtime.platform.path}` 会覆盖 GCC 内部前缀，破坏 multilib 路径解析。
**解决**：将 arduino-cli 相关文件放在 `src/` 根目录（即 sketch 目录），而非子目录。CWD = sketch 目录时构建正常。

### 6.2 main.cpp 与 .ino 冲突

Arduino 构建系统会自动编译 .ino 文件生成 `setup()`/`loop()`，此时不能有另一个包含 `setup()`/`loop()` 的 .cpp 文件。解决方案：
- `prepare.cpp` 包含 `setup()`/`loop()` 实现
- `main.cpp` 重命名为 `main.cpp_`（由 build.bat 自动处理）
- `app_setup()`/`app_loop()` 作为用户钩子在 `src.ino` 中定义

### 6.3 清缓存

修改头文件或库后若遇到奇怪错误，先执行 `build.bat clear` 再重新编译。缓存位于 `arduino_data/staging/`。

### 6.4 首次设置

```bash
cd src
build.bat init
```

这会安装核心、第三方库、创建 src.ino 并执行文件管理。

---

## 七、代码风格

- `.clang-format` 定义格式化规则（在 `src/` 下）
- 列宽限制：100
- 缩进：4 个空格
- 花括号换行（函数和控制语句 `{` 前换行）
- 指针对齐：右对齐
