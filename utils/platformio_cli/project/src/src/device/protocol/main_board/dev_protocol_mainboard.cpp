/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-28 15:45:07
 * @LastEditTime : 2026-07-24 08:42:39
 * @Description  : mainboard 协议处理模块
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol_mainboard.h"
#include "hardware/main_board/hw_main_board.h"
#include "map"

/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol_mainboard"


/****************************
 * function declaration
 ***************************/
static fmap_result_t fMap_restore_default(udc_pack_t *pack);

static fmap_result_t fMap_movement_set_motors(udc_pack_t *pack);
static fmap_result_t fMap_movement_start(udc_pack_t *pack);
static fmap_result_t fMap_movement_move(udc_pack_t *pack);
static fmap_result_t fMap_movement_drive(udc_pack_t *pack);
static fmap_result_t fMap_movement_drive_for(udc_pack_t *pack);
static fmap_result_t fMap_movement_stop(udc_pack_t *pack);
static fmap_result_t fMap_movement_set_speed(udc_pack_t *pack);

static fmap_result_t fMap_motors_run_for(udc_pack_t *pack);
static fmap_result_t fMap_motors_start(udc_pack_t *pack);
static fmap_result_t fMap_motors_stop(udc_pack_t *pack);
static fmap_result_t fMap_motors_set_speed(udc_pack_t *pack);
static fmap_result_t fMap_motors_position(udc_pack_t *pack);
static fmap_result_t fMap_motors_speed(udc_pack_t *pack);
static fmap_result_t fMap_motors_reset_position(udc_pack_t *pack);
static fmap_result_t fMap_motors_start_rpm(udc_pack_t *pack);
static fmap_result_t fMap_motors_rpm(udc_pack_t *pack);

static fmap_result_t fMap_servo_set_angle(udc_pack_t *pack);
static fmap_result_t fMap_servo_release(udc_pack_t *pack);
static fmap_result_t fMap_servo_set_speed(udc_pack_t *pack);
static fmap_result_t fMap_servo_stop(udc_pack_t *pack);

static fmap_result_t fMap_light_set_color(udc_pack_t *pack);
static fmap_result_t fMap_light_set_brightness(udc_pack_t *pack);

static fmap_result_t fMap_voice_recognized(udc_pack_t *pack);
static fmap_result_t fMap_voice_version(udc_pack_t *pack);

static fmap_result_t fMap_rtc_set_date(udc_pack_t *pack);
static fmap_result_t fMap_rtc_set_time(udc_pack_t *pack);
static fmap_result_t fMap_rtc_get(udc_pack_t *pack);

static fmap_result_t fMap_device_battery(udc_pack_t *pack);
static fmap_result_t fMap_device_voltage(udc_pack_t *pack);
static fmap_result_t fMap_device_version(udc_pack_t *pack);


/********************
 * static variables
 *******************/
static const function_map_t sys_function_map = {
    // clang-format off
    {"restore_default",         fMap_restore_default},
    // clang-format on
};

static const function_map_t function_map = {
    // clang-format off
    {"movement_set_motors",     fMap_movement_set_motors},
    {"movement_start",          fMap_movement_start},
    {"movement_move",           fMap_movement_move},
    {"movement_drive",          fMap_movement_drive},
    {"movement_drive_for",      fMap_movement_drive_for},
    {"movement_stop",           fMap_movement_stop},
    {"movement_set_speed",      fMap_movement_set_speed},

    {"motors_run_for",          fMap_motors_run_for},
    {"motors_start",            fMap_motors_start},
    {"motors_stop",             fMap_motors_stop},
    {"motors_set_speed",        fMap_motors_set_speed},
    {"motors_position",         fMap_motors_position},
    {"motors_speed",            fMap_motors_speed},
    {"motors_reset_position",   fMap_motors_reset_position},
    {"motors_start_rpm",        fMap_motors_start_rpm},
    {"motors_rpm",              fMap_motors_rpm},

    {"servo_set_angle",         fMap_servo_set_angle},
    {"servo_release",           fMap_servo_release},
    {"servo_set_speed",         fMap_servo_set_speed},
    {"servo_stop",              fMap_servo_stop},

    {"light_set_color",         fMap_light_set_color},
    {"light_set_brightness",    fMap_light_set_brightness},

    {"voice_recognized",        fMap_voice_recognized},
    {"voice_version",           fMap_voice_version},

    {"rtc_set_date",            fMap_rtc_set_date},
    {"rtc_set_time",            fMap_rtc_set_time},
    {"rtc_get",                 fMap_rtc_get},

    {"device_battery",           fMap_device_battery},
    {"device_voltage",           fMap_device_voltage},
    {"device_version",           fMap_device_version},
    // clang-format on
};

