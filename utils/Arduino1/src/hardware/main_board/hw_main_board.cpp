/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-15 15:44:10
 * @LastEditTime : 2026-04-30 12:00:35
 * @Description  : 主板、超声波、四路巡线
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hw_main_board.h"
#include "interface/i2c/if_i2c.h"
#include "math.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "hw_main_board"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
hw_main_board_c hw_main_board;


/*******************
 * class functions
 *******************/
hw_main_board_c::hw_main_board_c()
{}

hw_main_board_c::~hw_main_board_c()
{}

void hw_main_board_c::begin(void)
{}

int hw_main_board_c::writeData(uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    return if_i2c_internal_handle.write_bytes(dev_addr, data, len);
}

int hw_main_board_c::readData(uint8_t dev_addr, uint8_t *data, uint16_t len)
{
    return if_i2c_internal_handle.read_bytes(dev_addr, data, len);
}

int hw_main_board_c::writeReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return if_i2c_internal_handle.write_reg(dev_addr, reg, data, len);
}

int hw_main_board_c::readReg(uint8_t dev_addr, uint8_t reg, uint8_t *data, uint16_t len)
{
    return if_i2c_internal_handle.read_reg(dev_addr, reg, data, len);
}

int hw_main_board_c::isOnline(uint8_t dev_addr)
{
    return 0;
}

/****************************************
 *            双电机控制
 ***************************************/
// 等待完成动作。-1表示一直等待; >=0 表示等待时间 ms
int hw_main_board_c::movemen_wait_finsh(int timeout)
{
    delay(100);
    uint32_t last_tick = zst_tick_get();
    uint8_t running = 1;
    int ret = 0;
    while (1)
    {
        ret = s4s_mainBoard::encoder_motor_pair_get_action_runing(&running);
        if (ret < 0)
            return ret;
        if (0 == running)
            break;
        if (timeout >= 0 && zst_tick_elaps(last_tick) > timeout)
            break;
    }
    return ret;
}

// 设置电机端口 0~3
int hw_main_board_c::movement_set_motors(int left, int right)
{
    DATA_CHECK(left, 0, 3);
    DATA_CHECK(right, 0, 3);
    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_pair_set_group(left, right);
    return ret_error;
}


// 立即开始沿选定的方向移动机器人  0前进；1后退；2左转；3右转
int hw_main_board_c::movement_start(int dir)
{
    DATA_CHECK(dir, 0, 3);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_pair_set_action(dir + 1);

    return ret_error;
}


/**
 * @description: 在指定的时间或距离内向选定的方向移动
 * @param dir   0前进；1后退；2左转；3右转
 * @param value 0~1000
 * @param unit  0~3（0秒，1圈，2厘米）
 */
int hw_main_board_c::movement_move(int dir, int value, int unit)
{
    DATA_CHECK(dir, 0, 3);
    DATA_CHECK(value, 0, 1000);
    DATA_CHECK(unit, 0, 3);

    int ret_error = 0;
    if (0 == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_pair_set_time(value);
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(dir + 5);
    } else if (1 == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_pair_set_ring(value);
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(dir + 9);
    } else if (2 == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_pair_set_centimeter(value);
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(dir + 13);
    }
    ret_error += movemen_wait_finsh();
    return ret_error;
}


// 设定左右电机的速度，并立即开始移动。 -100 ~ 100
int hw_main_board_c::movement_drive(int lspeed, int rspeed)
{
    DATA_CHECK(lspeed, -100, 100);
    DATA_CHECK(rspeed, -100, 100);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_pair_set_dynamic_speed(abs(lspeed), abs(rspeed));
    if (lspeed >= 0 && rspeed >= 0)
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(1);
    else if (lspeed < 0 && rspeed < 0)
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(2);
    else if (lspeed < 0 && rspeed >= 0)
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(3);
    else if (lspeed >= 0 && rspeed < 0)
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(4);

    return ret_error;
}


