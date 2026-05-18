/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-12-04 10:52:58
 * @LastEditTime : 2026-05-08 14:42:50
 * @Description  : esp audio 协议处理
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_espAudio.h"
#include "hardware/esp_audio/hw_esp_audio.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_espAudio"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_get_sound_level(udc_pack_t *pack);
static fmap_result_t fMap_start_recording(udc_pack_t *pack);
static fmap_result_t fMap_play_recording(udc_pack_t *pack);
static fmap_result_t fMap_set_audio_file(udc_pack_t *pack);
static fmap_result_t fMap_set_volume(udc_pack_t *pack);
static fmap_result_t fMap_play_audio(udc_pack_t *pack);


/********************
 * static variables
 *******************/
static const function_map_t function_map = {
    // clang-format off
    {"get_sound_level",  fMap_get_sound_level},
    {"start_recording",  fMap_start_recording},
    {"play_recording",   fMap_play_recording},
    {"set_audio_file",   fMap_set_audio_file},
    {"set_volume",       fMap_set_volume},
    {"play_audio",       fMap_play_audio}
    // clang-format on
};


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_espAudio_init(void)
{
    function_map_collection["esp_audio"] = &function_map;
}


/****************************
 * static function
 ***************************/

// 获取周围的声音强度
static fmap_result_t fMap_get_sound_level(udc_pack_t *pack)
{
    int ret_error = 0;

    ret_error = hw_esp_audio.get_sound_level();
    
    return fmap_result_t::make_ok().add_int(ret_error);
}

// 开始录音
static fmap_result_t fMap_start_recording(udc_pack_t *pack)
{
    int sec = 0;
    int ret_error = 0;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &sec);
    ret_error = hw_esp_audio.start_recording(sec);

    return fmap_result_t::make_result(ret_error);
}

// 播放录音 0:停止播放 1:开始播放
static fmap_result_t fMap_play_recording(udc_pack_t *pack)
{
    int state = 0;
    int ret_error = 0;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &state);
    ret_error = hw_esp_audio.play_recording(state);
    
    return fmap_result_t::make_result(ret_error);
}

// 设置播放的音频文件
static fmap_result_t fMap_set_audio_file(udc_pack_t *pack)
{
    int ret_error = 0;
    std::string file_name;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &file_name);
    
    ret_error = hw_esp_audio.set_audio_file(file_name.c_str());
    return fmap_result_t::make_result(ret_error);
}

// 设置音量 0-100
static fmap_result_t fMap_set_volume(udc_pack_t *pack)
{
    int ret_error = 0;
    int volume = 0;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &volume);

    ret_error = hw_esp_audio.set_volume(volume);
    return fmap_result_t::make_result(ret_error);
}

// 播放音频文件 0:停止播放 1:开始播放
static fmap_result_t fMap_play_audio(udc_pack_t *pack)
{
    int state = 0;
    int ret_error = 0;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &state);
    ret_error = hw_esp_audio.play_audio(state);
    return fmap_result_t::make_result(ret_error);
}

