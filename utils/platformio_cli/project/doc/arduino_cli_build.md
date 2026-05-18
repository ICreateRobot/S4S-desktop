# Arduino CLI 编译指南

使用 `arduino-cli.exe` 替代 PlatformIO 编译此项目。

## 目录结构

所有 arduino-cli 相关文件放在 `src/` 根目录：

```
src/
├── arduino-cli.exe          # Arduino CLI 可执行文件
├── arduino-cli.yaml         # 本地配置
├── build.bat                # 一键编译脚本
├── src.ino                  # Arduino 入口文件
├── arduino_data/            # 核心和工具链（已下载好的 renesas_uno 等）
├── arduino_user/            # 第三方库（已下载好的 Adafruit SH110X 等）
├── src/                     # 项目源码（main.cpp_ 是重命名后的 main.cpp）
├── include/                 # 配置文件
├── lib/                     # 子模块库
└── platformio.ini           # PlatformIO 配置（可选保留）
```

## 快速开始

在 `src/` 目录下执行：

```bash
build.bat
```

脚本会自动处理：
1. 检查 `arduino-cli.exe` 是否存在
2. 检查 `arduino-cli.yaml` 配置文件
3. 检查 `src.ino` 入口文件
4. 若存在 `src/src/main.cpp`，自动重命名为 `main.cpp_`（避免与 .ino 冲突）
5. 若 `lib/zs_tools/src/component/cc/example/` 存在，禁掉其中的 `.c` 测试文件
6. 编译项目

## 首次设置（全新电脑）

```bash
cd src

# 安装核心
arduino-cli.exe core update-index
arduino-cli.exe core install arduino:renesas_uno

# 安装第三方库
arduino-cli.exe lib install "Adafruit BusIO"
arduino-cli.exe lib install "Adafruit GFX Library"
arduino-cli.exe lib install "Adafruit QMC5883P Library"
arduino-cli.exe lib install "Adafruit SH110X"
arduino-cli.exe lib install "Seeed Arduino LSM6DS3"
```

## build.bat 一键命令

`src/build.bat` 提供四个子命令，在 `src/` 目录下执行：

| 命令 | 说明 |
|------|------|
| `build.bat init` | 安装核心 & 第三方库（首次设置） |
| `build.bat clear` | 清除编译缓存 |
| `build.bat build [-d MACRO...]` | 编译，可选添加宏（如 `-d NOT_CUSTOM_BUILD`） |
| `build.bat burn PORT [BAUD] [-d MACRO...]` | 编译并烧录到指定串口 |

示例：
```bash
build.bat build                      # 仅编译
build.bat build -d NOT_CUSTOM_BUILD  # 带宏编译
build.bat burn COM99                 # 编译+烧录
build.bat burn COM99 115200          # 指定波特率烧录
build.bat burn COM99 -d FOO -d BAR   # 带宏烧录
```

## 手动编译命令

```bash
cd src
arduino-cli.exe --config-file arduino-cli.yaml compile ^
    --fqbn arduino:renesas_uno:unor4wifi ^
    --build-property "build.extra_flags=-I src -I include -D NOT_CUSTOM_BUILD" ^
    --library lib/arduino_s4sMainBoard ^
    --library lib/arduino_k210 ^
    --library lib/music_i2sPlayer ^
    --library lib/udcheck ^
    --library lib/zs_tools ^
    .
```

## 关键文件说明

### arduino-cli.yaml
```yaml
board_manager:
    additional_urls: []
directories:
    user: ./arduino_user
    data: ./arduino_data
```
路径相对于 CWD，运行时 CWD 为 `src/`。

### src.ino
```cpp
#include "prepare.h"

void app_setup(void) { }
void app_loop(void) { }
```
`setup()` 和 `loop()` 定义在 `prepare.cpp` 中，`app_setup()`/`app_loop()` 是用户自定义钩子。

### 子模块 library.properties
arduino-cli 依赖 `library.properties` 识别库。以下子模块需要此文件：

- `lib/zs_tools/library.properties` — architectures=*
- `lib/udcheck/library.properties` — architectures=*
- `lib/music_i2sPlayer/library.properties` — architectures=*（从 unoR4WIFI 改为 *）

## 编译命令备忘

```bash
# 清缓存
rm -rf arduino_data/staging

# 重新安装核心
arduino-cli.exe core uninstall arduino:renesas_uno
arduino-cli.exe core install arduino:renesas_uno

# 更新
arduino-cli.exe core update-index
arduino-cli.exe core upgrade
arduino-cli.exe lib update-index
arduino-cli.exe lib upgrade
```
