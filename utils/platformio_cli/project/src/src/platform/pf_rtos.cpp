/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-04-23 10:38:00
 * @LastEditTime : 2026-06-11 11:47:28
 * @Description  : 根据不同平台，实现不同平台下的线程、信号量等操作
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "pf_rtos.h"

/******************
 * data struct
 *****************/
#define LOG_TAG "pf_rtos"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/
// 二值信号量
static SemaphoreHandle_t xBinarySemaphore_array[(uint32_t)pf_rtos_binary_semaphore::MAX];


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void pf_rtos_init(void)
{
    /*******************
     * 二值信号量初始化
     ******************/
    for (uint16_t i = 0; i < (uint16_t)pf_rtos_binary_semaphore::MAX; i++)
    {
        xBinarySemaphore_array[i] = xSemaphoreCreateBinary();
        if (NULL == xBinarySemaphore_array[i])
        {
            ZST_LOGE(LOG_TAG, "BinarySemaphore create failed");
        } else
        {
            ZST_LOGI(LOG_TAG, "BinarySemaphore create success");
            xSemaphoreGive(xBinarySemaphore_array[i]);
        }
    }
}

/**
 * @description: 获取二值信号量
 * @param semaphore 信号量枚举值
 * @param timeout_ms 超时时间，单位毫秒
 * @return true 获取成功，false 获取失败
 */
int pf_rtos_semaphore_take(pf_rtos_binary_semaphore semaphore, int timeout_ms)
{
    BaseType_t xReturn;
    TickType_t xTicksToWait = 0;
    if (timeout_ms >= 0)
        xTicksToWait = pdMS_TO_TICKS(timeout_ms);
    else
        xTicksToWait = portMAX_DELAY;
    xReturn = xSemaphoreTake(xBinarySemaphore_array[(uint32_t)semaphore], xTicksToWait);
    if (xReturn == pdTRUE)
        return true;
    else
        return false;
}

/**
 * @description: 释放二值信号量
 * @param semaphore 信号量枚举值
 * @return true 释放成功，false 释放失败
 */
int pf_rtos_semaphore_give(pf_rtos_binary_semaphore semaphore)
{
    if (xSemaphoreGive(xBinarySemaphore_array[(uint32_t)semaphore]) == pdTRUE)
        return true;
    else
        return false;
}


/****************************
 * static function
 ***************************/
