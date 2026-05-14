/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-28 14:00:47
 * @LastEditTime : 2026-04-25 17:23:06
 * @Description  : 炫彩超声波 (colorful_ultr)
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hw_colorful_ultr.h"

/***
 * 炫彩超声波为兼容M5的协议，为方便交互式编写，将协议简化。
 * | 操作   | 字节数量  | 意义                 |
 * | ------ | -------- | --------------------|
 * | 读操作 | 2        | 距离                 |
 * | 写操作 | 4        | 颜色（亮度、R、G、B） |
 */

/******************
 * data struct
 *****************/
#define LOG_TAG "hw_colorful_ultr"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/


/********************
 * class functions
 *******************/
hw_cultr_c::hw_cultr_c() : i2c_handle(if_i2c_internal_handle)
{}


hw_cultr_c::~hw_cultr_c()
{}

int hw_cultr_c::write_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return this->i2c_handle.write_reg(this->dev_addr, reg, buf, len);
}

int hw_cultr_c::read_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    return this->i2c_handle.read_reg(this->dev_addr, reg, buf, len);
}

int hw_cultr_c::set_i2c_port(uint8_t port)
{
    switch (port)
    {
        case 0: i2c_handle = if_i2c_internal_handle; break;
        case 1: i2c_handle = if_i2c_external_handle; break;
        default: return -1;
    }
    return 0;
}

float hw_cultr_c::get_distance(void)
{
    float   distance  = 0;
    uint8_t buffer[3] = {0};
    this->read_reg(this->dev_addr, 0x01, buffer, 3);
    distance = ((uint32_t)buffer[0] << 16) | ((uint32_t)buffer[1] << 8) | (uint32_t)buffer[2];
    distance /= 10000.0;
    return distance;
}

float hw_cultr_c::get_distance(int unit)
{
    float distance = this->get_distance();
    switch (unit)
    {
        case ULTRASONIC_CM: distance = distance; break;
        case ULTRASONIC_INCH: distance = distance * 0.3937; break;
        case ULTRASONIC_M: distance = distance / 100; break;
        default: return -1;
    }
    return distance;
}

/**
 * @description: 设置颜色
 * @param {uint8_t} red   0-255
 * @param {uint8_t} green 0-255
 * @param {uint8_t} blue  0-255
 * @param {uint8_t} light 0-255
 * @return {*}
 */
int hw_cultr_c::set_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t light)
{
    int     ret       = 0;
    uint8_t buffer[4] = {light, red, green, blue};
    ret               = this->write_reg(this->dev_addr, 0x02, buffer, sizeof(buffer));
    return ret;
}


hw_cultr_c hw_cultr;
