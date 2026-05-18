/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-03-18 16:21:16
 * @LastEditTime : 2026-03-18 16:41:30
 * @Description  : 陀螺仪
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hw_gyro.h"
#include "LSM6DS3.h"
#include "platform/pf_rtos.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "hw_gyro"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/
static LSM6DS3 myIMU(I2C_MODE, 0x6b);


/********************
 * global variables
 *******************/
hw_gyro_c hw_gyro;


/********************
 * global functions
 *******************/


/*******************
 * class functions
 *******************/
hw_gyro_c::hw_gyro_c()
{}

hw_gyro_c::~hw_gyro_c()
{}

void hw_gyro_c::begin(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    myIMU.begin();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

float hw_gyro_c::readFloatAccelX(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    float x = 0;
    x = myIMU.readFloatAccelX();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return x;
}

float hw_gyro_c::readFloatAccelY(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    float y = 0;
    y = myIMU.readFloatAccelY();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return y;
}

float hw_gyro_c::readFloatAccelZ(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    float z = 0;
    z = myIMU.readFloatAccelZ();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return z;
}

float hw_gyro_c::readFloatGyroX(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    float x = 0;
    x = myIMU.readFloatGyroX();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return x;
}

float hw_gyro_c::readFloatGyroY(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    float y = 0;
    y = myIMU.readFloatGyroY();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return y;
}

float hw_gyro_c::readFloatGyroZ(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    float z = 0;
    z = myIMU.readFloatGyroZ();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return z;
}

float hw_gyro_c::readTempC(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    float temp = 0;
    temp = myIMU.readTempC();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return temp;
}

float hw_gyro_c::readTempF(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    float temp = 0;
    temp = myIMU.readTempF();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
    
    return temp;
}
