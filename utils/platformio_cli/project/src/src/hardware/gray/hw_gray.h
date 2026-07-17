/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-20 23:06:10
 * @LastEditTime : 2026-06-29 12:01:07
 * @Description  : 四路灰度
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_GRAY_H__
#define __HW_GRAY_H__

#include "main.h"
#include "interface/i2c/if_i2c.h"


class hw_gray_c
{
public:
    hw_gray_c();
    ~hw_gray_c();

    enum
    {
        LINE_SENSOR_GRAY = 0,   // 灰度学习
        LINE_SENSOR_COLOR_NONE, // 清空颜色学习/未检测到颜色
        LINE_SENSOR_RED,
        LINE_SENSOR_YELLOW,
        LINE_SENSOR_GREEN,
        LINE_SENSOR_CYAN,
        LINE_SENSOR_BLUE,
        LINE_SENSOR_PURPLE,
        LINE_SENSOR_BLACK,
        LINE_SENSOR_WHITE,

        LINE_SENSOR_PROBE_L2 = 0, // 最左侧探头
        LINE_SENSOR_PROBE_L1,
        LINE_SENSOR_PROBE_R1,
        LINE_SENSOR_PROBE_R2
    };

public:
    int set_i2c_port(uint8_t port);
    int wait_mode_changed(uint8_t mode, int timeout_ms);

    // 将线路跟踪传感器设置为学习模式，以便学习特定颜色的参考值。
    int line_sensor_learn(int study);
    // 从探头获取灰度值
    int line_sensor_gray(int port);
    // 检查是否已学习到的颜色得到了识别
    int line_sensor_color(int port, int color);
    // 检查是否检测到了黑色线条
    int line_sensor_detect_line(int port);


protected:
    virtual int write_bytes(uint8_t addr, uint8_t *buf, uint16_t len);
    virtual int read_bytes(uint8_t addr, uint8_t *buf, uint16_t len);

private:
    const uint8_t MODE_NONE           = 0;
    const uint8_t MODE_COLOR          = 1;
    const uint8_t MODE_GRAY           = 2;
    const uint8_t MODE_BIN            = 3;
    const uint8_t MODE_PHOTOSENSITIVE = 15;

    uint8_t dev_addr  = 0x6F;
    uint8_t dev_addr2 = 0x71;
    if_i2c &i2c_handle;
};


extern hw_gray_c hw_gray;

#endif /* __GRAY_H__ */