/**
 * @description: 按照指定的速度持续运行一段固定的时间或距离
 * @param lspeed 0~100
 * @param rspeed 0~100
 * @param data   0~1000
 * @param unit  0~3（0秒，1圈，2厘米)
 */
int hw_main_board_c::movement_drive_for(int lspeed, int rspeed, int data, int unit)
{
    DATA_CHECK(lspeed, 0, 100);
    DATA_CHECK(rspeed, 0, 100);
    DATA_CHECK(data, 0, 1000);
    DATA_CHECK(unit, 0, 2);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_pair_set_dynamic_speed(lspeed, rspeed);
    if (0 == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_pair_set_time(data);
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(5);
    } else if (1 == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_pair_set_ring(data);
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(9);
    } else if (2 == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_pair_set_centimeter(data);
        ret_error += s4s_mainBoard::encoder_motor_pair_set_action(13);
    }

    return ret_error;
}


// 停止机器人运动
int hw_main_board_c::movement_stop(void)
{
    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_pair_set_action(0);
    return ret_error;
}


// 将两个电机调至相同转速，但不启动运动。
int hw_main_board_c::movement_set_speed(int speed)
{
    DATA_CHECK(speed, 0, 100);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_pair_set_dynamic_speed(speed, speed);
    return ret_error;
}

/****************************************
 *            单电机控制
 ***************************************/
int hw_main_board_c::motors_wait_finsh(int motor, int timeout)
{
    DATA_CHECK(motor, 0, 3);
    
    delay(100);
    uint32_t last_tick = zst_tick_get();
    uint8_t running = 1;
    int ret = 0;
    while (1)
    {
        ret = s4s_mainBoard::encoder_motor_get_action_runing(motor, &running);
        if (ret < 0)
            return ret;
        if (0 == running)
            break;
        if (timeout >= 0 && zst_tick_elaps(last_tick) > timeout)
            break;
    }
    return ret;
}

/**
 * @description: 按照选定的方向运行指定的电机端口，持续一定时间或完成指定的旋转次数。
 * @param motor 0~3
 * @param dir   0正转 1反转
 * @param data  0~1000
 * @param unit 0~3（0圈,1度,2秒,3厘米）
 */
int hw_main_board_c::motors_run_for(int motor, int dir, int data, int unit)
{
    DATA_CHECK(motor, 0, 3);
    DATA_CHECK(dir, 0, 1);
    DATA_CHECK(data, 0, 1000);
    DATA_CHECK(unit, 0, 3);

    int ret_error = 0;
    if (MOTORS_ROTATIONS == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_set_ring(motor, data);
        ret_error += s4s_mainBoard::encoder_motor_set_action(motor, dir + 7);
    } else if (MOTORS_DEGREES == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_set_relative_angle(motor, data);
        ret_error += s4s_mainBoard::encoder_motor_set_action(motor, dir + 9);
    } else if (MOTORS_SECONDS == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_set_time(motor, data);
        ret_error += s4s_mainBoard::encoder_motor_set_action(motor, dir + 11);
    } else if (MOTORS_CENTIMETERS == unit)
    {
        ret_error += s4s_mainBoard::encoder_motor_set_centimeter(motor, data);
        ret_error += s4s_mainBoard::encoder_motor_set_action(motor, dir + 13);
    }
    ret_error += motors_wait_finsh(motor);
    return ret_error;
}


// 在指定端口控制电机，使其立即以选定方向开始持续旋转。 0正转，1反转
int hw_main_board_c::motors_start(int motor, int dir)
{
    DATA_CHECK(motor, 0, 3);
    DATA_CHECK(dir, 0, 1);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_set_action(motor, dir + 3);
}


// 停止指定的电机。
int hw_main_board_c::motors_stop(int motor)
{
    DATA_CHECK(motor, 0, 3);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_set_action(motor, 0);
    return ret_error;
}


// 在不启动电机的情况下设定其转速
int hw_main_board_c::motors_set_speed(int motor, int speed)
{
    DATA_CHECK(motor, 0, 3);
    DATA_CHECK(speed, 0, 100);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_set_dynamic_speed(motor, speed);
    return ret_error;
}


