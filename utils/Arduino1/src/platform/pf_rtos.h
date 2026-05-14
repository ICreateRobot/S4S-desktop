/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-04-23 10:38:19
 * @LastEditTime : 2026-04-30 15:24:13
 * @Description  : 根据不同平台，实现不同平台下的线程、信号量等操作
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __PF_RTOS_H__
#define __PF_RTOS_H__

#include "main.h"

enum class pf_rtos_binary_semaphore
{
    I2C=0,
    UART,
    
    MAX,
};


void pf_rtos_init(void);
int pf_rtos_semaphore_take(pf_rtos_binary_semaphore semaphore, int timeout_ms);
int pf_rtos_semaphore_give(pf_rtos_binary_semaphore semaphore);

#endif /* __PF_RTOS_H__ */
