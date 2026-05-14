#ifndef __ZST_HAL_TICK_H__
#define __ZST_HAL_TICK_H__

#ifdef __cplusplus
extern "C" {
#endif    

#include "zst_conf.h"



#if ZST_TICK_CUSTOM==0
/**
 * You have to call this function periodically
 * must be in 1 millisecond cycles
 */
void zst_tick_inc(uint32_t ms);
#endif

/**
 * Get the elapsed milliseconds since start up
 * @return the elapsed milliseconds
 */
uint32_t zst_tick_get(void);

/**
 * Get the elapsed milliseconds since a previous time stamp
 * @param prev_tick a previous time stamp (return value of udc_tick_get() )
 * @return the elapsed milliseconds since 'prev_tick'
 */
uint32_t zst_tick_elaps(uint32_t prev_tick);

#ifdef __cplusplus
}
#endif

#endif // __ZST_HAL_TICK_H__