// 返回指定电机的当前位置
int hw_main_board_c::motors_position(int motor)
{
    DATA_CHECK(motor, 0, 3);

    int     ret_error = 0;
    int32_t angle;
    ret_error += s4s_mainBoard::encoder_motor_get_angle(motor, &angle);

    if (ret_error == 0)
        return angle;
    else
        return ret_error;
}


// 获取当前速度百分比 0~100
int hw_main_board_c::motors_speed(int motor)
{
    DATA_CHECK(motor, 0, 3);

    int     ret_error = 0;
    int16_t speed;
    ret_error += s4s_mainBoard::encoder_motor_get_dynamic_speed(motor, &speed);

    return speed;
}


// 将指定端口上的电机位置计数器重置为 0 。
int hw_main_board_c::motors_reset_position(int motor)
{
    DATA_CHECK(motor, 0, 3);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_reset_angle(motor);
    return ret_error;
}


/**
 * @description: 在指定端口控制电机，使其立即以指定的目标转速（每分钟转数）开始运行。
 * @param motor  0~3
 * @param rpm   -180~180
 */
int hw_main_board_c::motors_start_rpm(int motor, int rpm)
{
    DATA_CHECK(motor, 0, 3);
    DATA_CHECK(rpm, -180, 180);

    int ret_error = 0;
    ret_error += s4s_mainBoard::encoder_motor_set_rpm_speed(motor, abs(rpm));
    if (rpm > 0)
        ret_error += s4s_mainBoard::encoder_motor_set_action(motor, 1);
    else
        ret_error += s4s_mainBoard::encoder_motor_set_action(motor, 2);
    return ret_error;
}


// 获取指定端口处电机的当前转速（单位：转/分钟）
int hw_main_board_c::motors_rpm(int motor)
{
    DATA_CHECK(motor, 0, 3);

    int     ret_error = 0;
    int16_t rpm;
    ret_error += s4s_mainBoard::encoder_motor_get_rpm_speed(motor, &rpm);

    return rpm;
}


/****************************************
 *            舵机控制
 ***************************************/
// 将标准伺服器旋转至指定角度 0 ~ 1, 0 ~ 180
int hw_main_board_c::servo_set_angle(int servo, int angle)
{
    DATA_CHECK(servo, 0, 1);
    DATA_CHECK(angle, 0, 180);

    int ret_error = 0;
    ret_error += s4s_mainBoard::servo_set_angle(servo, angle);
    return ret_error;
}


// 将伺服器从接口上拆下，以释放扭矩。
int hw_main_board_c::servo_release(int servo)
{
    DATA_CHECK(servo, 0, 1);

    int ret_error = 0;
    ret_error += s4s_mainBoard::servo_release(servo);
    return ret_error;
}


// 以指定的速度运行连续旋转伺服装置 0 ~ 1, -100 ~ 100
int hw_main_board_c::servo_set_speed(int servo, int speed)
{
    DATA_CHECK(servo, 0, 1);
    DATA_CHECK(speed, -100, 100);

    int ret_error = 0;
    ret_error += s4s_mainBoard::continuous_servo_set_speed(servo, speed);
    return ret_error;
}


// 停止连续伺服模式
int hw_main_board_c::servo_stop(int servo)
{
    DATA_CHECK(servo, 0, 1);

    int ret_error = 0;
    ret_error += s4s_mainBoard::continuous_servo_set_speed(servo, 0);
    return ret_error;
}

/****************************************
 *            氛围灯控制
 ***************************************/
// 设置机器人的环境灯光颜色 0 ~ 255
int hw_main_board_c::light_set_color(int r, int g, int b, int light)
{
    DATA_CHECK(r, 0, 255);
    DATA_CHECK(g, 0, 255);
    DATA_CHECK(b, 0, 255);
    DATA_CHECK(light, 0, 255);

    int ret_error = 0;
    ret_error += s4s_mainBoard::ambient_light_set_state(light, r, g, b);
    return ret_error;
}


