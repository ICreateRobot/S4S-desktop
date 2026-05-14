/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Description  : 流体模拟 OLED 渲染（128×128 SH1107）
 *                 陀螺仪加速度 → 重力方向，粒子随设备倾斜流动
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __APP_FLUID_OLED_H__
#define __APP_FLUID_OLED_H__

#include "main.h"

/**
 * @description: 初始化流体模拟（创建粒子、分配内存）
 *               在 setup() 中调用，需在 hw_gyro.begin() 之后
 */
void app_fluid_oled_begin(void);

/**
 * @description: 推进一帧模拟并刷新 OLED
 *               在 loop() 中调用，建议间隔 ≥ 20ms
 */
void app_fluid_oled_update(void);

/**
 * @description: 重置粒子到初始堆积状态
 */
void app_fluid_oled_reset(void);

/**
 * @description: 销毁模拟实例，释放内存
 */
void app_fluid_oled_destroy(void);

#endif /* __APP_FLUID_OLED_H__ */