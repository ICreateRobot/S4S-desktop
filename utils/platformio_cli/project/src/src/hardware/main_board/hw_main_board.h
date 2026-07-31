/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-15 15:44:19
 * @LastEditTime : 2026-07-23 18:04:58
 * @Description  : 主板、超声波、四路巡线
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_MAIN_BOARD_H__
#define __HW_MAIN_BOARD_H__

#include "../common/hw_common.h"
#include "s4s_mainBoard.h"


class hw_main_board_c : public s4s_mainBoard
{
public:
    hw_main_board_c();
    ~hw_main_board_c();

public:
    enum 
    {
        // 双电机
        MOVEMENT_FORWARD = 0, // 前进
        MOVEMENT_BACKWARD,    // 后退
        MOVEMENT_LEFT,        // 左转
        MOVEMENT_RIGHT,       // 右转

        MOVEMENT_SECONDS = 0, // 秒
        MOVEMENT_ROTATIONS,   // 圈
        MOVEMENT_CENTIMETERS, // 厘米

        // 单电机
        MOTORS_FORWARD = 0, // 前进
        MOTORS_BACKWARD,    // 后退

        MOTORS_ROTATIONS = 0, // 圈
        MOTORS_DEGREES,       // 度
        MOTORS_SECONDS,       // 秒
        MOTORS_CENTIMETERS,   // 厘米

        RTC_YEAR = 0,       // 年
        RTC_MONTH,          // 月
        RTC_DAY,            // 日
        RTC_WEEK,           // 周
        RTC_HOUR,           // 时
        RTC_MINUTE,         // 分
        RTC_SECOND,         // 秒

        VOICE_UNKNOWN = 0, // 未知
        VOICE_LINKBOT,     // 设备名称 / 机器人名称 唤醒词
        VOICE_GO_FORWARD,  // 前进
        VOICE_GO_BACK,     // 后退
        VOICE_TURN_LEFT,   // 左转
        VOICE_TURN_RIGHT,  // 右转
        VOICE_START_MOVING, // 开始移动
        VOICE_STOP_MOVING, // 停止移动（运动控制）
        VOICE_SPEED_UP,    // 加速
        VOICE_SLOW_DOWN,   // 减速
        VOICE_SPIN_AROUND, // 原地旋转一圈
        VOICE_FOLLOW_LINE, // 循迹（沿线移动）
        VOICE_LIGHTS_ON,   // 打开灯光
        VOICE_LIGHTS_OFF,  // 关闭灯光
        VOICE_SHOW_RED,    // 显示红灯
        VOICE_SHOW_GREEN,  // 显示绿灯
        VOICE_SHOW_BLUE,   // 显示蓝灯
        VOICE_CHECK_DISTANCE, // 检测距离
        VOICE_SCAN_SURROUNDINGS, // 扫描周围环境
        VOICE_DETECT_OBJECT,     // 检测物体
        VOICE_DETECT_FACE,       // 人脸检测
        VOICE_DETECT_CARD,       // 卡片检测
        VOICE_READ_SENSOR,       // 读取传感器数据
        VOICE_CHECK_BATTERY,     // 检测电池电量
        VOICE_FIND_LINE,         // 寻找线条
        VOICE_WHO_AM_I,          // 识别当前设备 / 自我识别
        VOICE_WHO_ARE_YOU,       // 识别对方 / 询问身份
        VOICE_START,             // 开始程序 / 流程
        VOICE_STOP,              // 停止程序 / 流程
        VOICE_REPEAT,            // 重复操作
        VOICE_NEXT,              // 下一步操作
        VOICE_RETURN_HOME,       // 返回起点 / 回到初始位置
        VOICE_PICK_UP,           // 抓取物体
        VOICE_PUT_DOWN,          // 放下物体
        VOICE_SLEEP,             // 进入休眠模式
        VOICE_WAKE_UP,           // 唤醒系统
        VOICE_PLAY_MUSIC,        // 播放音乐
        VOICE_RECORD_DATA,       // 记录数据
        VOICE_SEND_MESSAGE,      // 发送消息 / 信号
        VOICE_RECEIVE_MESSAGE,   // 接收消息 / 信号
        VOICE_ATTACK_NOW,        // 发起攻击动作
        VOICE_DEFEND_YOURSELF,   // 执行防御动作
        VOICE_SHOW_ME_A_DANCE,   // 执行舞蹈动作
        VOICE_TELL_ME_A_JOKE,    // 讲笑话
    };
    

public:
    void begin(void);

    // 跳出电机等待状态，并停止所有电机运动
    void motor_wait_break(void);
    void restore_default(void);

    /****************************************
     *            双电机控制
     ***************************************/
    // 等待完成动作。-1表示一直等待; >=0 表示等待时间 ms
    int movement_wait_finsh(int timeout = -1);

