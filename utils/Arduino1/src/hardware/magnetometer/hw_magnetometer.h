/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-03-18 16:39:48
 * @LastEditTime : 2026-03-18 17:07:43
 * @Description  : 磁力计
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_MAGNETOMETER_H__
#define __HW_MAGNETOMETER_H__

#include "main.h"
#include "interface/i2c/if_i2c.h"


class hw_magnetometer_c
{
public:
    hw_magnetometer_c();
    ~hw_magnetometer_c();

public:
    void begin(void);

    int getRawMagneticX(void);
    int getRawMagneticY(void);
    int getRawMagneticZ(void);

    float getGaussFieldX(void);
    float getGaussFieldY(void);
    float getGaussFieldZ(void);

private:
    int16_t RawMagneticX;
    int16_t RawMagneticY;
    int16_t RawMagneticZ;

    float GaussFieldX;
    float GaussFieldY;
    float GaussFieldZ;

    if_i2c &i2c_handle;
};

extern hw_magnetometer_c hw_magnetometer;

#endif /* __HW_MAGNETOMETER_H__ */
