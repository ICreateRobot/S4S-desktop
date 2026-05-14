#ifndef __C_LOG_H__
#define __C_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"

#if (ZST_USE_LOG == 1)

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum
{
    E_CLOGU = 1, // user
    E_CLOGE,
    E_CLOGW,
    E_CLOGI,
    E_CLOGD,
    E_CLOGV,
} LOG_LEVEL_E;

typedef enum
{
    CLOG_NONE,
    CLOG_USER,
    CLOG_ERROR,
    CLOG_WARN,
    CLOG_INFO,
    CLOG_DEBUG,
    CLOG_VERBOSE
} log_level_t;

void clog_write(log_level_t level, const char *tag, const char *format, ...) __attribute__((format(printf, 3, 4)));
uint32_t clog_timestamp(void);

/* 颜色定义 */
#define CLOG_COLOR_BLACK   "30"
#define CLOG_COLOR_RED     "31"
#define CLOG_COLOR_GREEN   "32"
#define CLOG_COLOR_BROWN   "33"
#define CLOG_COLOR_BLUE    "34"
#define CLOG_COLOR_PURPLE  "35"
#define CLOG_COLOR_CYAN    "36"
#define CLOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define CLOG_RESET_COLOR   "\033[0m"

#define CLOG_COLOR_E CLOG_COLOR(CLOG_COLOR_RED)
#define CLOG_COLOR_W CLOG_COLOR(CLOG_COLOR_BROWN)
#define CLOG_COLOR_I CLOG_COLOR(CLOG_COLOR_GREEN)
#define CLOG_COLOR_D ""
#define CLOG_COLOR_V ""
#define CLOG_COLOR_U ""

/* --- 统一格式定义，增加文件行号占位符 --- */
// 格式: [颜色][级别] (时间戳) [标签] 文件:行号: 内容 [清除颜色]
#define CLOG_FORMAT(letter, format)                                                                \
    CLOG_COLOR_##letter #letter " (%d) [%s] %s:%d: " format CLOG_RESET_COLOR "\n"

/* ---  所有的输出最终汇聚到这个私有宏 --- */
#define _CLOG_WRITE_INTERNAL(level, letter, tag, format, ...)                                      \
    clog_write(level, tag, CLOG_FORMAT(letter, format), clog_timestamp(), tag, __FILE__, __LINE__, \
               ##__VA_ARGS__)

#define CLOGE(tag, format, ...) _CLOG_WRITE_INTERNAL(CLOG_ERROR, E, tag, format, ##__VA_ARGS__)
#define CLOGW(tag, format, ...) _CLOG_WRITE_INTERNAL(CLOG_WARN, W, tag, format, ##__VA_ARGS__)
#define CLOGI(tag, format, ...) _CLOG_WRITE_INTERNAL(CLOG_INFO, I, tag, format, ##__VA_ARGS__)
#define CLOGD(tag, format, ...) _CLOG_WRITE_INTERNAL(CLOG_DEBUG, D, tag, format, ##__VA_ARGS__)
#define CLOGV(tag, format, ...) _CLOG_WRITE_INTERNAL(CLOG_VERBOSE, V, tag, format, ##__VA_ARGS__)

// User 级别通常不需要位置信息，单独定义
#define CLOGU(tag, format, ...)                                                                    \
    clog_write(CLOG_USER, "", CLOG_COLOR_U "" format CLOG_RESET_COLOR "\n",                        \
               ##__VA_ARGS__)

#define DEF_CLOG_DEFAULT_LEVEL ZST_LOG_LEVEL


#define CLOG(level, tag, format, ...)                                                              \
    do                                                                                             \
    {                                                                                              \
        if (level <= DEF_CLOG_DEFAULT_LEVEL)                                                       \
        {                                                                                          \
            switch (level)                                                                         \
            {                                                                                      \
                case E_CLOGE: CLOGE(tag, format, ##__VA_ARGS__); break;                            \
                case E_CLOGW: CLOGW(tag, format, ##__VA_ARGS__); break;                            \
                case E_CLOGI: CLOGI(tag, format, ##__VA_ARGS__); break;                            \
                case E_CLOGD: CLOGD(tag, format, ##__VA_ARGS__); break;                            \
                case E_CLOGV: CLOGV(tag, format, ##__VA_ARGS__); break;                            \
                case E_CLOGU: CLOGU(tag, format, ##__VA_ARGS__); break;                            \
                default: CLOGI(tag, format, ##__VA_ARGS__); break;                                 \
            }                                                                                      \
        }                                                                                          \
    } while (0)

/* 简写宏 */
#define ZST_LOG(format, ...)       CLOG(E_CLOGU, "*", format, ##__VA_ARGS__)
#define ZST_LOGE(tag, format, ...) CLOG(E_CLOGE, tag, format, ##__VA_ARGS__)
#define ZST_LOGW(tag, format, ...) CLOG(E_CLOGW, tag, format, ##__VA_ARGS__)
#define ZST_LOGI(tag, format, ...) CLOG(E_CLOGI, tag, format, ##__VA_ARGS__)
#define ZST_LOGD(tag, format, ...) CLOG(E_CLOGD, tag, format, ##__VA_ARGS__)
#define ZST_LOGV(tag, format, ...) CLOG(E_CLOGV, tag, format, ##__VA_ARGS__)


#ifdef ZST_DEBUG
#define zst_assert(expr)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(expr)) { ZST_LOGE("ASSERT", "%s", #expr); }                                          \
    } while (0)
#else
#define zst_assert(expr) ((void)0)
#endif

#endif // ZST_USE_LOG

#ifdef __cplusplus
}
#endif

#endif // !__C_LOG_H__
