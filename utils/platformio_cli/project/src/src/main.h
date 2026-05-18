/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 14:08:14
 * @LastEditTime : 2026-04-28 14:26:15
 * @Description  : main.h 文件
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __MAIN_H__
#define __MAIN_H__

#include "FreeRTOSConfig.h"
#include "Arduino_FreeRTOS.h"

#include "Arduino.h"
#include "zst_core.h"
#include "udc.h"


typedef struct {
    ptask_root_t *ptask_root_1;
} ptask_root_1_collection_t;

typedef struct {
    ptask_t *test;
} ptask_1_collection_t;

// 板载按键
#define BOARD_BUTTON_A_PIN "D2"
#define BOARD_BUTTON_B_PIN "D3"

// 版本
#define VERSION_0 0
#define VERSION_1 1
#define VERSION_2 3

extern ptask_root_1_collection_t ptask_root_1_collection;
extern ptask_1_collection_t      ptask_1_collection;

extern udc_pack_group_t udcPackGroup_main;
extern udc_pack_t udcPack_main;

extern udc_pack_group_t udcPackGroup_sys;
extern udc_pack_t udcPack_sys;


#endif /* __MAIN_H__ */
