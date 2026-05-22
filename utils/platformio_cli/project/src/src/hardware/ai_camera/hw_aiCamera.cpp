/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-20 21:55:56
 * @LastEditTime : 2026-05-21 19:45:29
 * @Description  : ai camera
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hw_aiCamera.h"
#include <map>

/******************
 * data struct
 *****************/
#define LOG_TAG "hw_aiCamera_c"


/****************************
 * function declaration
 ***************************/
typedef std::map<int, int> num_map_t;


/********************
 * static variables
 *******************/
static const uint8_t SYSTEM_REG         = 0;   // 系统设置寄存器
static const uint8_t COLOR_REG          = 15;  // 颜色获取寄存器
static const uint8_t BLOB_REG           = 30;  // 色块获取寄存器
static const uint8_t APRILTAG_REG       = 45;  // 标签获取寄存器
static const uint8_t LINE_REG           = 60;  // 线条获取寄存器
static const uint8_t OBJECT_REG         = 75;  // 物体获取寄存器
static const uint8_t QR_REG             = 90;  // 二维码获取寄存器
static const uint8_t FACE_ATTRIBUTE_REG = 105; // 人脸属性寄存器
static const uint8_t FACE_IDENTIFY_REG  = 120; // 人脸识别寄存器
static const uint8_t DEEPSTUDY_REG      = 135; // 深度学习寄存器
static const uint8_t CARD_REG           = 150; // 卡片获取寄存器
static const uint8_t ESP_REG            = 165; // ESP32寄存器
static const uint8_t SETTING_REG        = 180; // 设置寄存器


/********************
 * global variables
 *******************/
hw_aiCamera_c hw_ai_camera;


/********************
 * global functions
 *******************/


/********************
 * class functions
 *******************/
hw_aiCamera_c::hw_aiCamera_c() : i2c_handle(if_i2c_internal_handle)
{}

hw_aiCamera_c::~hw_aiCamera_c()
{}

int hw_aiCamera_c::writeReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return this->i2c_handle.write_reg(dev_addr, reg, data, len);
}

int hw_aiCamera_c::readReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return this->i2c_handle.read_reg(dev_addr, reg, data, len);
}

int hw_aiCamera_c::isOnline(uint8_t dev_addr)
{
    return 0;
}


int hw_aiCamera_c::set_i2c_port(uint8_t port)
{
    switch (port)
    {
        case 0: this->i2c_handle = if_i2c_internal_handle; break;

        case 1: this->i2c_handle = if_i2c_external_handle; break;

        default: ZST_LOGW(LOG_TAG, "invalid i2c port > %d", port); return -1;
    }
    return 0;
}


// 切换视觉识别模块的模式
int hw_aiCamera_c::set_mode(int mode)
{
    DATA_CHECK(mode, 0, 9);
    int     ret_error = 0;
    uint8_t mode_data = (uint8_t)mode;
    ret_error         = this->writeReg(this->AICAMERA_ADDR, SYSTEM_REG + 0, &mode_data, 1);
    delay(50);
    return ret_error;
}

// 当前模式
int hw_aiCamera_c::get_mode(void)
{
    uint8_t mode_data = 0;
    int     ret_error = 0;

    ret_error = this->readReg(this->AICAMERA_ADDR, SYSTEM_REG + 0, &mode_data, 1);

    if (0 != ret_error)
        return mode_data;
    else
        return ret_error;
}

// 获取识别选定的颜色值 （R/G/B）
int hw_aiCamera_c::color_value(int color)
{
    DATA_CHECK(color, 1, 3);
    int     ret_error     = 0;
    uint8_t color_data[3] = {0};
    ret_error             = this->readReg(this->AICAMERA_ADDR, COLOR_REG + 0, color_data, 3);
    if (ret_error != 0) { return ret_error; }

    return color_data[color - 1];
}

// 将跟踪颜色设置为指定的颜色
int hw_aiCamera_c::set_color(int color)
{
    DATA_CHECK(color, 1, 6);
    int     ret_error  = 0;
    uint8_t color_data = (uint8_t)color;
    ret_error          = this->writeReg(this->AICAMERA_ADDR, BLOB_REG + 0, &color_data, 1);
    return ret_error;
}

