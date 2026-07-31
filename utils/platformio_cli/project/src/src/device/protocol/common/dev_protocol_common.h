/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 16:47:12
 * @LastEditTime : 2026-07-24 08:45:57
 * @Description  : 协议的公共头文件
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __DEV_PROTOCOL_COMMON_H__
#define __DEV_PROTOCOL_COMMON_H__

#include "main.h"
#include <map>
#include <vector>

typedef struct fmap_result_t fmap_result_t;

typedef fmap_result_t (*func_ptr_t)(udc_pack_t *pack);
typedef std::map<const char *, func_ptr_t>             function_map_t;
typedef std::map<const char *, const function_map_t *> function_map_collection_t;
typedef std::map<const char *, int>                    enum_map_t;

enum class FMapValueType
{
    INVALID = 0,
    INT,
    FLOAT, // 双精度浮点数
    STRING,
    BYTES,
};

typedef struct {
    FMapValueType        type  = FMapValueType::INVALID;
    int                  i_val = 0;
    float                f_val = 0.0;
    std::string          s_val;
    std::vector<uint8_t> b_val;
} parseValue_t;

typedef struct fmap_field_t {
    // clang-format off
    parseValue_t parse_value;

    /* ---------- 静态工厂 ---------- */
    static fmap_field_t from_int   (int v)                        { fmap_field_t f; f.parse_value.type = FMapValueType::INT;    f.parse_value.i_val = v; return f; }
    static fmap_field_t from_float (float v)                      { fmap_field_t f; f.parse_value.type = FMapValueType::FLOAT;  f.parse_value.f_val = v; return f; }
    static fmap_field_t from_string(const std::string &v)         { fmap_field_t f; f.parse_value.type = FMapValueType::STRING; f.parse_value.s_val = v; return f; }
    static fmap_field_t from_bytes (const std::vector<uint8_t>&v) { fmap_field_t f; f.parse_value.type = FMapValueType::BYTES;  f.parse_value.b_val = v; return f; }
    // clang-format on
} fmap_field_t;

// ============================================================
//  handler 统一返回值（持有多个字段）
// ============================================================
struct fmap_result_t {
    // clang-format off
    bool                      ok       = true;
    int                       err_code = 0;
    std::vector<fmap_field_t> fields;

    /* ---------- 链式构造 ---------- */
    fmap_result_t &add_int   (int v)                        { fields.push_back(fmap_field_t::from_int(v));    return *this; }
    fmap_result_t &add_float (float v)                      { fields.push_back(fmap_field_t::from_float(v));  return *this; }
    fmap_result_t &add_string(const std::string &v)         { fields.push_back(fmap_field_t::from_string(v)); return *this; }
    fmap_result_t &add_bytes (const std::vector<uint8_t>&v) { fields.push_back(fmap_field_t::from_bytes(v));  return *this; }

    /* ---------- 便捷静态构造 ---------- */
    static fmap_result_t make_ok()            { return fmap_result_t{true,  0,   {}}; }
    static fmap_result_t make_error(int code) { return fmap_result_t{false, code,{}}; }
    static fmap_result_t make_result(int state) {return state>=0 ? fmap_result_t::make_ok() : fmap_result_t::make_error(state);}

    bool is_ok()    const { return ok; }
    bool has_data() const { return !fields.empty(); }
    // clang-format on
};


#define function_map_udcpack_get_param_int(pack, id, pIntVal)                                      \
    do                                                                                             \
    {                                                                                              \
        int ret = 0;                                                                               \
        ret     = function_map_udcpack_get_param_int_default(pack, id, pIntVal);                   \
        if (ret != 0)                                                                              \
        {                                                                                          \
            ZST_LOGE(LOG_TAG, "get int param %d failed", id);                                      \
            return fmap_result_t::make_error(ret);                                                 \
        }                                                                                          \
    } while (0)

#define function_map_udcpack_get_param_float(pack, id, pFloatVal)                                  \
    do                                                                                             \
    {                                                                                              \
        int ret = 0;                                                                               \
        ret     = function_map_udcpack_get_param_float_default(pack, id, pDoubleVal);              \
        if (ret != 0)                                                                              \
        {                                                                                          \
            ZST_LOGE(LOG_TAG, "get float param %d failed", id);                                    \
            return fmap_result_t::make_error(ret);                                                 \
        }                                                                                          \
    } while (0)

#define function_map_udcpack_get_param_string(pack, id, pStringVal)                                \
    do                                                                                             \
    {                                                                                              \
        int ret = 0;                                                                               \
        ret     = function_map_udcpack_get_param_string_default(pack, id, pStringVal);             \
        if (ret != 0)                                                                              \
        {                                                                                          \
            ZST_LOGE(LOG_TAG, "get string param %d failed", id);                                   \
            return fmap_result_t::make_error(ret);                                                 \
        }                                                                                          \
    } while (0)

#define enum_map_get_value(enum_map, key, value)                                                   \
    do                                                                                             \
    {                                                                                              \
        int ret = 0;                                                                               \
        ret     = enum_map_get_value_default(enum_map, key, value);                                \
        if (ret != 0)                                                                              \
        {                                                                                          \
            ZST_LOGE(LOG_TAG, "get enum param %s failed", key);                                    \
            return fmap_result_t::make_error(ret);                                                 \
        }                                                                                          \
    } while (0)

#define function_map_udcpack_id(id) (id + 12)

#define function_map_check(expr)                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(expr))                                                                               \
        {                                                                                          \
            ZST_LOGE("FMAP_ASSERT", "%s", #expr);                                                  \
            return fmap_result_t::make_error(-1);                                                  \
        }                                                                                          \
    } while (0)


void dev_protocol_common_init(void);

fmap_result_t function_map_exec(const function_map_t *function_map, std::string function_name,
                                udc_pack_t *pack);

std::string function_map_udcobj_to_string(udc_obj_t *udcobj);

/**************************************************************
 * 接收字节类型的 packobj, 按照大端格式拼接这几个字节到 int8_t、
 * uint8_t、int16_t、uint16_t、int32_t、uint32_t 的值
 *************************************************************/
int8_t   function_map_udcobj_to_int8(udc_obj_t *udcobj);
uint8_t  function_map_udcobj_to_uint8(udc_obj_t *udcobj);
int16_t  function_map_udcobj_to_int16(udc_obj_t *udcobj);
uint16_t function_map_udcobj_to_uint16(udc_obj_t *udcobj);
int32_t  function_map_udcobj_to_int32(udc_obj_t *udcobj);
uint32_t function_map_udcobj_to_uint32(udc_obj_t *udcobj);

/**************************************************************
 * 接收字符串 packobj, 解析这个字符串为 int、double、string 类
 * 型的值
 *************************************************************/
parseValue_t parse_value(const std::string &input);
int function_map_udcpack_get_param_int_default(const udc_pack_t *pack, uint8_t id, int *pIntVal);
int function_map_udcpack_get_param_float_default(const udc_pack_t *pack, uint8_t id,
                                                 float *pDoubleVal);
int function_map_udcpack_get_param_string_default(const udc_pack_t *pack, uint8_t id,
                                                  std::string *pStringVal);
int enum_map_get_value_default(const enum_map_t &enum_map, const char *key, int &value);

extern function_map_collection_t function_map_collection_main;
extern function_map_collection_t function_map_collection_sys;

#endif /* __DEV_PROTOCOL_COMMON_H__ */