static const enum_map_t enum_map = {
    // clang-format off
    {"MOVEMENT_FORWARD",   (int)hw_main_board.MOVEMENT_FORWARD},
    {"MOVEMENT_BACKWARD",  (int)hw_main_board.MOVEMENT_BACKWARD},
    {"MOVEMENT_LEFT",      (int)hw_main_board.MOVEMENT_LEFT},
    {"MOVEMENT_RIGHT",     (int)hw_main_board.MOVEMENT_RIGHT},

    {"MOVEMENT_SECONDS",     (int)hw_main_board.MOVEMENT_SECONDS},
    {"MOVEMENT_ROTATIONS",   (int)hw_main_board.MOVEMENT_ROTATIONS},
    {"MOVEMENT_CENTIMETERS", (int)hw_main_board.MOVEMENT_CENTIMETERS},

    {"MOTORS_FORWARD",  (int)hw_main_board.MOTORS_FORWARD},
    {"MOTORS_BACKWARD", (int)hw_main_board.MOTORS_BACKWARD},

    {"MOTORS_ROTATIONS",    (int)hw_main_board.MOTORS_ROTATIONS},
    {"MOTORS_DEGREES",      (int)hw_main_board.MOTORS_DEGREES},
    {"MOTORS_SECONDS",      (int)hw_main_board.MOTORS_SECONDS},
    {"MOTORS_CENTIMETERS",  (int)hw_main_board.MOTORS_CENTIMETERS},

    {"RTC_YEAR",       (int)hw_main_board.RTC_YEAR},
    {"RTC_MONTH",      (int)hw_main_board.RTC_MONTH},
    {"RTC_DAY",        (int)hw_main_board.RTC_DAY},
    {"RTC_WEEK",       (int)hw_main_board.RTC_WEEK},
    {"RTC_HOUR",       (int)hw_main_board.RTC_HOUR},
    {"RTC_MINUTE",     (int)hw_main_board.RTC_MINUTE},
    {"RTC_SECOND",     (int)hw_main_board.RTC_SECOND},

    {"VOICE_UNKNOWN",           (int)hw_main_board.VOICE_UNKNOWN}, 
    {"VOICE_LINKBOT",           (int)hw_main_board.VOICE_LINKBOT}, 
    {"VOICE_GO_FORWARD",        (int)hw_main_board.VOICE_GO_FORWARD}, 
    {"VOICE_GO_BACK",           (int)hw_main_board.VOICE_GO_BACK}, 
    {"VOICE_TURN_LEFT",         (int)hw_main_board.VOICE_TURN_LEFT}, 
    {"VOICE_TURN_RIGHT",        (int)hw_main_board.VOICE_TURN_RIGHT}, 
    {"VOICE_START_MOVING",      (int)hw_main_board.VOICE_START_MOVING}, 
    {"VOICE_STOP_MOVING",       (int)hw_main_board.VOICE_STOP_MOVING}, 
    {"VOICE_SPEED_UP",          (int)hw_main_board.VOICE_SPEED_UP}, 
    {"VOICE_SLOW_DOWN",         (int)hw_main_board.VOICE_SLOW_DOWN}, 
    {"VOICE_SPIN_AROUND",       (int)hw_main_board.VOICE_SPIN_AROUND}, 
    {"VOICE_FOLLOW_LINE",       (int)hw_main_board.VOICE_FOLLOW_LINE}, 
    {"VOICE_LIGHTS_ON",         (int)hw_main_board.VOICE_LIGHTS_ON}, 
    {"VOICE_LIGHTS_OFF",        (int)hw_main_board.VOICE_LIGHTS_OFF}, 
    {"VOICE_SHOW_RED",          (int)hw_main_board.VOICE_SHOW_RED}, 
    {"VOICE_SHOW_GREEN",        (int)hw_main_board.VOICE_SHOW_GREEN}, 
    {"VOICE_SHOW_BLUE",         (int)hw_main_board.VOICE_SHOW_BLUE}, 
    {"VOICE_CHECK_DISTANCE",    (int)hw_main_board.VOICE_CHECK_DISTANCE}, 
    {"VOICE_SCAN_SURROUNDINGS", (int)hw_main_board.VOICE_SCAN_SURROUNDINGS}, 
    {"VOICE_DETECT_OBJECT",     (int)hw_main_board.VOICE_DETECT_OBJECT}, 
    {"VOICE_DETECT_FACE",       (int)hw_main_board.VOICE_DETECT_FACE}, 
    {"VOICE_DETECT_CARD",       (int)hw_main_board.VOICE_DETECT_CARD}, 
    {"VOICE_READ_SENSOR",       (int)hw_main_board.VOICE_READ_SENSOR}, 
    {"VOICE_CHECK_BATTERY",     (int)hw_main_board.VOICE_CHECK_BATTERY}, 
    {"VOICE_FIND_LINE",         (int)hw_main_board.VOICE_FIND_LINE}, 
    {"VOICE_WHO_AM_I",          (int)hw_main_board.VOICE_WHO_AM_I}, 
    {"VOICE_WHO_ARE_YOU",       (int)hw_main_board.VOICE_WHO_ARE_YOU}, 
    {"VOICE_START",             (int)hw_main_board.VOICE_START}, 
    {"VOICE_STOP",              (int)hw_main_board.VOICE_STOP}, 
    {"VOICE_REPEAT",            (int)hw_main_board.VOICE_REPEAT}, 
    {"VOICE_NEXT",              (int)hw_main_board.VOICE_NEXT}, 
    {"VOICE_RETURN_HOME",       (int)hw_main_board.VOICE_RETURN_HOME}, 
    {"VOICE_PICK_UP",           (int)hw_main_board.VOICE_PICK_UP}, 
    {"VOICE_PUT_DOWN",          (int)hw_main_board.VOICE_PUT_DOWN}, 
    {"VOICE_SLEEP",             (int)hw_main_board.VOICE_SLEEP}, 
    {"VOICE_WAKE_UP",           (int)hw_main_board.VOICE_WAKE_UP}, 
    {"VOICE_PLAY_MUSIC",        (int)hw_main_board.VOICE_PLAY_MUSIC}, 
    {"VOICE_RECORD_DATA",       (int)hw_main_board.VOICE_RECORD_DATA}, 
    {"VOICE_SEND_MESSAGE",      (int)hw_main_board.VOICE_SEND_MESSAGE}, 
    {"VOICE_RECEIVE_MESSAGE",   (int)hw_main_board.VOICE_RECEIVE_MESSAGE}, 
    {"VOICE_ATTACK_NOW",        (int)hw_main_board.VOICE_ATTACK_NOW}, 
    {"VOICE_DEFEND_YOURSELF",   (int)hw_main_board.VOICE_DEFEND_YOURSELF}, 
    {"VOICE_SHOW_ME_A_DANCE",   (int)hw_main_board.VOICE_SHOW_ME_A_DANCE}, 
    {"VOICE_TELL_ME_A_JOKE",    (int)hw_main_board.VOICE_TELL_ME_A_JOKE}, 
    // clang-format on
};


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_mainBoard_init(void)
{
    function_map_collection_main["bot"] = &function_map;
    function_map_collection_sys["bot"] = &sys_function_map;
}