// 目标颜色已检测到吗？
int hw_aiCamera_c::color_detected(void)
{
    uint8_t detected_data = 0;
    int     ret_error     = 0;

    ret_error = this->readReg(this->AICAMERA_ADDR, BLOB_REG + 1, &detected_data, 1);
    if (ret_error != 0) { return ret_error; }
    return detected_data;
}

// 获取该颜色块的指定位置信息 （x、y、w、h）
int hw_aiCamera_c::color_position(int select)
{
    DATA_CHECK(select, 0, 3);
    uint8_t position_data[8] = {0};
    int     ret_error        = 0;

    ret_error = this->readReg(this->AICAMERA_ADDR, BLOB_REG + 2, position_data, 8);
    if (ret_error != 0) { return ret_error; }
    return (position_data[select * 2] << 8) | position_data[select * 2 + 1];
}

// 已识别标签的数量
int hw_aiCamera_c::tag_count(void)
{
    int     ret_error  = 0;
    uint8_t count_data = 0;
    ret_error          = this->readReg(this->AICAMERA_ADDR, APRILTAG_REG + 0, &count_data, 1);
    if (ret_error != 0) { return ret_error; }
    return count_data;
}

// 识别标签内容; 卡片索引 （可以不传入，默认1）
int hw_aiCamera_c::tag_id(int id)
{
    DATA_CHECK(id, 1, 4);
    int     ret_error   = 0;
    uint8_t id_data[12] = {0};
    ret_error           = this->readReg(this->AICAMERA_ADDR, APRILTAG_REG + id, id_data, 12);
    if (ret_error != 0) { return ret_error; }
    return (id_data[0] << 8) | id_data[1];
}

// 标签旋转角度; 卡片索引 （可以不传入，默认1）
int hw_aiCamera_c::tag_rotation(int id)
{
    DATA_CHECK(id, 1, 4);
    int     ret_error   = 0;
    uint8_t id_data[12] = {0};
    ret_error           = this->readReg(this->AICAMERA_ADDR, APRILTAG_REG + id, id_data, 12);
    if (ret_error != 0) { return ret_error; }
    return (id_data[2] << 8) | id_data[3];
}

// 标签位置; 卡片索引 （可以不传入，默认1）
int hw_aiCamera_c::tag_position(int select, int id)
{
    DATA_CHECK(id, 1, 4);
    DATA_CHECK(select, 0, 3);
    int     ret_error   = 0;
    uint8_t id_data[12] = {0};
    ret_error           = this->readReg(this->AICAMERA_ADDR, APRILTAG_REG + id, id_data, 12);
    if (ret_error != 0) { return ret_error; }
    return (id_data[4 + select * 2] << 8) | id_data[4 + select * 2 + 1];
}

// 一条线被识别了吗？
int hw_aiCamera_c::line_detected(void)
{
    int     ret_error     = 0;
    uint8_t detected_data = 0;
    ret_error             = this->readReg(this->AICAMERA_ADDR, LINE_REG + 0, &detected_data, 1);
    if (ret_error != 0) { return ret_error; }
    return detected_data;
}

// 获取指定位置的行位置信息；获取线条位置信息（上/中/下 + X/Y/W/H）
int hw_aiCamera_c::line_position(int region, int info)
{
    DATA_CHECK(region, 1, 3);
    DATA_CHECK(info, AXIS_X, AXIS_H);
    int     ret_error        = 0;
    uint8_t position_data[8] = {0};
    ret_error = this->readReg(this->AICAMERA_ADDR, LINE_REG + region, position_data, 8);
    if (ret_error != 0) { return ret_error; }
    return (position_data[info * 2] << 8) | position_data[info * 2 + 1];
}

// 已识别物体的数量 （20类物体）
int hw_aiCamera_c::object_count(void)
{
    int     ret_error  = 0;
    uint8_t count_data = 0;
    ret_error          = this->readReg(this->AICAMERA_ADDR, OBJECT_REG + 0, &count_data, 1);
    if (ret_error != 0) { return ret_error; }
    return count_data;
}

// 识别出指定对象了吗？
int hw_aiCamera_c::object_detected(int object_class, int id)
{
    DATA_CHECK(object_class, 0, 19);
    DATA_CHECK(id, 1, 4);
    int     ret_error        = 0;
    uint8_t detected_data[9] = {0};
    ret_error = this->readReg(this->AICAMERA_ADDR, OBJECT_REG + id, detected_data, 9);
    if (ret_error != 0) { return ret_error; }
    return detected_data[0] == object_class ? 1 : 0;
}

