/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-20 23:06:05
 * @LastEditTime : 2026-04-30 08:34:08
 * @Description  : 四路灰度
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hw_gray.h"
#include "../common/hw_common.h"
#include "map"


/******************
 * data struct
 *****************/
#define LOG_TAG "hw_gray"

typedef std::map<int, int> gray_color_t;


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/
static const gray_color_t identify_map = {
    // clang-format off
    {(int)hw_gray_c::LINE_SENSOR_COLOR_NONE, (int)0},  
    {(int)hw_gray_c::LINE_SENSOR_RED,        (int)1},  
    {(int)hw_gray_c::LINE_SENSOR_GREEN,      (int)2}, 
    {(int)hw_gray_c::LINE_SENSOR_BLUE,       (int)3},
    {(int)hw_gray_c::LINE_SENSOR_YELLOW,     (int)4}, 
    {(int)hw_gray_c::LINE_SENSOR_CYAN,       (int)5}, 
    {(int)hw_gray_c::LINE_SENSOR_PURPLE,     (int)6},
    {(int)hw_gray_c::LINE_SENSOR_BLACK,      (int)7},
    {(int)hw_gray_c::LINE_SENSOR_WHITE,      (int)8},
    // clang-format on
};

static const gray_color_t command_map = {
    // clang-format off
    {(int)hw_gray_c::LINE_SENSOR_COLOR_NONE,    (int)6},
    {(int)hw_gray_c::LINE_SENSOR_GRAY,          (int)4},
    {(int)hw_gray_c::LINE_SENSOR_RED,           (int)7},
    {(int)hw_gray_c::LINE_SENSOR_YELLOW,        (int)10},
    {(int)hw_gray_c::LINE_SENSOR_GREEN,         (int)8},
    {(int)hw_gray_c::LINE_SENSOR_CYAN,          (int)11},
    {(int)hw_gray_c::LINE_SENSOR_BLUE,          (int)9},
    {(int)hw_gray_c::LINE_SENSOR_PURPLE,        (int)12},
    {(int)hw_gray_c::LINE_SENSOR_BLACK,         (int)13},
    {(int)hw_gray_c::LINE_SENSOR_WHITE,         (int)14},
    // clang-format on
};

static const gray_color_t port_map = {
    {(int)hw_gray_c::LINE_SENSOR_PROBE_L2,  (int)0},
    {(int)hw_gray_c::LINE_SENSOR_PROBE_L1,  (int)1},
    {(int)hw_gray_c::LINE_SENSOR_PROBE_R1,  (int)2},
    {(int)hw_gray_c::LINE_SENSOR_PROBE_R2,  (int)3},
};

/********************
 * global variables
 *******************/
hw_gray_c hw_gray;

/********************
 * global functions
 *******************/


/*******************
 * class functions
 *******************/
hw_gray_c::hw_gray_c() : i2c_handle(if_i2c_internal_handle)
{}

hw_gray_c::~hw_gray_c()
{}

int hw_gray_c::write_bytes(uint8_t addr, uint8_t *buf, uint16_t len)
{
    return this->i2c_handle.write_bytes(addr, buf, len);
}

int hw_gray_c::read_bytes(uint8_t addr, uint8_t *buf, uint16_t len)
{
    return this->i2c_handle.read_bytes(addr, buf, len);
}


int hw_gray_c::set_i2c_port(uint8_t port)
{
    switch (port)
    {
        case 0: i2c_handle = if_i2c_internal_handle; break;
        case 1: i2c_handle = if_i2c_external_handle; break;
        default: return -1;
    }
    return 0;
}

int hw_gray_c::wait_mode_changed(uint8_t mode, int timeout_ms)
{
    int ret = 0;
    uint8_t data[5] = {0};
    uint32_t last_tick = zst_tick_get();
    while (1)
    {
        delay(20);
        if (timeout_ms >= 0 && zst_tick_elaps(last_tick) >= timeout_ms) return -1;
        ret += this->read_bytes(this->dev_addr, data, 5);
        if (0 != ret) return ret;
        if (mode == data[4]) break;
    }
    delay(20);
    return 0;
}

// 将线路跟踪传感器设置为学习模式，以便学习特定颜色的参考值。
int hw_gray_c::line_sensor_learn(int study)
{
    int ret = 0;
    uint8_t data;
    auto it = command_map.find(study);
    if (it == command_map.end())
        return -1;
    data = it->second;
    ret = this->write_bytes(this->dev_addr, (uint8_t *)&data, 1);
    return ret;
}


// 从探头获取灰度值
int hw_gray_c::line_sensor_gray(int port)
{
    int ret = 0;
    uint8_t data[5] = {0};
    uint8_t gray_mode = 2;
    auto port_it = port_map.find(port);
    if (port_it == port_map.end()) return -1;
    ret += this->write_bytes(this->dev_addr, &gray_mode, 1); // 设置为识别模式
    ret += this->read_bytes(this->dev_addr, data, 5);
    if (0 == ret && gray_mode != data[4])
        wait_mode_changed(gray_mode, 300);
    ret += this->read_bytes(this->dev_addr, data, 5);
    if (ret != 0)
        return ret;
    return data[port];
}


// 检查是否已学习到的颜色得到了识别
int hw_gray_c::line_sensor_color(int port, int color)
{
    int ret = 0;
    uint8_t data[5] = {0};
    uint8_t color_mode = 1;
    auto port_it = port_map.find(port);
    if (port_it == port_map.end()) return -1;

    auto color_it = identify_map.find(color);
    if (color_it == identify_map.end()) return -2;

    ret += this->write_bytes(this->dev_addr, &color_mode, 1); // 设置为识别模式
    ret += this->read_bytes(this->dev_addr, data, 5);
    if (0 == ret && color_mode != data[4])
        wait_mode_changed(color_mode, 300);
    ret += this->read_bytes(this->dev_addr, data, 5);
    if (ret != 0)
        return ret;
    
    if (data[port] != color_it->second)
        return false;
    return true;
}


// 检查是否检测到了黑色线条
int hw_gray_c::line_sensor_detect_line(int port)
{
    int ret = 0;
    uint8_t data[5] = {0};
    uint8_t binary_mode = 3;
    auto port_it = port_map.find(port);
    if (port_it == port_map.end()) return -1;

    ret += this->write_bytes(this->dev_addr, &binary_mode, 1); // 设置为识别模式
    ret += this->read_bytes(this->dev_addr, data, 5);
    if (ret != 0)
        return ret;
    if (0 == ret && binary_mode != data[4])
        wait_mode_changed(binary_mode, 300);
    ret += this->read_bytes(this->dev_addr, data, 5);
    if (data[port] == 1)
        return true;
    return false;
}


