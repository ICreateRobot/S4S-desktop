#ifndef __POLLING_TASK_H__
#define __POLLING_TASK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "zst_conf.h"

#if (ZST_USE_PTASK == 1)
#include "../component/cc/cc_inc.h"
/**
 * | tasks    
 * --------------------------
 * [ TASK ] --- TASK --- TASK
 *    |
 *    |
 *    task
 *    |     
 *    |       
 *    task 
 *          
 *    
 */


typedef struct _ptask_ ptask_t;
typedef struct _ptask_event_ ptask_event_t;
typedef void (* ptask_event_callback_t)(ptask_t *task, ptask_event_t *e);

typedef enum 
{
    PTASK_EVENT_SYS = 0,
    PTASK_EVENT_INIT,
    PTASK_EVENT_RUN,
    PTASK_EVENT_EXIT,
    PTASK_EVENT_DEL,
    PTASK_EVENT_USER,
} ptask_event_code_t;


struct _ptask_event_
{
    ptask_event_code_t code;
    void *param;
};


struct _ptask_
{
    ptask_event_callback_t  event_callback;
    void * user_data;

    uint8_t is_init : 1;       /** 是否初始化完成 */
    uint8_t paused : 1;        /**< 是否暂停 */
    uint8_t to_be_deleted : 1; /**< 是否需要删除 */
};


typedef struct 
{
    cc_list_t * list;          // list -> ptask
    uint8_t paused : 1;        /**< 是否暂停 */
    uint8_t to_be_deleted : 1; /**< 是否需要删除 */
} ptask_root_t;


typedef struct 
{
    cc_list_t * list;            // list -> root
    ptask_root_t * current_root; // current root

    uint8_t has_root_change : 1; // 发生root切换事件
} ptask_list_t;



// 初始化 ptask 运行空间
int ptask_init(ptask_list_t * ptask_list);
int ptask_run(ptask_list_t * ptask_list);

// 创建任务root
ptask_root_t * ptask_root_create(ptask_list_t * ptask_list);
ptask_t * ptask_create(ptask_root_t * root, ptask_event_callback_t event_callback, void * user_data);

// 会退出上一个任务root，并切换到当前任务root
int ptask_root_select(ptask_list_t * ptask_list, ptask_root_t * root);

int ptask_root_pause(ptask_root_t * root);
int ptask_root_resume(ptask_root_t * root);
int ptask_root_del(ptask_root_t * root);

int ptask_pause(ptask_t * task);
int ptask_resume(ptask_t * task);
int ptask_del(ptask_root_t * root, ptask_t * task);

int ptask_list_send_event(ptask_list_t * ptask_list, ptask_event_code_t event_code, void * param);
int ptask_root_send_event(ptask_root_t * ptask_root, ptask_event_code_t event_code, void * param);
int ptask_send_event(ptask_t * task, ptask_event_code_t event_code, void * param);

ptask_event_code_t ptask_get_code(ptask_event_t * e);
void * ptask_get_param(ptask_event_t * e);
void * ptask_get_user_data(ptask_t * task);


#endif /* ZST_USE_PTASK */

#ifdef __cplusplus
}
#endif
#endif /* __POLLING_TASK_H__ */