// 已识别的物体位置信息 x,y,z,h 分别0~4 枚举	顺序1~4 可以不使用默认1
// 获取物体位置信息（X/Y/W/H）
int hw_aiCamera_c::object_position(int select, int id)
{
    DATA_CHECK(id, 1, 4);
    DATA_CHECK(select, 0, 3);
    int     ret_error        = 0;
    uint8_t position_data[9] = {0};
    ret_error = this->readReg(this->AICAMERA_ADDR, OBJECT_REG + id, position_data, 9);
    if (ret_error != 0) { return ret_error; }
    return (position_data[1 + select * 2] << 8) | position_data[1 + select * 2 + 1];
}

// 二维码被识别了吗？
int hw_aiCamera_c::qr_detected(void)
{
    int     ret_error     = 0;
    uint8_t detected_data = 0;
    ret_error             = this->readReg(this->AICAMERA_ADDR, QR_REG + 0, &detected_data, 1);
    if (ret_error != 0) { return ret_error; }
    return detected_data;
}

// 已识别的二维码内容
String hw_aiCamera_c::qr_data(void)
{
    uint8_t length    = 0;
    int     ret_error = this->readReg(this->AICAMERA_ADDR, QR_REG + 1, &length, 1);
    if (ret_error != 0) { return String(""); }
    uint8_t data[length];
    ret_error = this->readReg(this->AICAMERA_ADDR, QR_REG + 3, data, length);
    if (ret_error != 0) { return String(""); }
    return String((char *)data, length);
}

// 已识别的二维码位置信息 x,y,z,h 分别0~4 枚举
int hw_aiCamera_c::qr_position(int select)
{
    DATA_CHECK(select, 0, 3);
    int     ret_error        = 0;
    uint8_t position_data[8] = {0};
    ret_error                = this->readReg(this->AICAMERA_ADDR, QR_REG + 2, position_data, 8);
    if (ret_error != 0) { return ret_error; }
    return (position_data[select * 2] << 8) | position_data[select * 2 + 1];
}

// 检测到的人脸数量
int hw_aiCamera_c::face_count(void)
{
    int     ret_error  = 0;
    uint8_t count_data = 0;
    ret_error          = this->readReg(this->AICAMERA_ADDR, FACE_ATTRIBUTE_REG + 0, &count_data, 1);
    if (ret_error != 0) { return ret_error; }
    return count_data;
}

// 所选检测到的人脸的位置信息  x,y,z,h 分别0~4 枚举	人脸索引 1~4
int hw_aiCamera_c::face_position(int select, int id)
{
    DATA_CHECK(id, 1, 4);
    DATA_CHECK(select, 0, 3);
    int     ret_error        = 0;
    uint8_t position_data[8] = {0};
    ret_error = this->readReg(this->AICAMERA_ADDR, FACE_ATTRIBUTE_REG + id, position_data, 8);
    if (ret_error != 0) { return ret_error; }
    return (position_data[select * 2] << 8) | position_data[select * 2 + 1];
}

// 所选的面部是否符合指定条件 1~3  :1张嘴/2微笑/3戴眼镜	人脸索引 1~4
int hw_aiCamera_c::face_attribute(int attribute, int id)
{
    DATA_CHECK(id, 1, 4);
    DATA_CHECK(attribute, 1, 3);
    int     ret_error         = 0;
    uint8_t attribute_data[4] = {0};
    ret_error = this->readReg(this->AICAMERA_ADDR, FACE_ATTRIBUTE_REG + 4 + id, attribute_data, 4);
    if (ret_error != 0) { return ret_error; }
    return attribute_data[attribute];
}

// 学习当前的人脸
int hw_aiCamera_c::face_recognized_learn(void)
{
    int     ret_error    = 0;
    uint8_t command_data = 1;
    ret_error = this->writeReg(this->AICAMERA_ADDR, FACE_IDENTIFY_REG + 0, &command_data, 1);
    return ret_error;
}

// 已识别的人脸数量
int hw_aiCamera_c::face_recognized_count(void)
{
    int     ret_error  = 0;
    uint8_t count_data = 0;
    ret_error          = this->readReg(this->AICAMERA_ADDR, FACE_IDENTIFY_REG + 0, &count_data, 1);
    if (ret_error != 0) { return ret_error; }
    return count_data;
}

