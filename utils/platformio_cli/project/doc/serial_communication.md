[TOC]



- 除非特殊说明，所有的多字节按照大端模式（高位在前，低位在后）的形式填写

## 协议打包规范

数据包的打包，遵守udc2协议规范

| 包头 |            包长            |       数据       | 数据（n） |    校验    |
| :--: | :------------------------: | :--------------: | :-------: | :--------: |
| 0xAA | 从第一个数据到校验位的长度 | id + size + data |           | 固定为0x55 |



## 协议通讯规范

为保证以后可能的兼容性与扩展，此文档的协议id从10开始，前10（0-9）个id暂做保留。

除非特殊说明，否则统一采用以下形式：

**数据包内容**

一个完成的数据包，包含 `目标`、`函数名称`、`参数`

**数据格式**

以上三种类型均以字符串的形式发送

**参数格式**

目前仅支持三种数据类型 `int` 、`float` 、`string`

如 12，-32，1202 等均属于 `int` 类型；

如 12.4、3.0、-2.1 等属于 `float` 类型；

如 "123"、"abc" 等属于`string` 类型；（注意需要包含两侧的 `""`）

*12 、 12.0、"12.0" 属于三种不同的数据类型，不要混用！！*



## 设备操作

### 目标（ID=10）

| 数据ID | 数据SIZE | 数据                                                         |
| :----: | :------: | :----------------------------------------------------------- |
|   10   |    n     | 标记<br />sys：sys<br />mainBoard：操作的是mainBoard设备（缺省）<br />gray：四路灰度<br />cultr：炫彩超声波<br />aiCamera：K210 视觉模块<br />esp_audio：ESP 音频<br />esp_oled：Arduino 板载OLED显示控制<br />esp_pin：Arduino 引脚控制 |

> 目标值为缺省，默认是mainBoard

---

## 设备

### sys （`sys`）

---

#### version_get

```
version_get()
```

获取软件版本

**返回值**

三段版本信息，如 `0.1.0`

---



### 主板（`mainBoard`）

#### encoder_motor_run_dir_state

```plain
encoder_motor_run_dir_state(int: motor, int: dir, int: state, int: data, int:timeout=-1)
```

控制编码电机以指定方向运行指定时间、圈数或距离。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`
- **dir**: 旋转方向；`0`: 正转, `1`: 反转
- **state**: 运行模式；`0`: 按秒运行, `1`: 按圈数运行, `2`: 按厘米运行, `3`\:按照度数运行
- **data**: 运行数值（秒数、圈数或厘米数，根据 state 决定）
- **timeout**： -1无限等待，>=0 最长等待时间 （单位：秒）

**返回值**

无

---

#### encoder_motor_run

```plain
encoder_motor_run(int: motor, int: dir)
```

启动编码电机以动态速度模式运行。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`
- **dir**: 旋转方向；`0`: 正转, `1`: 反转

**返回值**

无

---

#### encoder_motor_stop

```plain
encoder_motor_stop(int: motor)
```

停止指定编码电机（电机会失能）。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`

**返回值**

无

---

#### encoder_motor_set_dynamic_speed

```plain
encoder_motor_set_dynamic_speed(int: motor, int: speed)
```

设置编码电机的动态速度。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`
- **speed**: 速度值；取值范围 `0 ~ 100`

**返回值**

无

---

#### encoder_motor_get_angle

```plain
encoder_motor_get_angle(int: motor)
```

获取编码电机的旋转角度。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`

**返回值**

电机旋转角度数值字符串（单位：度）

---

#### encoder_motor_get_dynamic_speed

```plain
encoder_motor_get_dynamic_speed(int: motor)
```

获取编码电机的当前动态速度。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`

**返回值**

速度数值字符串；取值范围 `0 ~ 100`

---

#### encoder_motor_reset_angle

```plain
encoder_motor_reset_angle(int: motor)
```

将指定编码电机的当前位置重置为零点。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`

**返回值**

无

---

#### encoder_motor_start_rpm_speed

```plain
encoder_motor_start_rpm_speed(int: motor, int: speed)
```

以指定动力值启动编码电机。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`
- **speed**: 动力值；内部电源取值范围 `-100 ~ 100`，外部电源取值范围 `-180 ~ 180`；负值表示反转

**返回值**

无

---

#### encoder_motor_get_rpm_speed

```plain
encoder_motor_get_rpm_speed(int: motor)
```

获取编码电机的当前动力值。

**参数**

- **motor**: 电机端口号；取值范围 `0 ~ 3`

**返回值**

动力数值字符串；取值范围 `-180 ~ 180`

