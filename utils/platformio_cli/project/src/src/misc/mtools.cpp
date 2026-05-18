/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-12-03 20:02:26
 * @LastEditTime : 2026-01-21 00:17:22
 * @Description  : 工具函数
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "mtools.h"
#include <ctype.h>

/******************
 * data struct
 *****************/
#define LOG_TAG "mtools"


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
//  判断数据是否为可打印字符
int all_printable(uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        if (!isprint(data[i])) { return -1; }
    }
    return 0;
}


/****************************
 * static function
 ***************************/