// 是否检测到了一张学习过的脸
int hw_aiCamera_c::face_recognized_detected(void)
{
    int     ret_error     = 0;
    uint8_t detected_data = 0;
    ret_error = this->readReg(this->AICAMERA_ADDR, FACE_IDENTIFY_REG + 1, &detected_data, 1);
    if (ret_error != 0) { return ret_error; }
    return detected_data;
}

// 所选识别面部的位置信息 x,y,z,h 分别0~4 枚举	人脸索引 1~4
int hw_aiCamera_c::face_recognized_position(int select, int id)
{
    DATA_CHECK(id, 1, 4);
    DATA_CHECK(select, 0, 3);
    int     ret_error        = 0;
    uint8_t position_data[9] = {0};
    ret_error = this->readReg(this->AICAMERA_ADDR, FACE_IDENTIFY_REG + 1 + id, position_data, 9);
    if (ret_error != 0) { return ret_error; }
    return (position_data[1 + select * 2] << 8) | position_data[1 + select * 2 + 1];
}

// 指定的类是否已被识别
int hw_aiCamera_c::class_recognized(int class_id)
{
    DATA_CHECK(class_id, 1, 2);
    int     ret_error     = 0;
    uint8_t detected_data = 0;
    ret_error = this->readReg(this->AICAMERA_ADDR, DEEPSTUDY_REG + 1, &detected_data, 1);
    if (ret_error != 0) { return ret_error; }
    if (0 == detected_data) return 0;

    ret_error = this->readReg(this->AICAMERA_ADDR, DEEPSTUDY_REG + 2, &detected_data, 1);
    if (ret_error != 0) { return ret_error; }
    return detected_data == class_id ? 1 : 0;
}

// 已识别的卡片数量
int hw_aiCamera_c::card_count(void)
{
    int     ret_error  = 0;
    uint8_t count_data = 0;
    ret_error          = this->readReg(this->AICAMERA_ADDR, CARD_REG + 0, &count_data, 1);
    if (ret_error != 0) { return ret_error; }
    return count_data;
}

// 指定的这张卡是否已被识别？
int hw_aiCamera_c::card_detected(int type, int sel, int id)
{
    static const num_map_t color_map = {
        // clang-format off
        {hw_aiCamera_c::GREEN,       0},
        {hw_aiCamera_c::RED,         3},
        // clang-format on
    };

    static const num_map_t action_map = {
        // clang-format off
        {hw_aiCamera_c::TURN_LEFT,   1},
        {hw_aiCamera_c::STOP_MOVING, 2},
        
        {hw_aiCamera_c::TURN_RIGHT,  4},
        {hw_aiCamera_c::EVENT_HONK,  5},
        {hw_aiCamera_c::EVENT_TARGET,6},
        // clang-format on
    };

    DATA_CHECK(sel, 1, 2);
    DATA_CHECK(id, 1, 4);

    uint8_t data = 0;

    if (1 == sel)
    {
        auto it = color_map.find(type);
        if (it == color_map.end()) return -1;
        data = it->second;
    } else if (2 == sel)
    {
        auto it = action_map.find(type);
        if (it == action_map.end()) return -1;
        data = it->second;
    }

    int     ret_error        = 0;
    uint8_t detected_data[9] = {0};
    ret_error                = this->readReg(this->AICAMERA_ADDR, CARD_REG + id, detected_data, 9);
    if (ret_error != 0) { return ret_error; }
    return detected_data[0] == data ? 1 : 0;
}

// 识别出的卡片的位置信息
int hw_aiCamera_c::card_position(int select, int id)
{
    DATA_CHECK(id, 1, 4);
    DATA_CHECK(select, 0, 3);
    int     ret_error        = 0;
    uint8_t position_data[9] = {0};
    ret_error                = this->readReg(this->AICAMERA_ADDR, CARD_REG + id, position_data, 9);
    if (ret_error != 0) { return ret_error; }
    return (position_data[1 + select * 2] << 8) | position_data[1 + select * 2 + 1];
}

// 当前状态是否与指定状态相符 0：ai未启动、1：连接中、2：待命、3：聆听中、4：说话中、5：配网中
int hw_aiCamera_c::state_is(int state)
{
    DATA_CHECK(state, 0, 5);
    int     ret_error  = 0;
    uint8_t state_data = 0;
    ret_error          = this->readReg(this->AICAMERA_ADDR, ESP_REG + 4, &state_data, 1);
    if (ret_error != 0) { return ret_error; }
    return state_data == state ? 1 : 0;
}

