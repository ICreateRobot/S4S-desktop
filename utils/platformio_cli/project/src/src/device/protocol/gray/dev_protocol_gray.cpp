/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-30 20:20:52
 * @LastEditTime : 2026-06-29 13:31:56
 * @Description  : 灰度传感器
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_gray.h"
#include "hardware/gray/hw_gray.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_gray"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_set_i2c_port(udc_pack_t *pack);
static fmap_result_t fMap_line_sensor_learn(udc_pack_t *pack);
static fmap_result_t fMap_line_sensor_gray(udc_pack_t *pack);
static fmap_result_t fMap_line_sensor_color(udc_pack_t *pack);
static fmap_result_t fMap_line_sensor_detect_line(udc_pack_t *pack);

/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"set_i2c_port",            fMap_set_i2c_port},
    {"line_sensor_learn",       fMap_line_sensor_learn},
    {"line_sensor_gray",        fMap_line_sensor_gray},
    {"line_sensor_color",       fMap_line_sensor_color},
    {"line_sensor_detect_line", fMap_line_sensor_detect_line}
    // clang-format on
};


static const enum_map_t enum_map = {
    // clang-format off
    {"LINE_SENSOR_GRAY",         (int)hw_gray_c::LINE_SENSOR_GRAY},
    {"LINE_SENSOR_COLOR_NONE",   (int)hw_gray_c::LINE_SENSOR_COLOR_NONE},
    {"LINE_SENSOR_RED",          (int)hw_gray_c::LINE_SENSOR_RED},
    {"LINE_SENSOR_YELLOW",       (int)hw_gray_c::LINE_SENSOR_YELLOW},
    {"LINE_SENSOR_GREEN",        (int)hw_gray_c::LINE_SENSOR_GREEN},
    {"LINE_SENSOR_CYAN",         (int)hw_gray_c::LINE_SENSOR_CYAN},
    {"LINE_SENSOR_BLUE",         (int)hw_gray_c::LINE_SENSOR_BLUE},
    {"LINE_SENSOR_PURPLE",       (int)hw_gray_c::LINE_SENSOR_PURPLE},
    {"LINE_SENSOR_BLACK",        (int)hw_gray_c::LINE_SENSOR_BLACK},
    {"LINE_SENSOR_WHITE",        (int)hw_gray_c::LINE_SENSOR_WHITE},

    {"LINE_SENSOR_PROBE_L2",    (int)hw_gray_c::LINE_SENSOR_PROBE_L2},
    {"LINE_SENSOR_PROBE_L1",    (int)hw_gray_c::LINE_SENSOR_PROBE_L1},
    {"LINE_SENSOR_PROBE_R1",    (int)hw_gray_c::LINE_SENSOR_PROBE_R1},
    {"LINE_SENSOR_PROBE_R2",    (int)hw_gray_c::LINE_SENSOR_PROBE_R2}
    // clang-format on
};

/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_gray_init(void)
{
    function_map_collection["gray"] = &function_map;
}

/****************************
 * static function
 ***************************/
static fmap_result_t fMap_set_i2c_port(udc_pack_t *pack)
{
    int ret = 0;
    int port;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &port);
    ret = hw_gray.set_i2c_port(port);
    
    return fmap_result_t::make_result(ret);
}

static fmap_result_t fMap_line_sensor_learn(udc_pack_t *pack)
{
    int         ret_error = 0;
    std::string study;
    int         study_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &study);
    enum_map_get_value(enum_map, study.c_str(), study_int);
    ret_error = hw_gray.line_sensor_learn(study_int);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_line_sensor_gray(udc_pack_t *pack)
{
    int         ret_error = 0;
    std::string port;
    int         port_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &port);
    enum_map_get_value(enum_map, port.c_str(), port_int);
    ret_error = hw_gray.line_sensor_gray(port_int);

    if (ret_error < 0)
        return fmap_result_t::make_result(ret_error);
    else
        return fmap_result_t::make_ok().add_int(ret_error);
}

static fmap_result_t fMap_line_sensor_color(udc_pack_t *pack)
{
    int         ret_error = 0;
    std::string port, color;
    int         port_int, color_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &port);
    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(1), &color);
    enum_map_get_value(enum_map, port.c_str(), port_int);
    enum_map_get_value(enum_map, color.c_str(), color_int);
    ret_error = hw_gray.line_sensor_color(port_int, color_int);

    if (ret_error < 0)
        return fmap_result_t::make_error(ret_error);
    else
        return fmap_result_t::make_ok().add_int(ret_error);
}

static fmap_result_t fMap_line_sensor_detect_line(udc_pack_t *pack)
{
    int         ret_error = 0;
    std::string port;
    int         port_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &port);
    enum_map_get_value(enum_map, port.c_str(), port_int);
    ret_error = hw_gray.line_sensor_detect_line(port_int);

    if (ret_error < 0)
        return fmap_result_t::make_result(ret_error);
    else
        return fmap_result_t::make_ok().add_int(ret_error);
}