/****************************
 * static function
 ***************************/

/****************
 *     sys
 ***************/
static fmap_result_t fMap_restore_default(udc_pack_t *pack)
{
    hw_main_board.restore_default();
    return fmap_result_t::make_ok();
}


/****************
 *     main
 ***************/
static fmap_result_t fMap_movement_set_motors(udc_pack_t *pack)
{
    int left, right;
    int ret_error = 0;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &left);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &right);

    ret_error = hw_main_board.movement_set_motors(left, right);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_movement_start(udc_pack_t *pack)
{
    int         ret_error = 0;
    std::string direction;
    int         direction_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &direction);
    enum_map_get_value(enum_map, direction.c_str(), direction_int);

    ret_error = hw_main_board.movement_start(direction_int);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_movement_move(udc_pack_t *pack)
{
    int ret_error = 0;
    std::string direction, unit;
    int direction_int, value_int, unit_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &direction);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &value_int);
    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(2), &unit);

    enum_map_get_value(enum_map, direction.c_str(), direction_int);
    enum_map_get_value(enum_map, unit.c_str(), unit_int);

    ret_error = hw_main_board.movement_move(direction_int, value_int, unit_int);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_movement_drive(udc_pack_t *pack)
{
    int ret_error = 0;
    int lspeed, rspeed;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &lspeed);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &rspeed);

    ret_error = hw_main_board.movement_drive(lspeed, rspeed);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_movement_drive_for(udc_pack_t *pack)
{
    int ret_error = 0;
    std::string unit;
    int unit_int, lspeed_int, rspeed_int, data_int;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &lspeed_int);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &rspeed_int);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &data_int);
    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(3), &unit);
    enum_map_get_value(enum_map, unit.c_str(), unit_int);

    ret_error = hw_main_board.movement_drive_for(lspeed_int, rspeed_int, data_int, unit_int);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_movement_stop(udc_pack_t *pack)
{
    int ret_error = 0;

    ret_error = hw_main_board.movement_stop();

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_movement_set_speed(udc_pack_t *pack)
{
    int ret_error = 0;
    int speed;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &speed);

    ret_error = hw_main_board.movement_set_speed(speed);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_motors_run_for(udc_pack_t *pack)
{
    int ret_error = 0;
    std::string unit, direction;
    int motor_int, direction_int, data_int, unit_int;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor_int);
    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(1), &direction);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &data_int);
    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(3), &unit);

    enum_map_get_value(enum_map, direction.c_str(), direction_int);
    enum_map_get_value(enum_map, unit.c_str(), unit_int);

    ret_error = hw_main_board.motors_run_for(motor_int, direction_int, data_int, unit_int);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_motors_start(udc_pack_t *pack)
{
    int ret_error = 0;
    std::string direction;
    int motor_int, direction_int;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor_int);
    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(1), &direction);

    enum_map_get_value(enum_map, direction.c_str(), direction_int);

    ret_error = hw_main_board.motors_start(motor_int, direction_int);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_motors_stop(udc_pack_t *pack)
{
    int ret_error = 0;
    int motor;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor);

    ret_error = hw_main_board.motors_stop(motor);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_motors_set_speed(udc_pack_t *pack)
{
    int ret_error = 0;
    int motor, speed;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &speed);

    ret_error = hw_main_board.motors_set_speed(motor, speed);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_motors_position(udc_pack_t *pack)
{
    int ret_error = 0;
    int motor;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor);

    ret_error = hw_main_board.motors_position(motor);

    return fmap_result_t::make_ok().add_int(ret_error);
}