// 设置机器人的环境灯亮度 0 ~ 255
int hw_main_board_c::light_set_brightness(int light)
{
    DATA_CHECK(light, 0, 255);

    int ret_error = 0;
    ret_error += s4s_mainBoard::ambient_light_set_state(light, nullptr);
    return ret_error;
}

/****************************************
 *            语音识别
 ***************************************/
int hw_main_board_c::voice_recognized(int recognized)
{
    uint8_t state;
    if (0 != s4s_mainBoard::voice_get_state(&state)) return -1;

    if (state == recognized) return 1;
    else return 0;
}

String hw_main_board_c::voice_version(void)
{
    String version_str;
    uint8_t version[3];
    if (0 == s4s_mainBoard::voice_get_version(version))
    {
        version_str = String(version[0]) + "." + String(version[1]) + "." + String(version[2]);
    }

    return version_str;
}

/****************************************
 *               RTC
 ***************************************/
// 设置日期 0 ~ 99, 1 ~ 12, 1 ~ 31
int hw_main_board_c::rtc_set_date(int year, int month, int day)
{
    DATA_CHECK(year, 0, 99);
    DATA_CHECK(month, 1, 12);
    DATA_CHECK(day, 1, 31);

    int ret_error = 0;
    ret_error += s4s_mainBoard::rtc_set_date(year, month, day);
    return ret_error;
}


// 设置时间 0 ~ 23, 0 ~ 59, 0 ~ 59
int hw_main_board_c::rtc_set_time(int hour, int minute, int second)
{
    DATA_CHECK(hour, 0, 23);
    DATA_CHECK(minute, 0, 59);
    DATA_CHECK(second, 0, 59);

    int ret_error = 0;
    ret_error += s4s_mainBoard::rtc_set_time(hour, minute, second);
    return ret_error;
}


// 获取当前时间  (0年，1月，2日，3周，4时，5分，6秒)
int hw_main_board_c::rtc_get(int sel)
{
    DATA_CHECK(sel, 0, 6);

    int ret_error = -1;
    if (sel <= 3)
    {
        uint8_t year, month, day, week;
        ret_error = s4s_mainBoard::rtc_get_data(&year, &month, &day, &week);
        if (0 == ret_error)
        {
            if (sel == RTC_YEAR)
                ret_error = year;
            else if (sel == RTC_MONTH)
                ret_error = month;
            else if (sel == RTC_DAY)
                ret_error = day;
            else if (sel == RTC_WEEK)
                ret_error = week;
        }
    } else
    {
        uint8_t hour=0, minute=0, second=0;
        ret_error = s4s_mainBoard::rtc_get_time(&hour, &minute, &second);
        if (0 == ret_error)
        {
            if (sel == RTC_HOUR)
                ret_error = hour;
            else if (sel == RTC_MINUTE)
                ret_error = minute;
            else if (sel == RTC_SECOND) 
                ret_error = second;
        }
    }
    return ret_error;
}


/****************************************
 *               power
 ***************************************/
// "读取特定的时间值  读取机器人内部电池的剩余电量（百分比）"
int hw_main_board_c::device_battery(void)
{
    int     ret_error = 0;
    uint8_t level;
    ret_error += s4s_mainBoard::power_get_internal_battery_level(&level);
    if (0 == ret_error)
        return level;
    else
        return ret_error;
}


// 读取连接到机器人上的外部电池的电压值。
float hw_main_board_c::device_voltage(void)
{
    int   ret_error = 0;
    float voltage;
    ret_error += s4s_mainBoard::power_get_external_battery_voltage(&voltage);
    if (0 == ret_error)
        return voltage;
    else
        return ret_error;
}


// 获取bot版本
String hw_main_board_c::device_version(void)
{
    String  ret_error = "";
    uint8_t version[3];
    if (0 == s4s_mainBoard::version_get(version))
    {
        ret_error = String(version[0]) + "." + String(version[1]) + "." + String(version[2]);
    }
    return ret_error;
}
