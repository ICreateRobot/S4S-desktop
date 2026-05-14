#ifndef MUSIC_COMMANDS_H
#define MUSIC_COMMANDS_H
#include "Arduino.h"
#include "Music_I2SPlayer.h"



/* AT - 命令 */

/* -PLAY 开始播放
*-PLAY= 获取当前播放状态
*回复为 "AT-PLAY=1" 表示播放中，"AT-PLAY=0" 表示停止
* 支持WVA_RECORD\MP3
*/
#define Music_PLAY                    "-PLAY"  

/* -STOP 停止播放
* 支持WVA_RECORD\MP3
*/
#define Music_STOP                    "-STOP"

/* -FilePath 设置/获取文件路径
*支持WVA_RECORD\MP3
*/
#define Music_FILEPATH                "-FilePath="

/* -FileName 设置/获取文件名
*支持WVA_RECORD\MP3
*/
#define Music_FILENAME                "-FileName="

/* -VOLUME 设置音量，范围可自定义
* 支持MP3
* 只对MP3生效
*/
#define Music_VOLUME                  "-VOLUME="

/* -GAIN 设置增益（音量补偿或放大）
* 支持MP3
* 但都同步生效 WVA_RECORD/MP3
*/
#define Music_GAIN                    "-GAIN="

/* -Reload 重新加载文件或刷新播放
*支持WVA_RECORD\MP3
*/
#define Music_RELOAD                  "-Reload"

/* -Record 录音
* 只支持WVA_RECORD 有时间参数
*/
#define Music_RECORD                    "-Record="

/* -Begin 获取播放器初始化状态|初始化播放器
* 前置指令 | 支持AT
*/
#define Music_BEGIN                    "-Begin"

// -----------------------------
// AT 命令应答宏定义
// -----------------------------

// 正确执行命令后的应答
#define Music_OK                      "OK!"

// 正确执行命令并返回数值的应答
#define Music_OK_VALUE                "OK="

// 命令执行错误的应答
#define Music_ERR                     "ER!"

//未知命令应答
#define Music_ERR_UNKNOWN             "ERU_CMD!"

// -----------------------------
// 命令封装宏
// -----------------------------
//WVA_RECORD 指令
#define WVA_RECORD "REC"
#define _AT "AT"
// 用于封装 AT 命令判断，不需要 "\r\n" 结尾
#define Music_CMD(x) _AT x
#define Music_CMD_WRITE(x) _AT x "="

//用于封装 WVA_RECORD命令判断，不需要 "\r\n" 结尾
#define WVA_RECORD_CMD(x) WVA_RECORD x
#define WVA_RECORD_CMD_WRITE(x) WVA_RECORD x "="
#endif
