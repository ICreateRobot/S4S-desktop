#ifndef __ZST_CONF_H__
#define __ZST_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"
#include <stddef.h>

// #define ZST_DEBUG
#define ZST_RELEASE

#define ZST_USE_ALLOC    0
#define ZST_TICK_CUSTOM  0
#define ZST_USE_LOG      1
#define ZST_USE_TIMER    0
#define ZST_USE_EVENT    0
#define ZST_RUN_ONE      0
#define ZST_USE_PTASK    0
#define ZST_USE_REGISTER 0
#define ZST_USE_PID      0
#define ZST_SHA256       0
#define ZST_DEVICES      0
#define ZST_DRIVER       0


#if ZST_USE_ALLOC
    #define ZST_MEM_LOG    1
    #define ZST_MEM_CUSTOM 1
    #if (ZST_MEM_CUSTOM)
        #define ZST_MEM_CUSTOM_INCLUDE     "stdlib.h"
        #define ZST_MEM_CUSTOM_ALLOC(x)     malloc(x)
        #define ZST_MEM_CUSTOM_FREE(x)      free(x)
        #define ZST_MEM_CUSTOM_REALLOC(x,y) realloc(x,y)
    #else
        // For how many k bytes; Please ensure it is greater than 7
        #define ZST_MEM_SIZE (7)
        #if (ZST_MEM_LOG && ZST_USE_LOG)
            #define tlsf_assert(expr) do {if(!(expr)) ZST_LOGE("tlsf", "%s", #expr);}while(0)
        #else
            #define tlsf_assert(expr)
        #endif
    #endif
#endif

/*Use a custom tick source that tells the elapsed time in milliseconds.
 *It removes the need to manually update the tick with `zst_tick_inc()`).
*/
#if ZST_TICK_CUSTOM
    #define ZST_TICK_CUSTOM_INCLUDE     "Arduino.h"
    #define ZST_TICK_CUSTOM_SYS_TIME    (millis())
#endif


#if ZST_USE_LOG
    // 0 - 6
    #define ZST_LOG_LEVEL  4
#endif

#if ZST_DEVICES
    #define ZST_DEVICE_IRMOTE 0
    #define ZST_DEVICE_ULTR   0
    #define ZST_DEVICE_BUTTON 0
#endif

#if ZST_DRIVER
    #define ZST_SOFT_I2C     0
#endif

#if (ZST_USE_PID == 1)
    // only float or double
    #define ZST_PID_CALC_TYPE  float
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __ZST_CONF_H__ */