static fmap_result_t fMap_motors_speed(udc_pack_t *pack)
{
    int ret_error = 0;
    int motor;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor);

    ret_error = hw_main_board.motors_speed(motor);

    return fmap_result_t::make_ok().add_int(ret_error);
}

static fmap_result_t fMap_motors_reset_position(udc_pack_t *pack)
{
    int ret_error = 0;
    int motor;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor);

    ret_error = hw_main_board.motors_reset_position(motor);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_motors_start_rpm(udc_pack_t *pack)
{
    int ret_error = 0;
    int motor, rpm;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &rpm);

    ret_error = hw_main_board.motors_start_rpm(motor, rpm);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_motors_rpm(udc_pack_t *pack)
{
    int ret_error = 0;
    int motor;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &motor);

    ret_error = hw_main_board.motors_rpm(motor);

    return fmap_result_t::make_ok().add_int(ret_error);
}

static fmap_result_t fMap_servo_set_angle(udc_pack_t *pack)
{
    int ret_error = 0;
    int servo, angle;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &servo);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &angle);

    ret_error = hw_main_board.servo_set_angle(servo, angle);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_servo_release(udc_pack_t *pack)
{
    int ret_error = 0;
    int servo;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &servo);

    ret_error = hw_main_board.servo_release(servo);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_servo_set_speed(udc_pack_t *pack)
{
    int ret_error = 0;
    int servo, speed;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &servo);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &speed);

    ret_error = hw_main_board.servo_set_speed(servo, speed);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_servo_stop(udc_pack_t *pack)
{
    int ret_error = 0;
    int servo;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &servo);

    ret_error = hw_main_board.servo_stop(servo);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_light_set_color(udc_pack_t *pack)
{
    int ret_error = 0;
    int r, g, b, light = 255;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &r);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &g);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &b);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(0), &light);

    ret_error = hw_main_board.light_set_color(r, g, b, light);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_light_set_brightness(udc_pack_t *pack)
{
    int ret_error = 0;
    int light;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &light);

    ret_error = hw_main_board.light_set_brightness(light);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_voice_recognized(udc_pack_t *pack)
{
    int ret_error = 0;
    std::string recognized;
    int recognized_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &recognized);
    enum_map_get_value(enum_map, recognized.c_str(), recognized_int);
    ret_error = hw_main_board.voice_recognized(recognized_int);
    
    if (ret_error >= 0)
        return fmap_result_t::make_ok().add_int(ret_error);
    else
        return fmap_result_t::make_error(ret_error);
}