---

#### encoder_motor_pair_set_group

```plain
encoder_motor_pair_set_group(int: l_motor, int: r_motor)
```

设置双电机运动组的左右电机端口号。

**参数**

- **l_motor**: 左电机端口号；取值范围 `0 ~ 3`
- **r_motor**: 右电机端口号；取值范围 `0 ~ 3`

**返回值**

无

---

#### encoder_motor_pair_run

```plain
encoder_motor_pair_run(int: state)
```

控制双电机按指定状态运动。

**参数**

- **state**: 运动状态；`0`: 前进, `1`: 后退, `2`: 左转, `3`: 右转

**返回值**

无

---

#### encoder_motor_pair_run_for

```plain
encoder_motor_pair_run_for(int: state, int: data, int: _for, int: timeout=-1)
```

控制双电机按指定状态运行指定时间、圈数或距离。

**参数**

- **state**: 运动状态；`0`: 前进, `1`: 后退, `2`: 左转, `3`: 右转
- **data**: 运行数值
- **_for**: 运行模式；`0`: 按秒运行, `1`: 按圈数运行, `2`: 按厘米运行
- **timeout**:-1无限等待，>=0 最长等待时间 （单位：秒）

**返回值**

无

---

#### encoder_motor_pair_run_dynamic_speed

```plain
encoder_motor_pair_run_dynamic_speed(int: l_speed, int: r_speed)
```

以指定左右速度控制双电机运动。

**参数**

- **l_speed**: 左电机速度；取值范围 `-100 ~ 100`；负值表示反转
- **r_speed**: 右电机速度；取值范围 `-100 ~ 100`；负值表示反转

**返回值**

无

---

#### encoder_motor_pair_stop

```plain
encoder_motor_pair_stop()
```

停止双电机运动。

**返回值**

无

---

#### encoder_motor_pair_set_dynamic_speed

```plain
encoder_motor_pair_set_dynamic_speed(int: speed)
```

设置双电机运动的统一速度。

**参数**

- **speed**: 速度值；取值范围 `0 ~ 100`

**返回值**

无

---

#### version_get

```
version_get()
```

获取主板的版本

**返回值**

三段版本信息字符串；如 0.1.0

---

#### ambient_light_set_state

```plain
ambient_light_set_state(int: light, int: r, int: g, int: b)
```

设置环境光（RGB 灯）的状态。

**参数**

- **light**: 亮度值；取值范围 `0 ~ 255`
- **r**: 红色分量；取值范围 `0 ~ 255`
- **g**: 绿色分量；取值范围 `0 ~ 255`
- **b**: 蓝色分量；取值范围 `0 ~ 255`

**返回值**

无

---

#### rtc_set_data

```plain
rtc_set_data(int: year, int: month, int: day)
```

设置 RTC 实时时钟的日期。

**参数**

- **year**: 年份；取值范围 `0 ~ 99`（表示 2000 ~ 2099 年）
- **month**: 月份；取值范围 `1 ~ 12`
- **day**: 日期；取值范围 `1 ~ 31`

**返回值**

无

---

#### rtc_set_time

```plain
rtc_set_time(int: hour, int: minute, int: second)
```

设置 RTC 实时时钟的时间。

**参数**

- **hour**: 小时；取值范围 `0 ~ 23`
- **minute**: 分钟；取值范围 `0 ~ 59`
- **second**: 秒；取值范围 `0 ~ 59`

**返回值**

无

---

#### rtc_get_data

```plain
rtc_get_data(int: sel)
```

获取 RTC 实时时钟的日期信息。

**参数**

- **sel**: 选择获取的日期类型；`0`: 年份, `1`: 月份, `2`: 日期, `3`: 星期

**返回值**

选定的日期数值字符串

---

#### rtc_get_time

```plain
rtc_get_time(int: sel)
```

获取 RTC 实时时钟的时间信息。

**参数**

- **sel**: 选择获取的时间类型；`0`: 小时, `1`: 分钟, `2`: 秒

**返回值**

选定的时间数值字符串

---

#### servo_set_angle

```plain
servo_set_angle(int: id, int: angle)
```

设置舵机的旋转角度。

**参数**

- **id**: 舵机 ID；取值范围 `0 ~ 1`
- **angle**: 角度值；取值范围 `0 ~ 180`

**返回值**

无

---

#### continuous_servo_set_speed

```plain
continuous_servo_set_speed(int: id, int: speed)
```

设置连续旋转舵机的转速。

**参数**

- **id**: 舵机 ID；取值范围 `0 ~ 1`
- **speed**: 速度值；取值范围 `-100 ~ 100`；负值表示反向旋转

