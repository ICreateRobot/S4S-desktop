#include "zst_button.h"

#if ZST_DEVICE_BUTTON == 1
#include "../../hal/zst_hal_tick.h"
#include "../../log/c_log.h"
#include "../../misc/zst_mem.h"

#define LOG_TAG "zst_button"

void zst_button_init(zst_button_init_t *init)
{
    if (!init || !init->btn || !init->read_pin) return;

    zst_memset_00(init->btn, sizeof(zst_button_t));
    init->btn->read_pin = init->read_pin;

    // 初始化物理电平状态，防止开机误触发
    init->btn->last_irq_level = init->read_pin();
}

/**
 * @brief 硬件中断接口 (Any Edge) 或 高频定时器轮询
 */
void zst_button_irq_handler(zst_button_t *btn)
{
    zst_assert(NULL != btn);

    uint8_t current_level = btn->read_pin();
    uint32_t now          = zst_tick_get();

    // 1. 发现物理电平与上一次“确认”的电平不同
    if (current_level != btn->last_irq_level)
    {
        // 2. 如果这是电平跳变的开始，记录当前时间
        if (btn->irq_stable_tick == 0)
        {
            btn->irq_stable_tick = now;
            return;
        }

        // 3. 只有当电平持续稳定超过 BTN_IRQ_FILTER_MS (建议 15-20ms)
        if (zst_tick_elaps(btn->irq_stable_tick) >= BTN_IRQ_FILTER_MS)
        {
            btn->last_irq_level  = current_level; // 更新确认电平
            btn->irq_stable_tick = 0;             // 重置计时器

            uint8_t next = (btn->head + 1) % BTN_RAW_FIFO_SIZE;
            if (next != btn->tail)
            {
                btn->raw_fifo[btn->head].level = current_level;
                btn->raw_fifo[btn->head].tick  = now;
                btn->head                      = next;
            }
        }
    } else
    {
        // 如果中途电平变回去了（说明是毛刺），重置计时器
        btn->irq_stable_tick = 0;
    }
}

void zst_button_process(zst_button_t *btn)
{
    // A. 处理 FIFO 中的物理事件
    if (btn->tail != btn->head)
    {
        zst_btn_raw_event_t raw = btn->raw_fifo[btn->tail];
        btn->tail               = (btn->tail + 1) % BTN_RAW_FIFO_SIZE;

        uint8_t is_pressed = (raw.level == ZST_BTN_ACTIVE_LEVEL);
        uint32_t delta     = raw.tick - btn->last_tick;

        switch (btn->state)
        {
            case 0: // IDLE
                if (is_pressed)
                {
                    btn->state     = 1;
                    btn->last_tick = raw.tick;
                    zst_event_send_exe_now(btn, BTN_EVENT_PRESS,
                                           (void *)(uintptr_t)btn->click_count);
                }
                break;

            case 1: // PRESSING
                if (!is_pressed)
                {
                    if (delta >= BTN_DEBOUNCE_MS)
                    {
                        // 有效的点击，记录次数
                        btn->click_count++;
                        zst_event_send_exe_now(btn, BTN_EVENT_RELEASE,
                                               (void *)(uintptr_t)btn->click_count);
                        btn->last_tick = raw.tick;
                        btn->state     = 0;
                    } else
                    {
                        // 【修正点】：判定为抖动
                        // 不要直接回到 0，或者回到 0 但不重置某些关键状态
                        // 建议：如果判定为抖动，说明电平不稳定，保持在 state 1 或者回 0 但通过
                        // level 检查
                        btn->state = 0;
                        // 可以在这里打印一个 debug 来看是不是消抖触发了
                    }
                }
                break;

            case 2:              // LONG_PRESSING (长按中)
                if (!is_pressed) // 此时松开
                {
                    btn->state     = 0;
                    btn->last_tick = raw.tick;
                    // 发送专门的长按释放事件，param 传入 0 即可
                    zst_event_send_exe_now(btn, BTN_EVENT_LONG_RELEASE, (void *)(uintptr_t)btn->long_hold_count);
                    btn->long_hold_count = 0; // 释放后清零
                }
                break;
        }
    }

    // B. 处理超时逻辑（长按判定、多击结算、持续触发）
    uint32_t now_delta            = zst_tick_elaps(btn->last_tick);
    uint8_t is_physically_pressed = (btn->read_pin() == ZST_BTN_ACTIVE_LEVEL);

    // 1. 判定连击确认 (处于 IDLE 且没人按，等待超时)
    if (btn->state == 0 && btn->click_count > 0 && now_delta > BTN_CLICK_GAP_MS)
    {
        BTN_EVENT_T evt = (btn->click_count == 1) ? BTN_EVENT_CLICK :
                          (btn->click_count == 2) ? BTN_EVENT_DOUBLE_CLICK :
                                                    BTN_EVENT_MULTI_CLICK;
        zst_event_send_exe_now(btn, evt, (void *)(uintptr_t)btn->click_count);
        btn->click_count = 0;
    }

    // 2. 判定长按开始 (处于 PRESSING 且电平持续)
    if (btn->state == 1 && is_physically_pressed && now_delta > BTN_LONG_PRESS_MS)
    {
        btn->state           = 2;
        btn->click_count     = 0;
        btn->long_hold_count = 0;              // 长按刚开始，计数设为 0
        btn->last_tick       = zst_tick_get(); // 重置时间，作为 Hold 触发的起点
        zst_event_send_exe_now(btn, BTN_EVENT_LONG_START, NULL);
    }

    // 3. 【新增】判定长按持续触发 (处于 LONG_PRESSING 且电平持续)
    if (btn->state == 2 && is_physically_pressed && now_delta > BTN_LONG_HOLD_MS)
    {
        btn->long_hold_count++;          // 累加触发次数
        btn->last_tick = zst_tick_get(); // 再次重置时间，准备下一次 Hold 周期
        zst_event_send_exe_now(btn, BTN_EVENT_LONG_HOLD, (void *)(uintptr_t)btn->long_hold_count);
    }
}

uint8_t zst_button_get_click_count(zst_button_t *btn)
{
    return btn->click_count;
}

#endif
