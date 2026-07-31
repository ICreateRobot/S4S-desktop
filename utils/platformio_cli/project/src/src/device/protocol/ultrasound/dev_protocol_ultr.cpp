/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-12-03 16:39:00
 * @LastEditTime : 2026-05-20 14:19:01
 * @Description  : 超声波设备
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_ultr.h"
#include "interface/i2c/if_i2c.h"
#include "hardware/ultrasound/hw_colorful_ultr.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_ultr"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_set_i2c_port(udc_pack_t *pack);
static fmap_result_t fMap_ultrasonic_get_distance(udc_pack_t *pack);
static fmap_result_t fMap_ultrasonic_set_color(udc_pack_t *pack);


/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"set_i2c_port",             fMap_set_i2c_port},
    {"ultrasonic_get_distance",  fMap_ultrasonic_get_distance},
    {"ultrasonic_set_color",     fMap_ultrasonic_set_color}
    // clang-format on
};

static const enum_map_t enum_map = {
    // clang-format off
    {"ULTRASONIC_CM",   (int)hw_cultr.ULTRASONIC_CM},
    {"ULTRASONIC_INCH", (int)hw_cultr.ULTRASONIC_INCH},
    {"ULTRASONIC_M",    (int)hw_cultr.ULTRASONIC_M}
    // clang-format on
};

/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_ultr_init(void)
{
    function_map_collection_main["cultr"] = &function_map;
}


/****************************
 * static function
 ***************************/
static fmap_result_t fMap_set_i2c_port(udc_pack_t *pack)
{
    int ret = 0;
    int port;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &port);
    ret = hw_cultr.set_i2c_port(port);
    return fmap_result_t::make_result(ret);
}

static fmap_result_t fMap_ultrasonic_get_distance(udc_pack_t *pack)
{
    std::string unit;
    int         unit_int;
    float       distance;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &unit);
    enum_map_get_value(enum_map, unit.c_str(), unit_int);

    distance = hw_cultr.ultrasonic_get_distance(unit_int);
    return fmap_result_t::make_ok().add_float(distance);
}

static fmap_result_t fMap_ultrasonic_set_color(udc_pack_t *pack)
{
    int ret = 0;
    int red, green, blue, light=125;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &red);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &green);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &blue);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(3), &light);
    ret = hw_cultr.ultrasonic_set_color(red, green, blue, light);
    return fmap_result_t::make_result(ret);
}
