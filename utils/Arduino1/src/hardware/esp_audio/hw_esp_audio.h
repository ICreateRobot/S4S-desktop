/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-12-03 17:11:06
 * @LastEditTime : 2026-04-23 16:40:05
 * @Description  : esp 音频硬件接口
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_ESP_AUDIO_H__
#define __HW_ESP_AUDIO_H__

#include "main.h"

class hw_esp_audio_c
{
public:
    hw_esp_audio_c(void);
    ~hw_esp_audio_c(void);

public:
    typedef enum
    {
        ESP_AUDIO_STATE_NONE  = -1,
        ESP_AUDIO_STATE_OK    = 0,
        ESP_AUDIO_STATE_ERROR = 1
    } esp_audio_state_t;

    struct {
        uint8_t           rev_buffer[128]; // 接收缓存
        uint16_t          rev_size;        // 接收到的数据大小
        esp_audio_state_t state;           // 状态
        uint8_t          *ack_data;        // 响应数据
        uint16_t          ack_size;        // 响应数据大小
    } at_command = {0};

public:
    void begin(void);
    int  at_cmd(const char *cmd, uint16_t size = 0);
    
    // 获取周围的声音强度
    int get_sound_level(void);

    // 开始录音
    int start_recording(int sec);

    // 播放录音 0:停止播放 1:开始播放
    int play_recording(int state);

    // 设置播放的音频文件
    int set_audio_file(const char *file_name);

    // 设置音量 0-100
    int set_volume(int volume);

    // 播放音频文件 0:停止播放 1:开始播放
    int play_audio(int state);
};

extern hw_esp_audio_c hw_esp_audio;

#endif /* __HW_ESP_AUDIO_H__ */
