/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-13 11:17:08
 * @LastEditTime : 2026-07-03 10:57:11
 * @Description  : esp 引脚控制
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_pin.h"
#include "hardware/pin/hw_pin.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_pin"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_pinMode(udc_pack_t *pack);
static fmap_result_t fMap_digitalWrite(udc_pack_t *pack);
static fmap_result_t fMap_digitalRead(udc_pack_t *pack);
static fmap_result_t fMap_analogWrite(udc_pack_t *pack);
static fmap_result_t fMap_analogRead(udc_pack_t *pack);
static fmap_result_t fMap_pulseIn(udc_pack_t *pack);
static fmap_result_t fMap_button_pressed(udc_pack_t *pack);


/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"pinMode",         fMap_pinMode},
    {"digitalWrite",    fMap_digitalWrite},
    {"digitalRead",     fMap_digitalRead},
    {"analogWrite",     fMap_analogWrite},
    {"analogRead",      fMap_analogRead},
    {"pulseIn",         fMap_pulseIn},
    {"button_pressed",  fMap_button_pressed},
    // clang-format on
};


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_pin_init(void)
{
    function_map_collection["esp_pin"] = &function_map;
}


/****************************
 * static function
 ***************************/

// 设置引脚模式 0:输入 1:输出 2:输入上拉 3:输入下拉 4 开漏输出
static fmap_result_t fMap_pinMode(udc_pack_t *pack)
{
    int res_err = 0;
    std::string pin;
    int mode;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &pin);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &mode);

    res_err = hw_pin.pinMode(pin.c_str(), mode);

    return fmap_result_t::make_result(res_err);
}

// 设置引脚输出电平  0:低电平 1:高电平
static fmap_result_t fMap_digitalWrite(udc_pack_t *pack)
{
    int res_err = 0;
    std::string pin;
    int value;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &pin);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &value);

    res_err = hw_pin.digitalWrite(pin.c_str(), value);

    return fmap_result_t::make_result(res_err);
}

// 读取引脚电平  0:低电平 1:高电平
static fmap_result_t fMap_digitalRead(udc_pack_t *pack)
{
    int res_err = 0;
    std::string pin;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &pin);
    res_err = hw_pin.digitalRead(pin.c_str());

    if (res_err < 0)
        return fmap_result_t::make_error(res_err);
    else
        return fmap_result_t::make_ok().add_int(res_err);
}

// 设置引脚PWM输出频率和占空比  value:占空比 0~255
static fmap_result_t fMap_analogWrite(udc_pack_t *pack)
{
    int res_err = 0;
    std::string pin;
    int value;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &pin);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &value);
    res_err = hw_pin.analogWrite(pin.c_str(), value);

    return fmap_result_t::make_result(res_err);
}

// 读取引脚模拟值  0~1023
static fmap_result_t fMap_analogRead(udc_pack_t *pack)
{
    int res_err = 0;
    std::string pin;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &pin);
    res_err = hw_pin.analogRead(pin.c_str());

    if (res_err < 0)
        return fmap_result_t::make_error(res_err);
    else
        return fmap_result_t::make_ok().add_int(res_err);
}

// 读取引脚脉冲宽度  timeout:超时时间 单位ms
static fmap_result_t fMap_pulseIn(udc_pack_t *pack)
{
    int res_err = 0;
    std::string pin;
    int state, timeout;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &pin);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &state);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &timeout);

    res_err = hw_pin.pulseIn(pin.c_str(), state, timeout);

    if (res_err < 0)
        return fmap_result_t::make_error(res_err);
    else
        return fmap_result_t::make_ok().add_int(res_err);
}

// 板载按键是否按下; 0未按下、1A按键按下、2B按键按下、3AB按键按下
static fmap_result_t fMap_button_pressed(udc_pack_t *pack)
{
    return fmap_result_t::make_ok().add_int(hw_pin.button_pressed());
}
