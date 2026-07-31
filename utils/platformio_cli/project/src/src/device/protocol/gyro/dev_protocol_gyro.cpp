/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-03-18 17:26:52
 * @LastEditTime : 2026-04-16 14:31:55
 * @Description  : 陀螺仪
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_gyro.h"
#include "hardware/gyro/hw_gyro.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_gyro"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_readFloatAccelX(udc_pack_t *pack);
static fmap_result_t fMap_readFloatAccelY(udc_pack_t *pack);
static fmap_result_t fMap_readFloatAccelZ(udc_pack_t *pack);
static fmap_result_t fMap_readFloatGyroX(udc_pack_t *pack);
static fmap_result_t fMap_readFloatGyroY(udc_pack_t *pack);
static fmap_result_t fMap_readFloatGyroZ(udc_pack_t *pack);
static fmap_result_t fMap_readTempC(udc_pack_t *pack);
static fmap_result_t fMap_readTempF(udc_pack_t *pack);



/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"readFloatAccelX",     fMap_readFloatAccelX},
    {"readFloatAccelY",     fMap_readFloatAccelY},
    {"readFloatAccelZ",     fMap_readFloatAccelZ},
    {"readFloatGyroX",      fMap_readFloatGyroX},
    {"readFloatGyroY",      fMap_readFloatGyroY},
    {"readFloatGyroZ",      fMap_readFloatGyroZ},
    {"readTempC",           fMap_readTempC},
    {"readTempF",           fMap_readTempF},
    // clang-format on
};

/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_gyro_init(void)
{
    function_map_collection_main["gyro"] = &function_map;
}

/****************************
 * static function
 ***************************/
static fmap_result_t fMap_readFloatAccelX(udc_pack_t *pack)
{
    float accelX = hw_gyro.readFloatAccelX();

    return fmap_result_t::make_ok().add_float(accelX);
}

static fmap_result_t fMap_readFloatAccelY(udc_pack_t *pack)
{
    float accelY = hw_gyro.readFloatAccelY();
    return fmap_result_t::make_ok().add_float(accelY);
}

static fmap_result_t fMap_readFloatAccelZ(udc_pack_t *pack)
{
    float accelZ = hw_gyro.readFloatAccelZ();
    return fmap_result_t::make_ok().add_float(accelZ);
}

static fmap_result_t fMap_readFloatGyroX(udc_pack_t *pack)
{
    float gyroX = hw_gyro.readFloatGyroX();
    return fmap_result_t::make_ok().add_float(gyroX);
}

static fmap_result_t fMap_readFloatGyroY(udc_pack_t *pack)
{
    float gyroY = hw_gyro.readFloatGyroY();
    return fmap_result_t::make_ok().add_float(gyroY);
}

static fmap_result_t fMap_readFloatGyroZ(udc_pack_t *pack)
{
    float gyroZ = hw_gyro.readFloatGyroZ();
    return fmap_result_t::make_ok().add_float(gyroZ);
}

static fmap_result_t fMap_readTempC(udc_pack_t *pack)
{
    float tempC = hw_gyro.readTempC();
    return fmap_result_t::make_ok().add_float(tempC);
}

static fmap_result_t fMap_readTempF(udc_pack_t *pack)
{
    float tempF = hw_gyro.readTempF();
    return fmap_result_t::make_ok().add_float(tempF);
}

