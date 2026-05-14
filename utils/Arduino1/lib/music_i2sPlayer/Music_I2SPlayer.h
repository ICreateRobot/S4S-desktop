#ifndef MUSIC_I2SPLAYER_H
#define MUSIC_I2SPLAYER_H

#include "HardwareSerial.h"
#include "Music_Comms.h"

/* 音乐播放 抽象类型 */
class Music_I2SPlayer
{
private:
    // mp3 播放标志位 0停止/空闲 1 播放 2录音
    volatile int isPlaying = false;
    // WVA_RECORD_ 播放标志位 0停止/空闲 1 播放 2录音
    volatile short WVA_RECORD_PlayerBit = 0;
    HardwareSerial *serials;

    /*
     * 发送指令：
     */
    inline void sendCommand(const char *cmm)
    {
        if (cmm != nullptr)
        {
            serials->println(cmm);
        }
    }

public:
    /*
     *默认使用 Serial2 ，在unoR4Wifil 上Serial2 作为AT串口
     */
    Music_I2SPlayer(HardwareSerial *ats)
        : serials(ats ? ats : &Serial2){}

    Music_I2SPlayer()
        : serials(&Serial2){}

    ~Music_I2SPlayer() {}

    /*
     *  初始化音乐结构，串口也会在这初始化
     * @param none
     * @return void
     */
    void begin();

    /* 重新初始化播放器
    * @param true 初始化成功，false 初始化失败
    */
    bool music_begin();

    /*获取sd卡初始化状态
    * @param true 初始化成功，false 初始化失败
    */
    bool sdcard_begin();
    
    /*
     * 获取指令发送后面状态：
     * @return
     * -3 无数据
     * -2 超时
     * -1 指令返回为 错误
     * 0 指令返回 为正确
     * 1~255 成功与指令返回对应值
     */
    int getCommandStatus(uint32_t timeoutMs);

    /**
     * 重新加载音频文件，并且停止播放。
     * 如果之前调用过 FilePath 或 FileName 修改了播放文件，则加载修改后的文件；
     * 如果未修改过文件，则加载默认文件 "test.mp3"。。
     *
     * @param none
     *
     * 注意：调用此函数前请确保指定的文件存在，否则可能无法播放。
     */
    void Reload();

    /*
     * 停止播放音乐，并且关闭功放
     * @param enable 是否使能功放关闭，默认关闭
     * @return void
     */
    void stop();

    /*
     * 开始播放音乐，并且打开功放
     * @param enable 是否使能功放打开，默认打开
     * @return void
     */
    void play();

    /*
     * 设置播放文件路径+文件名
     * @param file 文件路径+文件名字符串指针
     * @return 见getCommandStatus() 函数
     *
     * 参数不要换行符直接字符串字面量就可以！！
     * 例如： "/music/test.mp3"
     */
    int FilePath(const char *file);

    /*
     * 设置播放文件名（路径使用默认）
     * @param file 文件名字符串指针
     * @return 见getCommandStatus() 函数 见getCommandStatus() 函数
     *
     * 参数 不要换行符直接字符串字面量就可以！！
     * 例如： "test.mp3"
     */
    int FileName(const char *file);

    /*
     * 设置增益
     * @param gain 增益，范围0~100
     * @return 见getCommandStatus() 函数
     */
    int SetGain(int gain);

    /*
     * 获取当前增益
     * @param none
     * @return 见getCommandStatus() 函数
     */
    int GetGain();

    /*
     * 设置音量
     * @param vol 音量，0~100
     * @return 见getCommandStatus() 函数
     */
    int SetVolume(int vol);

    /*
     * 获取当前音量
     * @param none
     * @return 见getCommandStatus() 函数
     */
    int GetVolume();

    /*
     * 是否正在播放
     * @param none
     * @return
     * 见getCommandStatus() 函数
     * 其中：
     * 1 正在播放，未播放返回其他
     */
    int IsPlaying();

    // 设置WVA_RECORD_路径
    int FilePath_wva_record(const char *filePath);
    // 默认路径设置文件名称
    int FileName_wva_record(const char *file);

    // 重新加载WVA_RECORD_音频文件，并且停止播放。
    int Reload_wva_record();
    // 开始录音
    int StartRecording_wva_record(int durationSeconds);
    // 播放录音
    int PlayRecording_wva_record();
    // 停止播放/录音
    void StopRecording_wva_record();
    /*
     * 是否正在播放
     * @param none
     * @return 见getCommandStatus() 函数
     * 其中：
     * 1 正在播放
     * 2 录音中
     * 未播放返回其他
     */
    int IsPlaying_wva_record();
};

#endif
