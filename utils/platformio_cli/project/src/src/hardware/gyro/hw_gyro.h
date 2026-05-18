/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-03-18 16:21:24
 * @LastEditTime : 2026-03-18 16:49:23
 * @Description  : 陀螺仪
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_GYRO_H__
#define __HW_GYRO_H__

#include "main.h"

class hw_gyro_c
{
public:
    hw_gyro_c();
    ~hw_gyro_c();

public:
    void begin(void);

    float readFloatAccelX(void);
    float readFloatAccelY(void);
    float readFloatAccelZ(void);

    float readFloatGyroX(void);
    float readFloatGyroY(void);
    float readFloatGyroZ(void);

    float readTempC(void);
    float readTempF(void);
};

extern hw_gyro_c hw_gyro;

#endif /* __HW_GYRO_H__ */