    // 设置电机端口 0 ~ 3
    int movement_set_motors(int left, int right);
    // 立即开始沿选定的方向移动机器人  0前进；1后退；2左转；3右转
    int movement_start(int dir);
    /**
     * @description: 在指定的时间或距离内向选定的方向移动
     * @param dir   0前进；1后退；2左转；3右转
     * @param value 0 ~ 1000
     * @param unit  0 ~ 3（0秒，1圈，2厘米）
     */
    int movement_move(int dir, int value, int unit);
    // 设定左右电机的速度，并立即开始移动。 -100 ~ 100
    int movement_drive(int lspeed, int rspeed);
    /**
     * @description: 按照指定的速度持续运行一段固定的时间或距离
     * @param lspeed -100 ~ 100
     * @param rspeed -100 ~ 100
     * @param data   0 ~ 1000
     * @param unit  0 ~ 2（0秒，1圈，2厘米)
     */
    int movement_drive_for(int lspeed, int rspeed, int data, int unit);
    // 停止机器人运动
    int movement_stop(void);
    // 将两个电机调至相同转速，但不启动运动。
    int movement_set_speed(int speed);

    /****************************************
     *            单电机控制
     ***************************************/
    int motors_wait_finsh(int motor, int timeout = -1);
    /**
     * @description: 按照选定的方向运行指定的电机端口，持续一定时间或完成指定的旋转次数。
     * @param motor 0 ~ 3
     * @param dir   0正转 1反转
     * @param data  0 ~ 1000
     * @param unit 0 ~ 3（0圈,1度,2秒,3厘米）
     */
    int motors_run_for(int motor, int dir, int data, int unit);
    // 在指定端口控制电机，使其立即以选定方向开始持续旋转。 0正转，1反转
    int motors_start(int motor, int dir);
    // 停止指定的电机。
    int motors_stop(int motor);
    // 在不启动电机的情况下设定其转速
    int motors_set_speed(int motor, int speed);
    // 返回指定电机的当前位置
    int motors_position(int motor);
    // 获取当前速度百分比 0~100
    int motors_speed(int motor);
    // 将指定端口上的电机位置计数器重置为 0 。
    int motors_reset_position(int motor);
    /**
     * @description: 在指定端口控制电机，使其立即以指定的目标转速（每分钟转数）开始运行。
     * @param motor  0 ~ 3
     * @param rpm   -180 ~ 180
     */
    int motors_start_rpm(int motor, int rpm);
    // 获取指定端口处电机的当前转速（单位：转/分钟）
    int motors_rpm(int motor);

    /****************************************
    *            舵机控制
    ***************************************/
    // 将标准伺服器旋转至指定角度 0 ~ 1, 0 ~ 180
    int servo_set_angle(int servo, int angle);
    // 将伺服器从接口上拆下，以释放扭矩。
    int servo_release(int servo);
    // 以指定的速度运行连续旋转伺服装置 0 ~ 1, -100 ~ 100
    int servo_set_speed(int servo, int speed);
    // 停止连续伺服模式
    int servo_stop(int servo);

    /****************************************
    *            氛围灯控制
    ***************************************/
    // 设置机器人的环境灯光颜色 0 ~ 255
    int light_set_color(int r, int g, int b, int light=255);
    // 设置机器人的环境灯亮度 0 ~ 255
    int light_set_brightness(int light);

    /****************************************
    *            语音识别
    ***************************************/
    int voice_recognized(int recognized);
    String voice_version(void);

    
    /****************************************
    *               RTC
    ***************************************/
    // 设置日期 0 ~ 99, 1 ~ 12, 1 ~ 31
    int rtc_set_date(int year, int month, int day);
    // 设置时间 0 ~ 23, 0 ~ 59, 0 ~ 59
    int rtc_set_time(int hour, int minute, int second);
    // 获取当前时间  （0年，1月，2日，3周，4时，5分，6秒）
    int rtc_get(int sel);


    /****************************************
    *               power
    ***************************************/
    // "读取特定的时间值  读取机器人内部电池的剩余电量（百分比）"
    int device_battery(void);
    // 读取连接到机器人上的外部电池的电压值。
    float device_voltage(void);
    // 获取bot版本
    String device_version(void);


protected:
    int writeData(uint8_t dev_addr, uint8_t *data, uint16_t len) override;
    int readData(uint8_t dev_addr, uint8_t *data, uint16_t len) override;
    int writeReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len) override;
    int readReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len) override;
    int isOnline(uint8_t dev_addr) override;

private:
    // 0 退出等待  1 进入等待 2 跳出等待
    volatile uint8_t motor_wait_break_flag = 0;
};

extern hw_main_board_c hw_main_board;

#endif /* __HW_MAIN_BOARD_H__ */
