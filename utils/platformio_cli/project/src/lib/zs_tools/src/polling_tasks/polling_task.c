#include "polling_task.h"

#if (ZST_USE_PTASK == 1)
#include <string.h>
#include "../misc/zst_mem.h"

static void event_callback(ptask_t *task, ptask_event_t *e);

int ptask_init(ptask_list_t * ptask_list)
{
    zst_memset_00(ptask_list, sizeof(ptask_list_t));
    if (0 != cc_list_new(&ptask_list->list))
    {
        return -1;
    }
    return 0;
}

// 创建单任务root
ptask_root_t * ptask_root_create(ptask_list_t * ptask_list)
{
    ptask_root_t * root = NULL;
    if (NULL == ptask_list || NULL == ptask_list->list)
    {
        return NULL;
    }
    root = zst_mem_calloc(1, sizeof(ptask_root_t));
    if (NULL == root) return NULL;
    if (0 != cc_list_new(&root->list))
    {
        zst_mem_free(root);
        return NULL;
    }
    if (0 != cc_list_insert_head(ptask_list->list, root))
    {
        cc_list_delete(root->list);
        zst_mem_free(root);
        return NULL;
    }
    return root;
}

ptask_t * ptask_create(ptask_root_t * root, ptask_event_callback_t  event_callback, void * user_data)
{
    ptask_t * task = NULL;
    if (NULL == root || NULL == root->list)
    {
        return NULL;
    }
    task = zst_mem_calloc(1, sizeof(ptask_t));
    if (NULL == task)
    {
        return NULL;
    }
    if (0 != cc_list_insert_head(root->list, task))
    {
        zst_mem_free(task);
        return NULL;
    }
    task->event_callback = event_callback;
    task->user_data = user_data;
    return task;
}

int ptask_root_select(ptask_list_t * ptask_list, ptask_root_t * root)
{
    if (NULL == ptask_list) return -1;
    if (NULL != ptask_list->current_root)
    {
        ptask_root_send_event(ptask_list->current_root, PTASK_EVENT_EXIT, NULL);
        ptask_list->has_root_change = 1;
    }
    ptask_list->current_root = root;
    // TODO : INIT 事件由 `run` 内部执行
    return 0;
}

int ptask_root_pause(ptask_root_t * root)
{
    return ptask_root_send_event(root, PTASK_EVENT_SYS, (void *)1);
}

int ptask_root_resume(ptask_root_t * root)
{
    return ptask_root_send_event(root, PTASK_EVENT_SYS, (void *) 2);
}

int ptask_list_send_event(ptask_list_t * ptask_list, ptask_event_code_t event_code, void * param)
{
    if (NULL == ptask_list || NULL == ptask_list->list) return -1;
    cc_list_iter_t list_iter;
	void *list_tmp;
	size_t list_index;
    cc_list_iter_init(&list_iter, ptask_list->list, 0);
    while (!cc_list_iter_next(&list_iter, &list_tmp, &list_index))
    {
        ptask_root_t * root = *(ptask_root_t **) list_tmp;
        if (NULL == list_tmp || NULL == root) continue;
        if (NULL == root->list) continue;
        cc_list_iter_t iter;
        void *tmp;
        size_t index;
        cc_list_iter_init(&iter, root->list, 0);
        while (!cc_list_iter_next(&iter, &tmp, &index))
        {
            ptask_t * task = *(ptask_t **) tmp;
            if (NULL == task || NULL == task->event_callback) continue;
            ptask_event_t event = {
                .code = event_code,
                .param = param,
            };
            task->event_callback(task, &event);
            event_callback(task, &event); // sys event
        }
    }
    return 0;
}

int ptask_root_send_event(ptask_root_t * ptask_root, ptask_event_code_t event_code, void * param)
{
    cc_list_t * list = ptask_root->list;
    if (NULL == list) return -1;
    cc_list_iter_t iter;
    void *tmp;
    size_t index;
    cc_list_iter_init(&iter, list, 0);
    while (!cc_list_iter_next(&iter, &tmp, &index))
    {
        ptask_t * task = *(ptask_t **) tmp;
        if (NULL == task || NULL == task->event_callback) continue;
        ptask_event_t event = {
            .code = event_code,
            .param = param,
        };
        task->event_callback(task, &event);
        event_callback(task, &event); // sys event
    }
    return 0;
}

int ptask_send_event(ptask_t * task, ptask_event_code_t event_code, void * param)
{
    if (NULL == task || NULL == task->event_callback) return -1;
    ptask_event_t event = {
            .code = event_code,
            .param = param,
        };
    task->event_callback(task, &event);
    event_callback(task, &event); // sys event
    return 0;
}


int ptask_run(ptask_list_t * ptask_list)
{
    ptask_root_t * root = ptask_list->current_root;
    if (NULL == root || NULL == root->list) return -1;
    cc_list_iter_t iter;
    void *tmp;
    size_t index;
    cc_list_iter_init(&iter, root->list, 0);
    while (!cc_list_iter_next(&iter, &tmp, &index))
    {
        ptask_t * task = *(ptask_t **) tmp;
        if (NULL == task) continue;
        if (0 == task->is_init)
        {
            task->is_init = 1;
            ptask_send_event(task, PTASK_EVENT_INIT, NULL);
            continue;
        }
        if (1 == task->paused) continue;
        ptask_send_event(task, PTASK_EVENT_RUN, NULL);
        if(1 == ptask_list->has_root_change)
        {
            ptask_list->has_root_change = 0;
            return -2;
        }
    }

    // TODO: 删除task

    return 0;
}

int ptask_pause(ptask_t * task)
{
    task->paused = 1;
    return 0;
}

int ptask_resume(ptask_t * task)
{
    task->paused = 0;
    return 0;
}


ptask_event_code_t ptask_get_code(ptask_event_t * e)
{
    return e->code;
}

void * ptask_get_param(ptask_event_t * e)
{
    return e->param;
}

void * ptask_get_user_data(ptask_t * task)
{
    return task->user_data;
}

static void event_callback(ptask_t *task, ptask_event_t *e)
{
    switch (ptask_get_code(e))
    {
        case PTASK_EVENT_SYS : {
            uintptr_t param = (uintptr_t)ptask_get_param(e);
            if (1 == param)
            {
                task->paused = 1;
            }
            else if (2 == param)
            {
                task->paused = 0;
            }
        }break;
        
        case PTASK_EVENT_EXIT : 
            task->is_init = 0;
            break;
        
        default :
            break;
    }
}


#endif /* ZST_USE_PTASK */
