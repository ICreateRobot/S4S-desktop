/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-31 15:14:22
 * @LastEditTime : 2026-05-05 10:18:14
 * @Description  : 引脚控制
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_PIN_H__
#define __HW_PIN_H__

#include "main.h"


class hw_pin_c
{
public:
    hw_pin_c();
    ~hw_pin_c();

public:
    // 设置引脚模式 0:输入 1:输出 2:输入上拉 3:输入下拉 4 开漏输出
    int pinMode(const char *pin, int mode);

    // 设置引脚输出电平  0:低电平 1:高电平
    int digitalWrite(const char *pin, int value);

    // 读取引脚电平  0:低电平 1:高电平
    int digitalRead(const char *pin);

    // 设置引脚PWM输出频率和占空比  value:占空比 0~255
    int analogWrite(const char *pin, int value);

    // 读取引脚模拟值  0~1023
    int analogRead(const char *pin);

    // 读取引脚脉冲宽度  timeout:超时时间 单位ms
    int pulseIn(const char *pin, int state, int timeout);

    // 板载按键是否按下; 0未按下、1A按键按下、2B按键按下、3AB按键按下
    int button_pressed(void);
};

extern hw_pin_c hw_pin;

#endif /* __HW_PIN_H__ */