**返回值**

无

---

#### voice_get_state

```plain
voice_get_state()
```

获取语音模块的识别状态。

**返回值**

语音指令状态码数值字符串

---



### 四路灰度（`gray`）

| 命令标识（十进制） | 功能描述                                     |
| ------------------ | -------------------------------------------- |
| 0                  | 空闲模式（无操作，传感器保持待机状态）       |
| 1                  | 颜色识别模式（检测并输出当前目标颜色类型）   |
| 2                  | 灰度识别模式（检测并输出当前目标灰度等级）   |
| 3                  | 二值识别模式（检测并输出当前目标二值化状态） |
| 4                  | 灰度学习模式（进入灰度阈值训练状态）         |
| 5                  | 二值学习模式（进入二值化阈值训练状态）       |
| 6                  | 清除所有已存储的颜色学习数据（恢复默认阈值） |
| 7                  | 学习红色阈值（针对红色通道单独训练阈值）     |
| 8                  | 学习绿色阈值（针对绿色通道单独训练阈值）     |
| 9                  | 学习蓝色阈值（针对蓝色通道单独训练阈值）     |
| 10                 | 学习黄色阈值（针对黄色混合色训练阈值）       |
| 11                 | 学习青色阈值（针对青色混合色训练阈值）       |
| 12                 | 学习紫色阈值（针对紫色混合色训练阈值）       |
| 13                 | 保留（暂未开放功能） 😓                       |
| 14                 | 保留（暂未开放功能） 😓                       |
| 15                 | 读取原始光敏值（输出0~255范围的模拟量）      |

------

#### set_i2c_port

```
set_i2c_port(int: port)
```

设置传感器的 I2C 通信端口。

**参数**

- **port**: 端口号 

  0：内部IIC

  1：外部IIC

  **返回值**

  无

------

#### gray_study

```
gray_study()
```

触发灰度学习模式，用于校准黑色和白色基准。

**返回值**

无

------

#### binary_study

```
binary_study()
```

触发二值化学习模式，用于设定黑白临界点。

**返回值**

无

------

#### color_study

```
color_study(int: color)
```

触发特定颜色的学习模式。

**参数**

- **color**: 需要学习的目标颜色编号

  **返回值**

  无

------

#### clear_color

```
clear_color()
```

清除已学习的颜色配置数据。

**返回值**

无

------

#### gray

```
gray(int: port)
```

获取指定端口传感器的原始灰度值。

**参数**

- **port**: 选择其中一个端口 （0 ~ 3）

  **返回值**

  灰度数值字符串。

------

#### color

```
color(int: port)
```

获取指定端口传感器识别到的颜色编号。

**参数**

- **port**:  选择其中一个端口 （0 ~ 3）

  **返回值**

  颜色编号的字符串。

------

#### black

```
black(int: port)
```

判断指定端口传感器是否检测到黑色（二值化结果）。

**参数**

- **port**:  选择其中一个端口 （0 ~ 3）

  **返回值**

  检测状态字符串（通常 1 表示黑色，0 表示非黑）。

------

#### photosensitive

```
photosensitive(int: port)
```

获取指定端口传感器的光敏强度数据。

**参数**

- **port**: 选择其中一个端口 （0 ~ 3）

  **返回值**

  光敏强度数值字符串。

---



### 炫彩超声波（`cultr`）

------

#### set_i2c_port

```
set_i2c_port(int: port)
```

设置超声波传感器的 I2C 通信端口。

**参数**

- **port**: 端口号

  **返回值**

  无

------

#### get_distance

```
get_distance()
```

获取超声波传感器测量到的距离。

**返回值**

距离数值字符串（通常单位为 cm）。

------

#### set_color

```
set_color(int: light, int: red, int: green, int: blue)
```

设置超声波传感器上 RGB 灯光的状态和颜色。

**参数**

- **light**: 亮度 （0-255）

- **red**: 红色分量 (0-255)

- **green**: 绿色分量 (0-255)

- **blue**: 蓝色分量 (0-255)

  **返回值**

  无

---



### K210（`aiCamera`）

------

#### set_i2c_port

```
set_i2c_port(int: port)
```

设置 AI 摄像头的 I2C 通信端口。

**参数**

- **port**: 端口号

  **返回值**

  无

------

#### set_sys_mode

```
set_sys_mode(int: mode)
```

设置系统运行模式（如：颜色识别、人脸识别等）。

**参数**

- **mode**: 模式枚举值

  **返回值**

  无

------

#### get_sys_mode

