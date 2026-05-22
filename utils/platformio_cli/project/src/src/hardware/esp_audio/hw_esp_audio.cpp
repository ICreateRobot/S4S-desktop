/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-12-03 17:10:54
 * @LastEditTime : 2026-05-20 16:11:17
 * @Description  : esp 音频硬件接口
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include <stdlib.h>
#include "hw_esp_audio.h"
#include "Music_I2SPlayer.h"
#include "misc/mtools.h"
#include "string.h"
#include "interface/uart/if_uart.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "hw_esp_audio_c"

#define AT_HEADER "AT-"
#define AT_TAIL "\n"
#define AT_CMD(x) AT_HEADER x AT_TAIL


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/
static Music_I2SPlayer music;

/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/


/********************
 * class functions
 *******************/
hw_esp_audio_c::hw_esp_audio_c()
{}


hw_esp_audio_c::~hw_esp_audio_c()
{}

void hw_esp_audio_c::begin(void)
{
    music.begin();
}

int hw_esp_audio_c::at_cmd(const char *cmd, uint16_t size)
{
    // 每次发送前，先清空串口数据
    if_uart_audio.clear();
    // 状态初始化
    at_command.state = ESP_AUDIO_STATE_NONE;
    if (0 == size)
    {
        if_uart_audio.send_bytes((uint8_t *)cmd, strlen(cmd));
    } else
    {
        if_uart_audio.send_bytes((uint8_t *)cmd, size);
    }
    delay(10);
    at_command.rev_size =
        if_uart_audio.read_bytes(at_command.rev_buffer, sizeof(at_command.rev_buffer));

    // 串口数据处理
    if (at_command.rev_size < 3)
    {
        at_command.state = ESP_AUDIO_STATE_ERROR;
        ZST_LOGE(LOG_TAG, "uart rev data error");
        return -1;
    }

    // 判断串口数据是否为可打印字符
    if (0 != all_printable(at_command.rev_buffer, at_command.rev_size))
    {
        ZST_LOGE(LOG_TAG, "received data is not printable");
        return -1;
    }

    if (0 == strncmp((const char *)at_command.rev_buffer, "OK", 2))
    {
        at_command.state = ESP_AUDIO_STATE_OK;
        if ('=' == at_command.rev_buffer[2])
        {
            at_command.ack_data = &at_command.rev_buffer[3];
            at_command.ack_size = at_command.rev_size - 3;
        } else if ('!' == at_command.rev_buffer[2])
        {
            at_command.ack_size = 0;
        }
    } else
    {
        at_command.state = ESP_AUDIO_STATE_ERROR;
        return -1;
    }
    return 0;
}


// 获取周围的声音强度
int hw_esp_audio_c::get_sound_level(void)
{
    int sound_level = music.Environmental_sound();
    delay(2);
    return sound_level;
}

// 开始录音
int hw_esp_audio_c::start_recording(int sec)
{
    int ret = 0;
    ret = music.StartRecording_WVA_RECORD(sec);
    delay(2);
    return ret;
}

// 播放录音 0:停止播放 1:开始播放
int hw_esp_audio_c::play_recording(int state)
{
    int ret = 0;
    switch (state)
    {
        case 0:
            music.StopRecording_WVA_RECORD();
            break;
        case 1:
            music.PlayRecording_WVA_RECORD();
            break;
    }
    delay(2);
    return ret;
}

// 设置播放的音频文件
int hw_esp_audio_c::set_audio_file(const char *file_name)
{
    int ret = 0;
    ret = music.FilePath(file_name);
    delay(2);
    return ret;
}

// 设置音量 0-100
int hw_esp_audio_c::set_volume(int volume)
{
    int ret = 0;
    ret = music.SetVolume(volume);
    delay(2);
    return ret;
}

// 播放音频文件 0:停止播放 1:开始播放
int hw_esp_audio_c::play_audio(int state)
{
    int ret = 0;
    switch (state)
    {
        case 0:
            music.stop();
            break;
        case 1:
            music.Reload();
            delay(5);
            music.play();
            break;
    }
    delay(2);
    return ret;
}


hw_esp_audio_c hw_esp_audio;
