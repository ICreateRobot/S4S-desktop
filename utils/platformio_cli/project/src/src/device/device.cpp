/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 14:51:00
 * @LastEditTime : 2026-04-17 14:20:36
 * @Description  : 设备初始化和操作
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "device.h"
#include "protocol/dev_protocol.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "device"


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
void device_init(void)
{
    /*****************
     * 设备初始化
     *****************/
    dev_protocol_init();
}


/****************************
 * static function
 ***************************/
