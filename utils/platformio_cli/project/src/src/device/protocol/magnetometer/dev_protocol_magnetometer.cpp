/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-03-18 17:39:21
 * @LastEditTime : 2026-03-18 17:40:52
 * @Description  : 磁力计
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_magnetometer.h"
#include "hardware/magnetometer/hw_magnetometer.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_magnetometer"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_getRawMagneticX(udc_pack_t *pack);
static fmap_result_t fMap_getRawMagneticY(udc_pack_t *pack);
static fmap_result_t fMap_getRawMagneticZ(udc_pack_t *pack);
static fmap_result_t fMap_getGaussFieldX(udc_pack_t *pack);
static fmap_result_t fMap_getGaussFieldY(udc_pack_t *pack);
static fmap_result_t fMap_getGaussFieldZ(udc_pack_t *pack);



/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"getRawMagneticX",   fMap_getRawMagneticX},
    {"getRawMagneticY",   fMap_getRawMagneticY},
    {"getRawMagneticZ",   fMap_getRawMagneticZ},
    {"getGaussFieldX",    fMap_getGaussFieldX},
    {"getGaussFieldY",    fMap_getGaussFieldY},
    {"getGaussFieldZ",    fMap_getGaussFieldZ},
    // clang-format on
};

/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_magnetometer_init(void)
{
    function_map_collection_main["magnetometer"] = &function_map;
}

/****************************
 * static function
 ***************************/
static fmap_result_t fMap_getRawMagneticX(udc_pack_t *pack)
{
    int RawMagneticX = hw_magnetometer.getRawMagneticX();
    return fmap_result_t::make_ok().add_int(RawMagneticX);
}

static fmap_result_t fMap_getRawMagneticY(udc_pack_t *pack)
{
    int RawMagneticY = hw_magnetometer.getRawMagneticY();
    return fmap_result_t::make_ok().add_int(RawMagneticY);
}

static fmap_result_t fMap_getRawMagneticZ(udc_pack_t *pack)
{
    int RawMagneticZ = hw_magnetometer.getRawMagneticZ();
    return fmap_result_t::make_ok().add_int(RawMagneticZ);
}

static fmap_result_t fMap_getGaussFieldX(udc_pack_t *pack)
{
    float GaussFieldX = hw_magnetometer.getGaussFieldX();
    return fmap_result_t::make_ok().add_float(GaussFieldX);
}

static fmap_result_t fMap_getGaussFieldY(udc_pack_t *pack)
{
    float GaussFieldY = hw_magnetometer.getGaussFieldY();
    return fmap_result_t::make_ok().add_float(GaussFieldY);
}

static fmap_result_t fMap_getGaussFieldZ(udc_pack_t *pack)
{
    float GaussFieldZ = hw_magnetometer.getGaussFieldZ();
    return fmap_result_t::make_ok().add_float(GaussFieldZ);
}
