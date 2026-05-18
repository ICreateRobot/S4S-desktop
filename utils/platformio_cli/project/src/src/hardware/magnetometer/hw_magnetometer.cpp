/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-03-18 16:39:27
 * @LastEditTime : 2026-04-24 10:54:36
 * @Description  : 磁力计
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hw_magnetometer.h"
#include "Adafruit_QMC5883P.h"
#include "platform/pf_rtos.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "hw_magnetometer"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/
static Adafruit_QMC5883P qmc;


/********************
 * global variables
 *******************/
hw_magnetometer_c hw_magnetometer;


/********************
 * global functions
 *******************/


/*******************
 * class functions
 *******************/
hw_magnetometer_c::hw_magnetometer_c() : i2c_handle(if_i2c_internal_handle)
{}

hw_magnetometer_c::~hw_magnetometer_c()
{}

void hw_magnetometer_c::begin(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    qmc.begin(QMC5883P_DEFAULT_ADDR, i2c_handle.get_wire());
    qmc.setMode(QMC5883P_MODE_NORMAL);         // 设置为正常模式
    qmc.setODR(QMC5883P_ODR_50HZ);             // 设置输出数据速率 (ODR) 为 50Hz
    qmc.setOSR(QMC5883P_OSR_4);                // 设置超采样比率 (OSR) 为 4
    qmc.setDSR(QMC5883P_DSR_2);                // 设置下采样比率 (DSR) 为 2
    qmc.setRange(QMC5883P_RANGE_8G);           // 设置量程为 8G
    qmc.setSetResetMode(QMC5883P_SETRESET_ON); // 设置建立/复位模式为开启

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

int hw_magnetometer_c::getRawMagneticX(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    if (qmc.isDataReady()) { qmc.getRawMagnetic(&RawMagneticX, &RawMagneticY, &RawMagneticZ); }

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return RawMagneticX;
}

int hw_magnetometer_c::getRawMagneticY(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    if (qmc.isDataReady()) { qmc.getRawMagnetic(&RawMagneticX, &RawMagneticY, &RawMagneticZ); }

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return RawMagneticY;
}

int hw_magnetometer_c::getRawMagneticZ(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    if (qmc.isDataReady()) { qmc.getRawMagnetic(&RawMagneticX, &RawMagneticY, &RawMagneticZ); }

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return RawMagneticZ;
}

float hw_magnetometer_c::getGaussFieldX(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    if (qmc.isDataReady()) { qmc.getGaussField(&GaussFieldX, &GaussFieldY, &GaussFieldZ); }

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return GaussFieldX;
}

float hw_magnetometer_c::getGaussFieldY(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    if (qmc.isDataReady()) { qmc.getGaussField(&GaussFieldX, &GaussFieldY, &GaussFieldZ); }

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return GaussFieldY;
}

float hw_magnetometer_c::getGaussFieldZ(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    if (qmc.isDataReady()) { qmc.getGaussField(&GaussFieldX, &GaussFieldY, &GaussFieldZ); }

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
    
    return GaussFieldZ;
}
