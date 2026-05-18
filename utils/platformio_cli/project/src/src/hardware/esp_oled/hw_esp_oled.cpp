/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-12-04 11:38:14
 * @LastEditTime : 2026-05-15 16:32:25
 * @Description  : esp oled 设备
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "hw_esp_oled.h"
#include <SPI.h>
#include <Wire.h>
#include "platform/pf_rtos.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "hw_esp_oled"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels
#define OLED_RESET -1     // can set an oled reset pin if desired


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


/*******************
 * class functions
 *******************/
hw_esp_oled_c::hw_esp_oled_c() :
    Adafruit_SH1107(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
{}

hw_esp_oled_c::~hw_esp_oled_c()
{}

void hw_esp_oled_c::begin(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    if (0 == is_init)
    {
        is_init = 1;
        Adafruit_SH1107::begin(0x3C, true);
    }
    setContrast(255); // 亮度
    setRotation(2);   // 旋转
    clearDisplay();   // 清屏
    display();
    setTextSize(1);
    setTextColor(SH110X_WHITE);
    setCursor(0, 0);

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

void hw_esp_oled_c::clear_screen(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    clearDisplay();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

void hw_esp_oled_c::set_text_size(uint8_t size)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    setTextSize(size);

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

void hw_esp_oled_c::print(int x, int y, const char *text)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    setCursor(x, y);
    Adafruit_SH1107::print(text);

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

void hw_esp_oled_c::draw_pixel(int x, int y)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    drawPixel(x, y, SH110X_WHITE);

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

void hw_esp_oled_c::draw_line(int x0, int y0, int x1, int y1)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    drawLine(x0, y0, x1, y1, SH110X_WHITE);

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

void hw_esp_oled_c::draw_rect(int x, int y, int w, int h)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    drawRect(x, y, w, h, SH110X_WHITE);

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

void hw_esp_oled_c::draw_circle(int x0, int y0, int r)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    drawCircle(x0, y0, r, SH110X_WHITE);

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}

void hw_esp_oled_c::refresh(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    display();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
}


hw_esp_oled_c hw_esp_oled;
