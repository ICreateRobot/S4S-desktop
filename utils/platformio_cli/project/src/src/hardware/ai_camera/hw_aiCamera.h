/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-20 21:56:04
 * @LastEditTime : 2026-05-21 16:47:52
 * @Description  : ai camera
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_AICAMERA_H__
#define __HW_AICAMERA_H__

#include "../common/hw_common.h"
#include "interface/i2c/if_i2c.h"

class hw_aiCamera_c
{
public:
    hw_aiCamera_c();
    ~hw_aiCamera_c();

public:
    enum
    {
        COLOR = 0,       // 颜色识别模式
        COLOR_TRACK,     // 色块追踪模式
        TAG,             // 标签识别模式
        LINE,            // 线条识别模式
        OBJECT,          // 物体识别模式
        QR,              // 二维码识别模式
        FACE_DETECT,     // 人脸检测模式
        FACE_RECOGNIZE,  // 人脸识别模式
        AI,              // 深度学习模式
        CARD,            // 路标识别模式

        AXIS_X = 0,       // 物体位置X轴
        AXIS_Y,           // 物体位置Y轴
        AXIS_W,           // 物体宽度
        AXIS_H,           // 物体高度
        DOWN=1,           // 下区域
        MIDDLE,           // 中区域
        UP,               // 上区域

        GO_FORWARD = 0,  // 前进
        GO_BACKWARD,     // 后退
        TURN_LEFT,       // 左转
        TURN_RIGHT,      // 右转
        STOP_MOVING,     // 停止

        AI_OFF = 0,     // AI未启动
        AI_CONNECTING,  // 连接中
        AI_IDLE,        // 待命
        AI_LISTENING,   // 聆听中
        AI_SPEAKING,    // 说话中
        AI_CONFIGURING, // 配网中

        RED = 1,       // 红色
        GREEN,         // 绿色
        BLUE,          // 蓝色
        YELLOW,        // 黄色
        BLACK,         // 黑色
        WHITE,         // 白色

        OBJECT_AIRPLANE = 0, 	// 飞机
        OBJECT_BICYCLE,
        OBJECT_BIRD,
        OBJECT_BOAT,
        OBJECT_BOTTLE,
        OBJECT_BUS,
        OBJECT_CAR,
        OBJECT_CAT,
        OBJECT_CHAIR,
        OBJECT_COW,
        OBJECT_DININGTABLE,
        OBJECT_DOG,
        OBJECT_HORSE,
        OBJECT_MOTORBIKE,
        OBJECT_PERSON,
        OBJECT_POTTEDPLANT,
        OBJECT_SHEEP,
        OBJECT_SOFA,
        OBJECT_TRAIN,
        OBJECT_TV,

        EVENT_HONK = 5,	// 鸣笛
        EVENT_TARGET,	// 标靶

        FACE_OPEN_MOUTH = 1,	// 张嘴
        FACE_SMILE,		    // 微笑
        FACE_GLASSES,		// 戴眼镜
    };

public:
    // 0:主板i2c; 1:外接i2c
    int set_i2c_port(uint8_t port);

    // 切换视觉识别模块的模式
    int set_mode(int mode);
    // 当前模式
    int get_mode(void);
    // 获取识别选定的颜色值 （R/G/B）
    int color_value(int color);
    // 将跟踪颜色设置为指定的颜色
    int set_color(int color);
    // 目标颜色已检测到吗？
    int color_detected(void);
    // 获取该颜色块的指定位置信息 （x、y、w、h）
    int color_position(int select);
    // 已识别标签的数量
    int tag_count(void);
    // 识别标签内容; 卡片索引 （可以不传入，默认1）
    int tag_id(int id = 1);
    // 标签旋转角度; 卡片索引 （可以不传入，默认1）
    int tag_rotation(int id = 1);
    // 标签位置; 卡片索引 （可以不传入，默认1）
    int tag_position(int select, int id = 1);
    // 一条线被识别了吗？
    int line_detected(void);
    // 获取指定位置的行位置信息；获取线条位置信息（上/中/下 + X/Y/W/H）
    int line_position(int region, int info);
    // 已识别物体的数量 （20类物体）
    int object_count(void);
    // 识别出指定对象了吗？
    int object_detected(int object_class, int id = 1);
    // 已识别的物体位置信息 x,y,z,h 分别0~4 枚举	顺序1~4 可以不使用默认1
    // 获取物体位置信息（X/Y/W/H）
    int object_position(int select, int id = 1);
    // 二维码被识别了吗？
    int qr_detected(void);
    // 已识别的二维码内容
    String qr_data(void);
    // 已识别的二维码位置信息 x,y,z,h 分别0~4 枚举
    int qr_position(int select);
    // 检测到的人脸数量
    int face_count(void);
    // 所选检测到的人脸的位置信息  x,y,z,h 分别0~4 枚举	人脸索引 1~4
    int face_position(int select, int id = 1);
    // 所选的面部是否符合指定条件 1~3  :1张嘴/2微笑/3戴眼镜	人脸索引 1~4
    int face_attribute(int attribute, int id = 1);
    // 学习当前的人脸
    int face_recognized_learn(void);
    // 已识别的人脸数量
    int face_recognized_count(void);
    // 是否检测到了一张学习过的脸
    int face_recognized_detected(void);
    // 所选识别面部的位置信息 x,y,z,h 分别0~4 枚举	人脸索引 1~4
    int face_recognized_position(int select, int id = 1);
    // 指定的类是否已被识别
    int class_recognized(int class_id);
    // 已识别的卡片数量
    int card_count(void);
    // 指定的这张卡是否已被识别？
    int card_detected(int type, int sel, int id = 1);
    // 识别出的卡片的位置信息
    int card_position(int select, int id = 1);
    // 当前状态是否与指定状态相符 0：ai未启动、1：连接中、2：待命、3：聆听中、4：说话中、5：配网中
    int state_is(int state);
    // 是否检测到了指定的运动指令 0：前进、1：后退、2：左转、3：右转、4：停止
    int motion_command_detected(int command);
    // 检测到的运动速度值
    int motion_speed(void);
    // 检测到自定义命令
    int custom_command(void);
    // 获取操纵杆位置 坐标枚举
    int joystick_position(int select);
    // bool 按键状态 1~6
    int button_pressed(int button);
    // 是否按下了指定的键盘按键 1~4 wasd
    int key_pressed(int key);
    // （1开，0关） 可以使用设置补光亮度
    int fill_light(int level);
    // 设置补光亮度 补光灯亮度（0-10) 0关闭
    int set_fill_light_brightness(int brightness);
    // 获取当前补光亮度
    int get_fill_light_brightness(void);


protected:
    int writeReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
    int readReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len);
    int isOnline(uint8_t dev_addr);

private:
    if_i2c &i2c_handle;
    const uint8_t AICAMERA_ADDR = 0x24;
};

extern hw_aiCamera_c hw_ai_camera;

#endif /* __AI_CAMERA_H__ */
