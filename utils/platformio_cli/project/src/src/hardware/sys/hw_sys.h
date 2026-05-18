/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-03-20 14:37:15
 * @LastEditTime : 2026-03-20 14:41:32
 * @Description  : 系统函数
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_SYS_H__
#define __HW_SYS_H__


#include "main.h"

class hw_sys_c
{
public:
    hw_sys_c();
    ~hw_sys_c();

public:
    void        version(uint8_t version[3]);
    String      version(void);

    uint32_t tick_get(void);
    void     tick_reset(void);

private:
    uint32_t last_tick = 0;
};

extern hw_sys_c hw_sys;

#endif /* __HW_SYS_H__ */