```
get_sys_mode()
```

获取系统当前的运行模式。

**返回值**

当前模式的数值字符串。

------

#### get_color_rgb

```
get_color_rgb(int: sel)
```

获取摄像头中心点的 RGB 颜色分量。

**参数**

- **sel**: 0:红色分量(R), 1:绿色分量(G), 2:蓝色分量(B)

  **返回值**

  对应颜色分量的数值字符串（0-255）。

------

#### set_find_color

```
set_find_color(int: color)
```

设置需要寻找的目标颜色。

**参数**

- **color**: 目标颜色编号或值

  **返回值**

  无

------

#### face_study

```
face_study()
```

触发人脸学习/录入功能。

**返回值**

无

------

#### deep_learn_study

```
deep_learn_study()
```

触发深度学习目标的录入功能。

**返回值**

无

------

#### get_qrcode_content

```
get_qrcode_content()
```

获取识别到的二维码文本内容。

**返回值**

二维码内容的字符串。

------

#### get_identify_num

```
get_identify_num(int: features, int: total)
```

获取指定特征目标的识别数量。

**参数**

- **features**: 功能/特征类型

- **total**: 预设的总数（可选，默认为0）

  **返回值**

  识别到的数量字符串。

------

#### get_face_attributes

```
get_face_attributes(int: index, int: sel)
```

获取识别到的人脸属性信息。

**参数**

- **index**: 人脸索引

- **sel**: 0:是否张嘴, 1:是否微笑, 2:是否戴眼镜

  **返回值**

  属性状态字符串（1 表示是，0 表示否）。

------

#### get_identify_id

```
get_identify_id(int: features, int: index)
```

获取识别目标的 ID 编号。

**参数**

- **features**: 功能/特征类型

- **index**: 目标索引

  **返回值**

  目标 ID 的字符串。

------

#### get_identify_rotation

```
get_identify_rotation(int: features, int: index)
```

获取识别目标的旋转角度。

**参数**

- **features**: 功能/特征类型

- **index**: 目标索引

  **返回值**

  旋转角度的字符串数值。

------

#### get_identify_position

```
get_identify_position(int: features, int: index, int: sel)
```

获取识别目标在画面中的位置和尺寸信息。

**参数**

- **features**: 功能/特征类型

- **index**: 目标索引

- **sel**: 0:坐标X, 1:坐标Y, 2:宽度W, 3:高度H

  **返回值**

  对应位置或尺寸的字符串数值。

------

#### set_light_status

```
set_light_status(int: status)
```

设置摄像头补光灯的开关状态。

**参数**

- **status**: 1 为开启，0 为关闭

  **返回值**

  无

------

#### set_light_brightness

```
set_light_brightness(int: brightness)
```

设置补光灯的亮度。

**参数**

- **brightness**: 亮度值（通常 0-100）

  **返回值**

  无

------



### ESP 音频（`esp_audio`）

------

#### get_state

```
get_state(int: state)
```

获取音频系统的播放、音量、增益或录音状态。

**参数**

- **state**: 获取类型。

  - `3`: 播放状态

  - `6`: 音量

  - `7`: 增益

  - `10`: 录音状态

    **返回值**

    对应状态的数值字符串。

------

#### music_play

```
music_play(int: state)
```

设置音乐播放器的运行状态。

**参数**

- **state**: 0: 停止播放, 1: 开始播放, 2: 重新播放

  **返回值**

  无

------

#### set_file_path

```
set_file_path(string: file_path)
```

设置要播放的音频文件完整路径。

**参数**

- **file_path**: 文件路径（如："/music/song.mp3"）

  **返回值**

  无

------

#### set_file_name

```
set_file_name(string: function_name)
```

设置要播放的音频文件名称。

**参数**

- **function_name**: 文件名称（如："song.mp3"）

  **返回值**

  无

------

#### set_volume

```
set_volume(int: volume)
```

设置系统的输出音量。

**参数**

- **volume**: 音量大小 (0 - 100)

  **返回值**

  无

------

#### set_gain

```
set_gain(int: gain)
```

设置系统的音频增益。

**参数**

- **gain**: 增益大小 (0 - 100)

  **返回值**

  无

------

#### recoding_set_file_path

```
recoding_set_file_path(string: file_path)
```

设置录音文件的存储路径。

**参数**

- **file_path**: 存储路径

  **返回值**

  无

------

#### recoding_set_file_name

```
recoding_set_file_name(string: function_name)
```

设置录音文件的保存名称。

**参数**

- **function_name**: 文件名称

  **返回值**

  无

------

#### recoding_play

