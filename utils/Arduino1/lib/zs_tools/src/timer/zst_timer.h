#ifndef __ZST_TIMER_H__
#define __ZST_TIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "zst_conf.h"
#if (ZST_USE_TIMER == 1)

#include "../hal/zst_hal.h"

typedef struct _zst_timer_t zst_timer_t;
typedef void (*zst_timer_cb_t)(zst_timer_t *timer);

/**
 * 软件定时器对象结构体
 */
struct _zst_timer_t
{
    zst_timer_cb_t cb; /**< 定时器回调函数 */
    void *user_data;   /**< 用户数据 */
    zst_timer_t *next; /**< 链表指针 */

    uint32_t period;   /**< 周期时间（单位：ms） */
    uint32_t last_run; /**< 上次触发时间（tick） */

    int32_t repeat_count; /**< 重复次数，-1为无限次，0为停止，大于0为剩余次数 */
    uint8_t paused : 1;          /**< 是否暂停 */
    uint8_t ready : 1;           /**< 是否立即触发 */
    uint8_t to_be_deleted : 1;   /**< 是否需要删除 */
};

typedef struct 
{
    zst_timer_t *timer_list;
    uint32_t last_idle_tick;
    uint32_t idle_time_acc;
    bool timer_enabled;
} ztimer_t;


/**
 * 初始化软件定时器模块
 */
void zst_timer_core_init(ztimer_t *ztimer);

/**
 * 定期调用以处理所有定时器
 * @return 下一次需要调用 handler 的最小等待时间（单位：ms）
 */
uint32_t zst_timer_handler(ztimer_t *ztimer);

/**
 * 以固定周期运行定时器处理函数
 * @param ms 执行周期（单位：ms）
 * @return 剩余时间（如果未执行）或下一次调度间隔
 */
uint32_t zst_timer_handler_run_in_period(ztimer_t *ztimer, uint32_t ms);

/**
 * 创建一个空的定时器
 * @return 指向新定时器的指针
 * @note 需手动设置回调和周期
 */
zst_timer_t *zst_timer_create_basic(ztimer_t *ztimer);

/**
 * 创建一个新的软件定时器
 * @param cb 回调函数
 * @param period 周期时间（ms）
 * @param user_data 用户参数指针
 * @return 指向新定时器的指针
 */
zst_timer_t *zst_timer_create(ztimer_t *ztimer, zst_timer_cb_t cb, uint32_t period, void *user_data);

/**
 * 删除一个定时器
 * @param timer 要删除的定时器指针
 */
void zst_timer_del(ztimer_t *ztimer, zst_timer_t *timer);

/**
 * 暂停一个定时器
 * @param timer 要暂停的定时器指针
 */
void zst_timer_pause(zst_timer_t *timer);

/**
 * 恢复一个定时器
 * @param timer 要恢复的定时器指针
 */
void zst_timer_resume(zst_timer_t *timer);

/**
 * 设置定时器的回调函数
 * @param timer 定时器指针
 * @param cb 回调函数指针
 */
void zst_timer_set_cb(zst_timer_t *timer, zst_timer_cb_t cb);

/**
 * 设置定时器的周期
 * @param timer 定时器指针
 * @param period 周期时间（单位：ms）
 */
void zst_timer_set_period(zst_timer_t *timer, uint32_t period);

/**
 * 设置定时器的重复次数
 * @param timer 定时器指针
 * @param repeat_count -1：无限次；0：停止；n > 0：剩余次数
 */
void zst_timer_set_repeat_count(zst_timer_t *timer, int32_t repeat_count);

/**
 * 将定时器设置为“就绪”，即立即触发
 * @param timer 定时器指针
 */
void zst_timer_ready(zst_timer_t *timer);

/**
 * 获取定时器的用户数据
 * @param timer 定时器指针
 * @return 用户数据指针
 */
void * zst_timer_get_user_data(zst_timer_t *timer);

/**
 * 重置定时器，使其下次在 period 毫秒后再次触发
 * @param timer 定时器指针
 */
void zst_timer_reset(zst_timer_t *timer);

/**
 * 启用或禁用所有定时器的处理
 * @param en true：启用；false：禁用
 */
void zst_timer_enable(ztimer_t *ztimer, bool en);

/**
 * 获取定时器模块的“空闲率”（用于估算 CPU 占用）
 * @return 空闲百分比（0~100）
 */
uint8_t zst_timer_get_idle(ztimer_t *ztimer);

/**
 * 遍历所有定时器
 * @param timer NULL 表示开始遍历，或传入上一个定时器
 * @return 下一个定时器指针，若无更多定时器则返回 NULL
 */
zst_timer_t *zst_timer_get_next(ztimer_t *ztimer, zst_timer_t *timer);
#endif // ZST_TIMER_H

#ifdef __cplusplus
}
#endif

#endif /* __ZST_TIMER_H__ */
