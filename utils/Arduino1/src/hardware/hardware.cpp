/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-28 15:05:54
 * @LastEditTime : 2026-04-27 20:54:25
 * @Description  : 硬件设备
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hardware.h"
#include "esp_audio/hw_esp_audio.h"
#include "esp_oled/hw_esp_oled.h"
#include "ultrasound/hw_colorful_ultr.h"
#include "gyro/hw_gyro.h"
#include "magnetometer/hw_magnetometer.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "hardware"


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
void hardware_init(void)
{
    hw_esp_oled.begin();
    hw_esp_audio.begin();
    hw_gyro.begin();
    hw_magnetometer.begin();
}


/****************************
 * static function
 ***************************/
