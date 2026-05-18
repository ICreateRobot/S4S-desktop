/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-01-31 15:14:16
 * @LastEditTime : 2026-04-29 14:36:47
 * @Description  : 引脚
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hw_pin.h"
#include "device/protocol/common/dev_protocol_common.h"
#include "map"

/******************
 * data struct
 *****************/
#define LOG_TAG "hw_pin"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/
static const enum_map_t pin_map = {
    {"D38", 38},
    {"D37", 37},
    {"D36", 36},
    {"D35", 35},
    {"D34", 34},
    {"D33", 33},
    {"D32", 32},
    {"D31", 31},
    {"D30", 30},
    {"A0", A0},
    {"A1", A1},
    {"A2", A2},
    {"A3", A3},
    {"A5", A5},
    {"A4", A4},

    {"D29", 29},
    {"D28", 28},
    {"D13", 13},
    {"D12", 12},
    {"D11", 11},
    {"D10", 10},
    {"D9", 9},
    {"D8", 8},
    {"D7", 7},
    {"D6", 6},
    {"D5", 5},
    {"D4", 4},
    {"D3", 3},
    {"D2", 2},
    {"D1", 1},
    {"D0", 0},
};


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
hw_pin_c hw_pin;


/*******************
 * class functions
 *******************/
hw_pin_c::hw_pin_c(void)
{}
hw_pin_c::~hw_pin_c(void)
{}

int hw_pin_c::pinMode(const char * pin, int mode)
{
    int pin_id = 0;
    if (0 != enum_map_get_value_default(pin_map, pin, pin_id))
    {
        return -1;
    }
    ::pinMode(pin_id, (PinMode)mode);

    return 0;
}

int hw_pin_c::digitalWrite(const char * pin, int value)
{
    int pin_id = 0;
    if (0 != enum_map_get_value_default(pin_map, pin, pin_id))
    {
        return -1;
    }
    ::digitalWrite(pin_id, (PinStatus)value);

    return 0;
}

int hw_pin_c::digitalRead(const char * pin)
{
    int pin_id = 0;
    if (0 != enum_map_get_value_default(pin_map, pin, pin_id))
    {
        return -1;
    }

    return ::digitalRead(pin_id);
}

int hw_pin_c::analogWrite(const char * pin, int value)
{
    int pin_id = 0;
    if (0 != enum_map_get_value_default(pin_map, pin, pin_id))
    {
        return -1;
    }
    ::analogWrite(pin_id, value);
    return 0;
}

int hw_pin_c::analogRead(const char *pin)
{
    int pin_id = 0;
    if (0 != enum_map_get_value_default(pin_map, pin, pin_id))
    {
        return -1;
    }
    return ::analogRead(pin_id);
}

int hw_pin_c::pulseIn(const char *pin, int state, int timeout)
{
    int pin_id = 0;
    if (0 != enum_map_get_value_default(pin_map, pin, pin_id))
    {
        return -1;
    }
    return ::pulseIn(pin_id, state, timeout);
}

int hw_pin_c::button_pressed(void)
{
    int value = 0;
    if (0 == digitalRead(BOARD_BUTTON_A_PIN)) value |= 0x01;
    if (0 == digitalRead(BOARD_BUTTON_B_PIN)) value |= 0x02;
    return value;
}
