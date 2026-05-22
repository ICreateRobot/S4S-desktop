/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-12-04 11:38:22
 * @LastEditTime : 2026-05-20 11:20:40
 * @Description  : esp oled 设备
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __HW_ESP_OLED_H__
#define __HW_ESP_OLED_H__

#include "main.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>


class hw_esp_oled_c : public Adafruit_SH1107
{
public:
    hw_esp_oled_c();
    ~hw_esp_oled_c();

public:
    /**
     * @description: 初始化
     */
    void begin(void);
    void init(void) {begin();}

    /**
     * @description: 清除屏幕缓冲区
     */
    void clear_screen(void);

    /**
     * @description: 设置字体大小
     * @param size  Desired text size. 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
     */
    void set_text_size(uint8_t size);

    /**
     * @description: 向屏幕中写入文本
     * @param x x坐标
     * @param y y坐标
     * @param text 文本内容
     */
    void print(int x, int y, const char *text);
    void print(int x, int y, int value);
    void print(int x, int y, String text);
    
    /**
     * @description: 点亮屏幕中的像素
     * @param x 像素的x坐标
     * @param y 像素的y坐标
     */    
    void draw_pixel(int x, int y);

    /**
     * @description: 画线
     * @param x0 开始坐标的x值
     * @param y0 开始坐标的y值
     * @param x1 结束坐标的x值
     * @param y1 结束坐标的y值
     */
    void draw_line(int x0, int y0, int x1, int y1);

    /**
     * @description: 画矩形（无填充）
     * @param x 矩形左上角的x坐标
     * @param y 矩形左上角的y坐标
     * @param w 矩形的宽度
     * @param h 矩形的高度
     */
    void draw_rect(int x, int y, int w, int h);

    /**
     * @description: 画圆形（无填充）
     * @param x0  圆心的x坐标
     * @param y0  圆心的y坐标
     * @param r 圆的半径
     */
    void draw_circle(int x0, int y0, int r);

    /**
     * @description: 刷新缓冲区的内容到屏幕
     */
    void refresh(void);

private:
    uint8_t is_init = 0; // 是否初始化
};


extern hw_esp_oled_c hw_esp_oled;

#endif /* __HW_ESP_OLED_H__ */
