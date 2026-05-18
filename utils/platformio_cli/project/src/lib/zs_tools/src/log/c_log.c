#include "c_log.h"
#include "../hal/zst_hal.h"

#if (ZST_USE_LOG == 1)



void clog_write(log_level_t level, const char *tag, const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
}

uint32_t clog_timestamp()
{
    return zst_tick_get();
}


#endif /* ZST_USE_LOG */
