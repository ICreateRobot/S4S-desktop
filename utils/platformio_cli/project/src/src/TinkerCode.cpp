/*
 * @Author       : 蔡雅超 (zishen)
 * @LastEditors  : zishen
 * @Date         : 2026-04-27 19:49:10
 * @LastEditTime : 2026-07-23 16:06:12
 * @Description  : 在进入用户程序前进行的初始化操作
 * Copyright (c) 2026 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "TinkerCode.h"
#include "interface/interface.h"
#include "hardware/hardware.h"
#include "device/device.h"
#include "platform/pf_rtos.h"


/******************
 * data struct
 *****************/


/****************************
 * function declaration
 ***************************/
extern void app_setup(void);
extern void app_loop(void);
extern void test_init(void);

static void app_task_init(void *param);
static void app_task_main(void *param);
static void app_task_sys(void *param);
static void app_task_udcheck(void *param);


/********************
 * static variables
 *******************/


/********************
 * global variables
 *******************/

// 循环任务
ptask_root_1_collection_t ptask_root_1_collection = {0};
ptask_1_collection_t      ptask_1_collection      = {0};


// udcheck
udc_pack_group_t udcPackGroup_main = {0};
udc_pack_t       udcPack_main      = {0};

udc_pack_group_t udcPackGroup_sys = {0};
udc_pack_t       udcPack_sys      = {0};


/********************
 * global functions
 *******************/
void setup(void)
{
    delay(1000);

    taskENTER_CRITICAL();
    {
        // clang-format off
        xTaskCreate((TaskFunction_t ) app_task_init,      //任务函数
                    (const char*    ) "main",             //任务名称
                    (uint16_t       ) 500,                //任务堆栈大小(单位字)
                    (void*          ) nullptr,            //传递给任务函数的参数
                    (UBaseType_t    ) 1,                  //任务优先级
                    (TaskHandle_t*  ) NULL);              //任务句柄
        // clang-format on
    }
    taskEXIT_CRITICAL();

    // 开始任务调度
    vTaskStartScheduler();
    while (1);
}

void loop(void)
{

}

static void app_task_init(void *param)
{
    /*********************
     * 硬件初始化
     ********************/
    zst_init();

    /*******************
     * 系统结构初始化
     ******************/
    ptask_root_1_collection.ptask_root_1 = ptask_root_create(&zst_ptask_list); // 创建根任务
    ptask_root_select(&zst_ptask_list, ptask_root_1_collection.ptask_root_1);  // 选择根任务


    /*******************
     * 设备初始化
     * ****************/
    interface_init();
    pf_rtos_init();
    hardware_init();
    device_init();

    // test_init();
    app_setup();

    taskENTER_CRITICAL();
    {
        // clang-format off
#ifdef NOT_CUSTOM_BUILD
        xTaskCreate((TaskFunction_t ) app_task_main,      //任务函数
                    (const char*    ) "main",             //任务名称
                    (uint16_t       ) 512,                //任务堆栈大小(单位字)
                    (void*          ) nullptr,            //传递给任务函数的参数
                    (UBaseType_t    ) 1,                  //任务优先级
                    (TaskHandle_t*  ) NULL);              //任务句柄

        xTaskCreate((TaskFunction_t ) app_task_sys,      
                    (const char*    ) "sys",             
                    (uint16_t       ) 256,               
                    (void*          ) nullptr,           
                    (UBaseType_t    ) 1,                 
                    (TaskHandle_t*  ) NULL);             

        xTaskCreate((TaskFunction_t ) app_task_udcheck,   
                    (const char*    ) "udcheck",          
                    (uint16_t       ) 256,                
                    (void*          ) nullptr,            
                    (UBaseType_t    ) 1,                  
                    (TaskHandle_t*  ) NULL);
#else
        xTaskCreate((TaskFunction_t ) app_task_main,  //任务函数
                    (const char*    ) "main",         //任务名称
                    (uint16_t       ) 1024,           //任务堆栈大小(单位字)
                    (void*          ) nullptr,        //传递给任务函数的参数
                    (UBaseType_t    ) 1,              //任务优先级
                    (TaskHandle_t*  ) NULL);          //任务句柄
#endif
        // clang-format on
    }
    taskEXIT_CRITICAL();

    vTaskDelete(nullptr);
}


/**
 * @description: 处理主要的串口协议与设备
 * @param param
 * @return
 */
static void app_task_main(void *param)
{
    while (1)
    {
#ifdef NOT_CUSTOM_BUILD
        zst_task_handler();
        udc_pack_task(&udcPackGroup_main);
#endif
        app_loop();
        vTaskDelay(1);
    }
}

/**
 * @description: 处理主要的串口协议与设备
 * @param param
 * @return
 */
static void app_task_sys(void *param)
{
    while (1) 
    { 
        udc_pack_task(&udcPackGroup_sys);
        vTaskDelay(1); 
    }
}

/**
 * @description: 用于接收串口数据到udcheck
 * @param param
 */
static void app_task_udcheck(void *param)
{
    while (1)
    {
        while (if_uart_comm.available() > 0)
        {
            uint8_t rev_data = if_uart_comm.read();
            udc_pack_receive_data(&udcPackGroup_main, &rev_data, 1);
            udc_pack_receive_data(&udcPackGroup_sys, &rev_data, 1);
        }
        vTaskDelay(1);
    }
}

