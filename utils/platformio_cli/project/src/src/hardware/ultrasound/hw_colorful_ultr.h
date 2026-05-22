/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-28 14:00:58
 * @LastEditTime : 2026-05-20 14:10:46
 * @Description  : 炫彩超声波 (colorful_ultr)
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_ULTR_H__
#define __HW_ULTR_H__

#include "main.h"
#include "interface/i2c/if_i2c.h"

class hw_cultr_c
{
public:
    hw_cultr_c(void);
    ~hw_cultr_c(void);

    enum
    {
        ULTRASONIC_CM = 0, // 厘米
        ULTRASONIC_INCH,   // 英寸
        ULTRASONIC_M,      // 米
    };

public:
    int set_i2c_port(uint8_t port);
    int ultrasonic_set_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t light = 125);
    float get_distance(void);
    float ultrasonic_get_distance(int unit);

protected:
    virtual int write_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
    virtual int read_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);

private:
    const uint8_t dev_addr = 0x57;
    if_i2c       &i2c_handle;
};

extern hw_cultr_c hw_cultr;

#endif /* __HW_ULTR_H__ */
