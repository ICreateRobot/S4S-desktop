/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 14:48:31
 * @LastEditTime : 2026-05-08 14:48:05
 * @Description  : 串口通讯协议
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "dev_protocol.h"
#include "sys/dev_sys.h"
#include "common/dev_protocol_common.h"
#include "interface/uart/if_uart.h"
#include "main_board/dev_protocol_mainboard.h"
#include "ai_camera/dev_protocol_aiCamera.h"
#include "gray/dev_protocol_gray.h"
#include "ultrasound/dev_protocol_ultr.h"
#include "audio/dev_protocol_espAudio.h"
#include "oled/dev_protocol_espOLED.h"
#include "pin/dev_protocol_pin.h"
#include "gyro/dev_protocol_gyro.h"
#include "magnetometer/dev_protocol_magnetometer.h"

/******************
 * data struct
 *****************/
#define LOG_TAG "dev_protocol"


/****************************
 * function declaration
 ***************************/
static int  udc_send_bytes_callback(const struct _udc_pack_t *pack, const uint8_t *buf,
                                    uint16_t len);
static int  calculate_verify(const struct _udc_pack_t *pack, const uint8_t *buf, uint16_t len,
                             uint8_t *verify);
static void udc_event_receive_finsh(udc_event_t *e);
static void serialize_field(const fmap_field_t &field);
static void function_map_send_result(const fmap_result_t &result);

/********************
 * static variables
 *******************/
static uint8_t         udc_protocol_rx_buffer[256] = {0};
static udc_event_dsc_t udc_event_receive_finsh_dsc = {0};


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void dev_protocol_init(void)
{
    /****************
     * 初始化
     ***************/
    dev_sys_init();
    dev_protocol_common_init();
    dev_protocol_pin_init();
    dev_protocol_espAudio_init();
    dev_protocol_espOLED_init();
    dev_protocol_mainBoard_init();
    dev_protocol_aiCamera_init();
    dev_protocol_gray_init();
    dev_protocol_ultr_init();
    dev_protocol_gyro_init();
    dev_protocol_magnetometer_init();


    /****************
     * 协议初始化
     ***************/
    udc_pack_init_t init = {.pack_group = &udcPackGroup_main,
                            .pack       = &udcPack_main,
                            .header =
                                {
                                    .header     = "\xAA\x01",
                                    .header_len = 2,
                                },
                            .verify = {
                                .calculate_verify = calculate_verify,
                                .verify_len       = 1,
                            }};
    udc_pack_init(&init);
    udc_pack_set_send_bytes_func(&udcPack_main, udc_send_bytes_callback);
    udc_pack_set_buffer_static(&udcPack_main, UDC_PACK_RECEIVE, udc_protocol_rx_buffer,
                               sizeof(udc_protocol_rx_buffer));


    /****************
     * 协议事件注册
     ***************/
    udc_pack_add_event_cb_static(&udcPack_main, &udc_event_receive_finsh_dsc,
                                 udc_event_receive_finsh, UDC_EVENT_PACK_RECEIVE_FINSHED, NULL);
}

void dev_protocol_function_map_execute(function_map_collection_t &function_map_collection,
                                       udc_pack_t                *pack)
{
    // 调试输出 obj
    // if (ZST_LOG_LEVEL >= 4)
    // {
    //     udc_obj_t obj = {0};
    //     // clang-format off
    //     UDC_PACK_OBJ_FOREACH(UDC_PACK_RECEIVE, pack, &obj, 
    //         printf("obj id: %d\n", obj.id);
    //         printf("obj size: %d\n", obj.size);
    //         printf("obj data: ");
    //         for (int i = 0; i < obj.size; i++)
    //         {
    //             printf("%d ", obj.data[i]);
    //         }
    //         printf("\n-----------\n");
    //     );
    //     // clang-format on
    // }

    std::string function_map = "mainBoard";
    int         ret = function_map_udcpack_get_param_string_default(pack, 10, &function_map);
    if (-1 == ret)
    {
        ZST_LOGW(LOG_TAG, "no target device, use default: %s", function_map.c_str());
    } else if (-2 == ret)
    {
        ZST_LOGE(LOG_TAG, "type error");
        if_uart_comm.send_bytes(">>>");
        return;
    }

    std::string function_name;
    if (0 == function_map_udcpack_get_param_string_default(pack, 11, &function_name))
    {
        uint8_t is_find_map = 0;
        for (auto &it : function_map_collection)
        {
            if (0 == strcmp(it.first, function_map.c_str()))
            {
                is_find_map = 1;
                fmap_result_t result = function_map_exec(it.second, function_name, pack);
                function_map_send_result(result);
                if (false == result.is_ok())
                {
                    ZST_LOGE(LOG_TAG, "execute function failed: %s, code: %d",
                             function_name.c_str(), result.err_code);
                }
            }
        }
        if (0 == is_find_map)
        {
            ZST_LOGW(LOG_TAG, "no target device: %s", function_map.c_str());
        }
    } else
    {
        ZST_LOGE(LOG_TAG, "type error");
    }
    if_uart_comm.send_bytes(">>>");
}


/****************************
 * static function
 ***************************/


static int udc_send_bytes_callback(const struct _udc_pack_t *pack, const uint8_t *buf, uint16_t len)
{
    if_uart_comm.send_bytes((uint8_t *)buf, len);
    return 0;
}

static int calculate_verify(const struct _udc_pack_t *pack, const uint8_t *buf, uint16_t len,
                            uint8_t *verify)
{
    *verify = 0x55;
    return 0;
}

/***************************************************
 *                  协议
 **************************************************/

static void udc_event_receive_finsh(udc_event_t *e)
{
    dev_protocol_function_map_execute(function_map_collection, &udcPack_main);
}


static void serialize_field(const fmap_field_t &field)
{
    switch (field.parse_value.type)
    {
        case FMapValueType::INT: {
            if_uart_comm.send_bytes(std::to_string(field.parse_value.i_val));
            break;
        }
        case FMapValueType::FLOAT: {
            if_uart_comm.send_bytes(std::to_string(field.parse_value.f_val));
            break;
        }
        case FMapValueType::STRING: {
            if_uart_comm.send_bytes(field.parse_value.s_val);
            break;
        }
        case FMapValueType::BYTES: {
            if_uart_comm.send_bytes("[");
            for (auto &b : field.parse_value.b_val)
            {
                if_uart_comm.send_bytes(std::to_string(b));
                if_uart_comm.send_bytes(",");
            }
            if_uart_comm.send_bytes("]");
            break;
        }
    }
}


// ----------------------------------------------------------------
//  统一输出管理：所有字段逐个输出
// ----------------------------------------------------------------
static void function_map_send_result(const fmap_result_t &result)
{
    uint16_t obj_count = 0;
    for (const auto &field : result.fields)
    {
        if (obj_count++ > 0) if_uart_comm.send_bytes(",");
        serialize_field(field);
    }
}
