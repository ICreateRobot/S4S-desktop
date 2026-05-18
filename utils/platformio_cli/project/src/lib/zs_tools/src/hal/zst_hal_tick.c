#include "zst_hal_tick.h"


#if ZST_TICK_CUSTOM==0
    static volatile uint32_t sys_time = 0;
#else
    #include ZST_TICK_CUSTOM_INCLUDE
#endif


#if ZST_TICK_CUSTOM==0
/**
 * To ensure the accuracy of processing, the call of udc_tick_inc() must be in 1 millisecond cycles
 */
void zst_tick_inc(uint32_t ms)
{
    sys_time += ms;
}
#endif


uint32_t zst_tick_get(void)
{
    #if ZST_TICK_CUSTOM==0
        return sys_time;
    #else
        return ZST_TICK_CUSTOM_SYS_TIME;
    #endif
}

/**
 * Get the elapsed milliseconds since a previous time stamp
 * @param prev_tick a previous time stamp (return value of udc_tick_get() )
 * @return the elapsed milliseconds since 'prev_tick'
 */
uint32_t zst_tick_elaps(uint32_t prev_tick)
{
    uint32_t now_tick = zst_tick_get();
    if (now_tick >= prev_tick)
    {
        return now_tick - prev_tick;
    }
    else
    {
        return UINT32_MAX - prev_tick + 1 + now_tick;
    }
}