```
recoding_play(int: play)
```

控制录音播放或停止录音。

**参数**

- **play**: 0: 停止播放/录音, 1: 播放录音, 2: 重新播放

  **返回值**

  无

------

#### recoding_start_recording

```
recoding_start_recording(int: sec)
```

启动录音并设置录制时长。

**参数**

- **sec**: 录音时长 (1 ~ 60 秒)

  **返回值**

  无

---



### Arduino板载OLED（`esp_oled`）

------

#### init

```plain
init()
```

初始化 OLED 显示屏。

**说明**

- 在开机时自动初始化，不需要手动调用

**返回值**

无

------

#### clear_screen

```plain
clear_screen()
```

清除屏幕缓冲区，将屏幕内容清空。

**返回值**

无

------

#### set_text_size

```plain
set_text_size(int: size)
```

设置文本显示大小。

**参数**

- **size**: 字体大小倍数；`1` 为默认 6×8 像素，`2` 为 12×16 像素，`3` 为 18×24 像素，以此类推；取值范围 `1 ~ 4`

**返回值**

无

------

#### print

```plain
print(int: x, int: y, string: str)
```

向屏幕缓冲区写入文本内容。

**参数**

- **x**: 起始 X 坐标（像素）
- **y**: 起始 Y 坐标（像素）
- **str**: 要显示的文本字符串

**返回值**

无

------

#### draw_line

```plain
draw_line(int: x0, int: y0, int: x1, int: y1)
```

在屏幕上绘制一条直线。

**参数**

- **x0**: 起点 X 坐标
- **y0**: 起点 Y 坐标
- **x1**: 终点 X 坐标
- **y1**: 终点 Y 坐标

**返回值**

无

------

#### draw_rect

```plain
draw_rect(int: x, int: y, int: w, int: h)
```

在屏幕上绘制一个无填充的矩形边框。

**参数**

- **x**: 矩形左上角的 X 坐标
- **y**: 矩形左上角的 Y 坐标
- **w**: 矩形的宽度（像素）
- **h**: 矩形的高度（像素）

**返回值**

无

------

#### draw_circle

```plain
draw_circle(int: x, int: y, int: r)
```

在屏幕上绘制一个无填充的圆形边框。

**参数**

- **x**: 圆心的 X 坐标
- **y**: 圆心的 Y 坐标
- **r**: 圆的半径（像素）

**返回值**

无

------

#### refresh

```plain
refresh()
```

将缓冲区内容刷新到屏幕显示。

**说明**

- 所有绘图操作（print、draw_line、draw_rect、draw_circle 等）均在缓冲区中进行，需要调用此函数才能实际显示到屏幕上

**返回值**

无

----

### Arduino 引脚控制（`esp_pin`）

引脚映射表

| 引脚编号 | 实际对应引脚 | 引脚编号 | 实际对应引脚 |
| :------: | :----------: | :------: | ------------ |
|    0     |     P101     |    20    | P304         |
|    1     |     P100     |    21    | P112         |
|    2     |     P408     |    22    | P111         |
|    3     |     P014     |    23    | P107         |
|    4     |     P000     |    24    | P106         |
|    5     |     P001     |    25    | P105         |
|    6     |     P113     |    26    | P104         |
|    7     |     P002     |    27    | P302         |
|    8     |     P400     |    28    | P301         |
|    9     |     P401     |    29    | P003         |
|    10    |     P011     |    30    | P004         |
|    11    |     P012     |          |              |
|    12    |     P013     |          |              |
|    13    |     P015     |          |              |
|    14    |     P204     |          |              |
|    15    |     P102     |          |              |
|    16    |     P410     |          |              |
|    17    |     P411     |          |              |
|    18    |     P103     |          |              |
|    19    |     P303     |          |              |

------

#### setMode

```
setMode(int: pin, int: mode)
```

设置指定引脚的输入输出模式。

**参数**

- **pin**: 引脚编号。

- **mode**: 模式控制

  0 ：输入

  1：输出

  2：输入上拉

  3：输入下拉

  4：开漏输出

  **返回值**

  无

------

#### digitalWrite

```
digitalWrite(int: pin, int: value)
```

设置数字引脚的输出电平。

**参数**

- **pin**: 引脚编号。

- **value**: 电平值（ 0 为低电平，1 为高电平）。

  **返回值**

  无

------

#### digitalRead

```
digitalRead(int: pin)
```

读取数字引脚的当前电平状态。

**参数**

- **pin**: 引脚编号。

  **返回值**

  电平状态的数值字符串（"0" 或 "1"）。

----