static fmap_result_t fMap_voice_version(udc_pack_t *pack)
{
    String version;

    version = hw_main_board.voice_version();
    if (version.length() > 0)
        return fmap_result_t::make_ok().add_string(version.c_str());
    else
        return fmap_result_t::make_error(-1);
}

static fmap_result_t fMap_rtc_set_date(udc_pack_t *pack)
{
    int ret_error = 0;
    int year, month, day;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &year);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &month);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(2), &day);

    ret_error = hw_main_board.rtc_set_date(year, month, day);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_rtc_set_time(udc_pack_t *pack)
{
    int ret_error = 0;
    int hour, minute, second;

    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(0), &hour);
    function_map_udcpack_get_param_int(pack, function_map_udcpack_id(1), &minute);
    function_map_udcpack_get_param_int_default(pack, function_map_udcpack_id(2), &second);

    ret_error = hw_main_board.rtc_set_time(hour, minute, second);

    return fmap_result_t::make_result(ret_error);
}

static fmap_result_t fMap_rtc_get(udc_pack_t *pack)
{
    int ret_error = 0;
    std::string select;
    int select_int;

    function_map_udcpack_get_param_string(pack, function_map_udcpack_id(0), &select);
    enum_map_get_value(enum_map, select.c_str(), select_int);
    ret_error = hw_main_board.rtc_get(select_int);
    if (ret_error >= 0)
        return fmap_result_t::make_ok().add_int(ret_error);
    else
        return fmap_result_t::make_error(ret_error);
}

static fmap_result_t fMap_device_battery(udc_pack_t *pack)
{
    int ret_error = 0;

    ret_error = hw_main_board.device_battery();
    if (ret_error >= 0)
        return fmap_result_t::make_ok().add_int(ret_error);
    else
        return fmap_result_t::make_error(ret_error);
}

static fmap_result_t fMap_device_voltage(udc_pack_t *pack)
{
    float ret_error = 0.0f;

    ret_error = hw_main_board.device_voltage();
    if (ret_error >= 0.0f)
        return fmap_result_t::make_ok().add_float(ret_error);
    else
        return fmap_result_t::make_error(ret_error);
}

static fmap_result_t fMap_device_version(udc_pack_t *pack)
{
    String version;

    version = hw_main_board.device_version();

    if (version.length() > 0)
        return fmap_result_t::make_ok().add_string(version.c_str());
    else
        return fmap_result_t::make_error(-1);
}
