#ifndef __ZST_IRMOTE_H__
#define __ZST_IRMOTE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"
#if ZST_DEVICE_IRMOTE==1


/**
 * 需要一个微秒级的计数器，可以获取其中断次数以及计数值
 * 以及一个可以调整中断的触发IO口，这个IO口需要能够在上升沿和下降沿之间切换。
 * 如果将gpio设置为上升下降触发，可以不提供 `set_falling` 与 `set_rising` 回调函数
 * 假设配置为 gpio 上升下降触发：
 * 典型配置为：
 * .id = 0,
 * .count_us = 1,
 * .overflow_count = 10000,
 * .get_gpio_pin_level = get_gpio_level,
 * .get_countVal = get_count,
 */
typedef struct
{
    uint32_t min;
    uint32_t max;
} IRemote_standardTime_t;

typedef struct
{
    uint32_t data; // 收到的数据
    uint8_t count; // 按键按下的次数
    struct
    {
        uint8_t get_guidanceCode : 1; // 收到了引导码标志
        uint8_t get_keyAllInfo   : 1; // 收到了一个按键的所有信息
        uint8_t                  : 1; // 保留
        uint8_t get_risingEdge   : 1; // 记上升沿是否已经被捕获
        uint8_t overflowCounter  : 4; // 溢出计时器
    } state;
} IRemote_t;

typedef struct _iremote
{
    uint8_t id;// 遥控器ID
    uint16_t overflow_count;// 溢出计时器
    void (*set_falling)(struct _iremote *);// 下降沿回调函数
    void (*set_rising)(struct _iremote *);// 上升沿回调函数
    uint8_t (*get_gpio_pin_level)(struct _iremote *);// 获取GPIO电平
    uint32_t (*get_countVal)(struct _iremote *);// 获取计数器时间
    uint32_t last_val;
    IRemote_t g_remote; // 遥控器数据
    IRemote_standardTime_t ir_560us;
    IRemote_standardTime_t ir_1680us;
    IRemote_standardTime_t ir_2250us;
    IRemote_standardTime_t ir_4500us;
    uint8_t init_finsh : 1;
} iremote_t;

void iremote_init(iremote_t * iremote, float tick_us, uint16_t overflow_count);
void iremote_timOverFlow_callback(iremote_t * iremote);
void iremote_gpio_interrupt_callback(iremote_t * iremote);
uint8_t remote_scan(iremote_t * iremote);
uint8_t remote_get_key_count(iremote_t * iremote);

#endif

#ifdef __cplusplus
}
#endif


#endif /* __IRMOTE_H__ */

