/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-02-26 16:38:25
 * @LastEditTime : 2026-05-08 16:52:49
 * @Description  : 测试文件, 对底板进行测试
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "TinkerCode.h"

#include "app_fluid_oled.h"

/******************
 * data struct
 *****************/
#define LOG_TAG "test"

#if 1
/****************************
 * function declaration
 ***************************/
static void    ptask_event_callback(ptask_t *task, ptask_event_t *e);
static void    timer_callback(zst_timer_t *timer);
static void    button_callback(zst_event_t *e);
static uint8_t read_pin_a(void);
static uint8_t read_pin_b(void);


/********************
 * static variables
 *******************/
static uint8_t      button_a_val = 0, button_b_val = 0;
static zst_button_t button_a = {0};
static zst_button_t button_b = {0};


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void test_init(void)
{
    /***************
     * 注册按键
     **************/
    zst_button_init_t button_i = {0};

    button_i.btn      = &button_a;
    button_i.read_pin = read_pin_a;
    zst_button_init(&button_i);

    button_i.btn      = &button_b;
    button_i.read_pin = read_pin_b;
    zst_button_init(&button_i);

    button_a.user_data = &button_a_val;
    button_b.user_data = &button_b_val;

    zst_target_add_event_cb(&button_a, button_callback, BTN_EVENT_CLICK, &button_a);
    zst_target_add_event_cb(&button_b, button_callback, BTN_EVENT_CLICK, &button_b);


    /***************
     * 创建任务
     **************/
    ptask_1_collection.test =
        ptask_create(ptask_root_1_collection.ptask_root_1, ptask_event_callback, NULL);
    if (NULL == ptask_1_collection.test)
        ZST_LOGE(LOG_TAG, "create ptask failed");
    else
        ZST_LOGI(LOG_TAG, "create ptask success");


    /***************
     * 创建定时回调
     **************/
    if (NULL == zst_timer_create(&zst_ztimer, timer_callback, 200, NULL))
    {
        ZST_LOGE(LOG_TAG, "zst_timer_create failed");
    } else
    {
        ZST_LOGI(LOG_TAG, "zst_timer_create success");
    }

    // app_fluid_oled_begin();
}


/****************************
 * static function
 ***************************/
static void ptask_run_callback(ptask_t *ptask);

static void ptask_event_callback(ptask_t *task, ptask_event_t *e)
{
    switch (ptask_get_code(e))
    {
        case PTASK_EVENT_RUN: ptask_run_callback(task); break;
        default: break;
    }
}

// 任务运行回调
static void ptask_run_callback(ptask_t *ptask)
{
    zst_button_irq_handler(&button_a);
    zst_button_irq_handler(&button_b);
    zst_button_process(&button_a);
    zst_button_process(&button_b);
}

// 按键回调
static void button_callback(zst_event_t *e)
{
    zst_button_t *btn     = (zst_button_t *)zst_event_get_user_data(e);
    uint8_t      *btn_val = (uint8_t *)btn->user_data;

    if (btn == &button_a)
    {
        if_uart_comm.send_bytes("button_a press");

    } else if (btn == &button_b)
    {
        if_uart_comm.send_bytes("button_b press");
    }
}

// 定时器回调
static void timer_callback(zst_timer_t *timer)
{
    hw_esp_oled.refresh();
    hw_esp_oled.clear_screen();
    static char display_buffer[70] = {0};
    // app_fluid_oled_update();

        
    if_uart_comm.send_bytes("ok\n");
}


static uint8_t read_pin_a(void)
{
    if (0 == hw_pin.digitalRead(BOARD_BUTTON_A_PIN)) { return ZST_BTN_ACTIVE_LEVEL; }
    return !ZST_BTN_ACTIVE_LEVEL;
}

static uint8_t read_pin_b(void)
{
    if (0 == hw_pin.digitalRead(BOARD_BUTTON_B_PIN)) { return ZST_BTN_ACTIVE_LEVEL; }
    return !ZST_BTN_ACTIVE_LEVEL;
}
#endif