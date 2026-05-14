/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-20 23:36:26
 * @LastEditTime : 2026-05-07 10:20:07
 * @Description  : 硬件设备公共头文件
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_COMMON_H__
#define __HW_COMMON_H__

#include "main.h"


#define DATA_CHECK(data, min, max)                                                                 \
    do                                                                                             \
    {                                                                                              \
        if (data < min || data > max)                                                              \
        {                                                                                          \
            ZST_LOGE(LOG_TAG, "data:(%d) out of scope (%d, %d)", data, min, max);                  \
            return -1;                                                                             \
        }                                                                                          \
    } while (0)

#endif /* __HW_COMMON_H__ */
