/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 15:19:02
 * @LastEditTime : 2026-04-17 10:05:52
 * @Description  : 与外部库的接口
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "interface.h"
#include "uart/if_uart.h"
#include "i2c/if_i2c.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "interface"


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
void interface_init(void)
{
    if_uart_init();
    if_i2c_init();
}


/****************************
 * static function
 ***************************/
