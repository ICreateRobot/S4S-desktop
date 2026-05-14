/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-12-04 14:24:34
 * @LastEditTime : 2026-04-17 09:29:07
 * @Description  : esp oled 设备协议处理
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_espOLED.h"
#include "hardware/esp_oled/hw_esp_oled.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_espOLED"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_init(udc_pack_t *pack);
static fmap_result_t fMap_clear_screen(udc_pack_t *pack);
static fmap_result_t fMap_set_text_size(udc_pack_t *pack);
static fmap_result_t fMap_print(udc_pack_t *pack);
static fmap_result_t fMap_draw_pixel(udc_pack_t *pack);
static fmap_result_t fMap_draw_line(udc_pack_t *pack);
static fmap_result_t fMap_draw_rect(udc_pack_t *pack);
static fmap_result_t fMap_draw_circle(udc_pack_t *pack);
static fmap_result_t fMap_refresh(udc_pack_t *pack);

/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"init",              fMap_init          },
    {"clear_screen",      fMap_clear_screen  },
    {"set_text_size",     fMap_set_text_size },
    {"print",             fMap_print         },
    {"draw_pixel",        fMap_draw_pixel    },
    {"draw_line",         fMap_draw_line     },
    {"draw_rect",         fMap_draw_rect     },
    {"draw_circle",       fMap_draw_circle   },
    {"refresh",           fMap_refresh       },
    // clang-format on
};

/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_espOLED_init(void)
{
    function_map_collection["esp_oled"] = &function_map;
}

/****************************
 * static function
 ***************************/
static fmap_result_t fMap_init(udc_pack_t *pack)
{
    hw_esp_oled.init();
    return fmap_result_t::make_ok();
}

static fmap_result_t fMap_clear_screen(udc_pack_t *pack)
{
    hw_esp_oled.clear_screen();
    return fmap_result_t::make_ok();
}

static fmap_result_t fMap_set_text_size(udc_pack_t *pack)
{
    int size = 0;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &size);
    hw_esp_oled.set_text_size(size);
    return fmap_result_t::make_ok();
}

static fmap_result_t fMap_print(udc_pack_t *pack)
{
    int         x, y;
    std::string str;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &x);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &y);
    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(2), &str);
    hw_esp_oled.print(x, y, str.c_str());
    return fmap_result_t::make_ok();
}

static fmap_result_t fMap_draw_pixel(udc_pack_t *pack)
{
    int x, y;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &x);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &y);
    hw_esp_oled.draw_pixel(x, y);
    return fmap_result_t::make_ok();
}

static fmap_result_t fMap_draw_line(udc_pack_t *pack)
{
    int x0, y0, x1, y1;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &x0);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &y0);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &x1);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(3), &y1);
    hw_esp_oled.draw_line(x0, y0, x1, y1);
    return fmap_result_t::make_ok();
}

static fmap_result_t fMap_draw_rect(udc_pack_t *pack)
{
    int x, y, w, h;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &x);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &y);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &w);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(3), &h);
    hw_esp_oled.draw_rect(x, y, w, h);
    return fmap_result_t::make_ok();
}

static fmap_result_t fMap_draw_circle(udc_pack_t *pack)
{
    int x, y, r;
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &x);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &y);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &r);
    hw_esp_oled.draw_circle(x, y, r);
    return fmap_result_t::make_ok();
}

static fmap_result_t fMap_refresh(udc_pack_t *pack)
{
    hw_esp_oled.refresh();
    return fmap_result_t::make_ok();
}
