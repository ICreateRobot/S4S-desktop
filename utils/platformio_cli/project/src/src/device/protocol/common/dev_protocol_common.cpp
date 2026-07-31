/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-27 11:01:58
 * @LastEditTime : 2026-07-23 17:19:27
 * @Description  : 协议的公共接口
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_common.h"

/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_common"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/


/********************
 * global variables
 *******************/
function_map_collection_t function_map_collection_main;
function_map_collection_t function_map_collection_sys;


/********************
 * global functions
 *******************/
void dev_protocol_common_init(void)
{
    // cc_hash_map_print(dev_mainboard_reg, NULL, "\n");
}

/**
 * @description: 在函数map中寻找函数并执行
 * @param {function_map_t *} function_map 函数map
 * @param {char *} function_name          函数名
 * @param {udc_pack_t *} pack             数据pack
 * @return {*}
 */
fmap_result_t function_map_exec(const function_map_t *function_map, std::string function_name,
                                udc_pack_t *pack)
{    
    for (auto &it : *function_map)
    {
        if (0 == strcmp(it.first, function_name.c_str()))
        {
            if (nullptr == it.second) return fmap_result_t::make_error(-11);
            else return it.second(pack);
        }
    }
    return fmap_result_t::make_error(-10);    
}

std::string function_map_udcobj_to_string(udc_obj_t *udcobj)
{
    char str[128] = {0};
    for (uint16_t i = 0; i < udcobj->size; i++) { str[i] = udcobj->data[i]; }
    return std::string(str);
}

int8_t function_map_udcobj_to_int8(udc_obj_t *udcobj)
{
    return (int8_t)udcobj->data[0];
}

uint8_t function_map_udcobj_to_uint8(udc_obj_t *udcobj)
{
    return udcobj->data[0];
}

int16_t function_map_udcobj_to_int16(udc_obj_t *udcobj)
{
    return (int16_t)((uint32_t)udcobj->data[0] << 8 | (uint32_t)udcobj->data[1]);
}

uint16_t function_map_udcobj_to_uint16(udc_obj_t *udcobj)
{
    return (uint32_t)udcobj->data[0] << 8 | (uint32_t)udcobj->data[1];
}

int32_t function_map_udcobj_to_int32(udc_obj_t *udcobj)
{
    return (int32_t)((uint32_t)udcobj->data[0] << 24 | (uint32_t)udcobj->data[1] << 16
                     | (uint32_t)udcobj->data[2] << 8 | (uint32_t)udcobj->data[3]);
}

uint32_t function_map_udcobj_to_uint32(udc_obj_t *udcobj)
{
    return (uint32_t)udcobj->data[0] << 24 | (uint32_t)udcobj->data[1] << 16
           | (uint32_t)udcobj->data[2] << 8 | (uint32_t)udcobj->data[3];
}


/**
 * @description: 根据传入的字符串，解析出数据
 * @param {string} &input
 * @return {*}
 */
parseValue_t parse_value(const std::string &input)
{
    parseValue_t result;

    if (input.empty()) return result;

    /* ---------- 1. 引号字符串 ---------- */
    if (input.size() >= 2 && input.front() == '"' && input.back() == '"')
    {
        result.type  = FMapValueType::STRING;
        result.s_val = input.substr(1, input.size() - 2); // 去掉引号
        return result;
    }

    /* ---------- 2. 整数 ---------- */
    {
        char *end = nullptr;
        errno     = 0;
        long v    = std::strtol(input.c_str(), &end, 10);

        if (errno == 0 && end != input.c_str() && *end == '\0')
        {
            result.type  = FMapValueType::INT;
            result.i_val = static_cast<int>(v);
            return result;
        }
    }

    /* ---------- 3. 浮点 ---------- */
    {
        char *end = nullptr;
        errno     = 0;
        float v   = std::strtod(input.c_str(), &end);

        if (errno == 0 && end != input.c_str() && *end == '\0')
        {
            result.type  = FMapValueType::FLOAT;
            result.f_val = v;
            return result;
        }
    }

    /* ---------- 4. 非法 ---------- */
    return result;
}

static int function_map_udcpack_get_type_param_default(const udc_pack_t *pack, uint8_t id,
                                                       parseValue_t *parseValue, FMapValueType type)
{
    udc_obj_t    param_obj = {0};
    std::string  param_str;
    parseValue_t parseValue_temp;
    if (0 != udc_pack_get_obj(pack, UDC_PACK_RECEIVE, id, &param_obj)) { return -1; }
    param_str       = function_map_udcobj_to_string(&param_obj);
    parseValue_temp = parse_value(param_str);
    if (type != parseValue_temp.type) { return -2; }
    *parseValue = parseValue_temp;
    return 0;
}

int function_map_udcpack_get_param_int_default(const udc_pack_t *pack, uint8_t id, int *pIntVal)
{
    int          ret = 0;
    parseValue_t parseValue;
    ret = function_map_udcpack_get_type_param_default(pack, id, &parseValue, FMapValueType::INT);
    if (0 != ret) { return ret; }
    *pIntVal = parseValue.i_val;
    return 0;
}

int function_map_udcpack_get_param_float_default(const udc_pack_t *pack, uint8_t id,
                                                 float *pDoubleVal)
{
    int          ret = 0;
    parseValue_t parseValue;
    ret = function_map_udcpack_get_type_param_default(pack, id, &parseValue, FMapValueType::FLOAT);
    if (0 != ret) { return ret; }
    *pDoubleVal = parseValue.f_val;
    return 0;
}

int function_map_udcpack_get_param_string_default(const udc_pack_t *pack, uint8_t id,
                                                  std::string *pStringVal)
{
    int          ret = 0;
    parseValue_t parseValue;
    ret = function_map_udcpack_get_type_param_default(pack, id, &parseValue, FMapValueType::STRING);
    if (0 != ret) { return ret; }
    *pStringVal = parseValue.s_val;
    return 0;
}

int enum_map_get_value_default(const enum_map_t &enum_map, const char * key, int &value)
{
    for (auto &it : enum_map)
    {
        if (0 == strcmp(it.first, key))
        {
            value = it.second;
            return 0;
        }
    }
    return -1;
}

/****************************
 * static function
 ***************************/
