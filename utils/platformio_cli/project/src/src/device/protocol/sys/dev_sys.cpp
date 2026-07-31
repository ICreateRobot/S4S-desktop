/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-03-11 15:33:18
 * @LastEditTime : 2026-04-29 14:10:59
 * @Description  : 系统设备
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_sys.h"
#include "hardware/sys/hw_sys.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_sys"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_version(udc_pack_t *pack);
static fmap_result_t fMap_tick_get(udc_pack_t *pack);
static fmap_result_t fMap_tick_reset(udc_pack_t *pack);


/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"version",         fMap_version},
    {"tick_get",        fMap_tick_get},
    {"tick_reset",      fMap_tick_reset}
    // clang-format on
};

/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_sys_init(void)
{
    function_map_collection_main["sys"] = &function_map;
}

/****************************
 * static function
 ***************************/
static fmap_result_t fMap_version(udc_pack_t *pack)
{
    return fmap_result_t::make_ok().add_string(hw_sys.version().c_str());
}

static fmap_result_t fMap_tick_get(udc_pack_t *pack)
{
    return fmap_result_t::make_ok().add_int(hw_sys.tick_get());
}

static fmap_result_t fMap_tick_reset(udc_pack_t *pack)
{
    hw_sys.tick_reset();
    return fmap_result_t::make_ok();
}