// 是否检测到了指定的运动指令 0：前进、1：后退、2：左转、3：右转、4：停止
int hw_aiCamera_c::motion_command_detected(int command)
{
    DATA_CHECK(command, 0, 4);
    int     ret_error       = 0;
    uint8_t command_data[2] = {0};
    ret_error               = this->readReg(this->AICAMERA_ADDR, ESP_REG + 5, command_data, 2);
    if (ret_error != 0) { return ret_error; }
    return command_data[0] == command ? 1 : 0;
}

// 检测到的运动速度值
int hw_aiCamera_c::motion_speed(void)
{
    int     ret_error     = 0;
    uint8_t speed_data[2] = {0};
    ret_error             = this->readReg(this->AICAMERA_ADDR, ESP_REG + 5, speed_data, 2);
    if (ret_error != 0) { return ret_error; }
    return speed_data[1];
}

// 检测到自定义命令
int hw_aiCamera_c::custom_command(void)
{
    int     ret_error    = 0;
    uint8_t command_data = 0;
    ret_error            = this->readReg(this->AICAMERA_ADDR, ESP_REG + 6, &command_data, 1);
    if (ret_error != 0) { return ret_error; }
    return command_data;
}

// 获取操纵杆位置 坐标枚举
int hw_aiCamera_c::joystick_position(int select)
{
    DATA_CHECK(select, 0, 1);
    int     ret_error        = 0;
    uint8_t position_data[2] = {0};
    ret_error                = this->readReg(this->AICAMERA_ADDR, ESP_REG + 7, position_data, 2);
    if (ret_error != 0) { return ret_error; }
    return (int)position_data[select];
}

// bool 按键状态 1~6
int hw_aiCamera_c::button_pressed(int button)
{
    DATA_CHECK(button, 1, 6);
    int     ret_error   = 0;
    uint8_t button_data = 0;
    ret_error           = this->readReg(this->AICAMERA_ADDR, ESP_REG + 8, &button_data, 1);
    if (ret_error != 0) { return ret_error; }
    return (button_data >> (6 - button)) & 0x01;
}

// 是否按下了指定的键盘按键 1~4 wasd
int hw_aiCamera_c::key_pressed(int key)
{
    DATA_CHECK(key, 1, 4);
    int     ret_error = 0;
    uint8_t key_data  = 0;
    ret_error         = this->readReg(this->AICAMERA_ADDR, ESP_REG + 9, &key_data, 1);
    if (ret_error != 0) { return ret_error; }
    return (key_data >> (4 - key)) & 0x01;
}

// （1开，0关） 可以使用设置补光亮度
int hw_aiCamera_c::fill_light(int level)
{
    DATA_CHECK(level, 0, 10);
    int     ret_error  = 0;
    uint8_t level_data = (uint8_t)level;
    if (0 == level_data)
    {
        ret_error = this->writeReg(this->AICAMERA_ADDR, SETTING_REG + 1, &level_data, 1);
    } else
    {
        uint8_t light_state = 1;
        ret_error           = this->writeReg(this->AICAMERA_ADDR, SETTING_REG + 1, &light_state, 1);
        ret_error += this->writeReg(this->AICAMERA_ADDR, SETTING_REG + 0, &level_data, 1);
    }
    return ret_error;
}

// 设置补光亮度 补光灯亮度（0-10) 0关闭
int hw_aiCamera_c::set_fill_light_brightness(int brightness)
{
    DATA_CHECK(brightness, 0, 10);
    int     ret_error  = 0;
    uint8_t level_data = (uint8_t)brightness;
    ret_error          = this->writeReg(this->AICAMERA_ADDR, SETTING_REG + 0, &level_data, 1);
    return ret_error;
}

// 获取当前补光亮度
int hw_aiCamera_c::get_fill_light_brightness(void)
{
    int     ret_error       = 0;
    uint8_t brightness_data = 0;
    ret_error = this->readReg(this->AICAMERA_ADDR, SETTING_REG + 0, &brightness_data, 1);
    if (ret_error != 0) { return ret_error; }
    return (int)brightness_data;
}
