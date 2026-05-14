#ifndef __ZST_BUTTON_H__
#define __ZST_BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"

#if ZST_DEVICE_BUTTON == 1
#include "../../event/zst_event.h"

/** 配置参数 **/
#define ZST_BTN_ACTIVE_LEVEL 1    // 有效电平：1-高电平，0-低电平
#define BTN_RAW_FIFO_SIZE    8    // 建议设为 2 的幂次方
#define BTN_DEBOUNCE_MS      20   // 逻辑消抖时间
#define BTN_LONG_PRESS_MS    600  // 长按判定时间
#define BTN_CLICK_GAP_MS     250  // 连击间隔时间
#define BTN_IRQ_FILTER_MS    10   // 中断前置滤毛刺时间
#define BTN_LONG_HOLD_MS     150  // 长按期间每隔 150ms 触发一次

typedef enum
{
    _BTN_EVENT_ = ZST_EVENT_LAST,
    BTN_EVENT_PRESS,        // 按下瞬间
    BTN_EVENT_RELEASE,      // 普通释放瞬间（对应短按/连击）
    BTN_EVENT_LONG_START,   // 长按触发瞬间
    BTN_EVENT_LONG_HOLD,    // 长按期间持续触发
    BTN_EVENT_LONG_RELEASE, // 长按释放瞬间
    BTN_EVENT_CLICK,        // 单击确认
    BTN_EVENT_DOUBLE_CLICK, // 双击确认
    BTN_EVENT_MULTI_CLICK   // 多击
} BTN_EVENT_T;

typedef struct {
    uint8_t level;
    uint32_t tick;
} zst_btn_raw_event_t;

typedef struct {
    _zst_obj_spec_attr_t spec_attr;
    uint8_t (*read_pin)(void);
    void *user_data;

    // --- 状态机变量 ---
    uint8_t state;            // 0:IDLE, 1:PRESSING, 2:LONG_PRESS
    uint32_t last_tick;       // 逻辑计时
    uint16_t click_count;     // 连击计数器
    uint16_t long_hold_count; // 长按期间持续触发的次数

    // --- 消抖与同步 ---
    uint32_t irq_stable_tick; // 用于在 handler 中确认电平稳定的计时器
    uint32_t last_irq_tick;   // 中断前置消抖计时
    uint8_t last_irq_level;   // 上次中断记录的电平

    // --- FIFO 队列 ---
    zst_btn_raw_event_t raw_fifo[BTN_RAW_FIFO_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} zst_button_t;

typedef struct {
    zst_button_t *btn;
    uint8_t (*read_pin)(void);
} zst_button_init_t;

void zst_button_init(zst_button_init_t *init);
void zst_button_irq_handler(zst_button_t *btn);
void zst_button_process(zst_button_t *btn);

#endif

#ifdef __cplusplus
}
#endif

#endif
