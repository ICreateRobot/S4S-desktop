#include "zst_event.h"

#if (ZST_USE_EVENT == 1)

#include "stdlib.h"
#include "string.h"
#include "../misc/zst_mem.h"
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int event_send_core(zst_event_t *e);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
int zst_event_send_exe_now(void * target_obj, uint16_t event_code, void * param)
{
    if (NULL == target_obj) 
        return -1;

    zst_event_t e;
    e.target = target_obj;
    e.code = event_code;
    e.param = param;
    e.user_data = NULL;
    
    return event_send_core(&e);
}


zst_event_dsc_t *zst_target_add_event_cb_static(void * target_obj, zst_event_dsc_t *event_dsc, zst_event_cb_t event_cb, uint16_t filter,
                                              void *user_data)
{
    if (NULL == event_dsc)
    {
        return NULL;
    }
    memset(event_dsc, 0, sizeof(zst_event_dsc_t));
    event_dsc->cb = event_cb;
    event_dsc->filter = filter;
    event_dsc->user_data = user_data;
    event_dsc->is_alloc = 0;
    event_dsc->next = NULL;


    _zst_obj_spec_attr_t *obj = (_zst_obj_spec_attr_t *)target_obj;

    zst_event_dsc_t *target_event_dsc = obj->event_dsc;
    obj->event_dsc_cnt++;
    if (NULL == target_event_dsc)
    {
        obj->event_dsc = event_dsc;
    }
    else
    {
        while (target_event_dsc->next)
        {
            target_event_dsc = target_event_dsc->next;
        }
        target_event_dsc->next = event_dsc;
    }
    return event_dsc;
}

zst_event_dsc_t * zst_target_add_event_cb(void * target_obj, zst_event_cb_t event_cb, uint16_t filter, void * user_data)
{
    zst_event_dsc_t *event_dsc = zst_mem_alloc(sizeof(zst_event_dsc_t));
    if (NULL == event_dsc) return NULL;
    zst_target_add_event_cb_static(target_obj, event_dsc, event_cb, filter, user_data);
    event_dsc->is_alloc = 1;
    return event_dsc;
}

int zst_pack_remove_event_cb(void * target_obj, zst_event_cb_t event_cb)
{
    _zst_obj_spec_attr_t *obj_attr = (_zst_obj_spec_attr_t *)target_obj;
    if (0 == obj_attr->event_dsc_cnt) return -1;
    zst_event_dsc_t *pthis = obj_attr->event_dsc;
    zst_event_dsc_t *prv = pthis;
    int ret_flag = -1;

    while (pthis)
    {
        if (NULL == event_cb || pthis->cb == event_cb) 
        {
            ret_flag = 0;
            obj_attr->event_dsc_cnt--;
            prv->next = pthis->next;
            if (1 == pthis->is_alloc)
                zst_mem_free(pthis);
            if (0 == obj_attr->event_dsc_cnt)
            {
                obj_attr->event_dsc = NULL;
                return 0;
            }
            if (NULL == event_cb)
            {
                return 0;
            }
        }
        prv = pthis;
        pthis = pthis->next;
    }
    return ret_flag;
}



void *zst_event_get_target(zst_event_t *e)
{
    return e->target;
}

uint16_t zst_event_get_code(zst_event_t *e)
{
    return e->code;
}

void *zst_event_get_param(zst_event_t *e)
{
    return e->param;
}

void *zst_event_get_user_data(zst_event_t *e)
{
    return e->user_data;
}


/**********************
 *   STATIC FUNCTIONS
 **********************/

static zst_event_dsc_t *zst_target_get_event_dsc(const void *target_obj, uint32_t id)
{
    const _zst_obj_spec_attr_t *obj = (_zst_obj_spec_attr_t *)target_obj;
    if (NULL == obj->event_dsc)
        return NULL;
    if (id >= obj->event_dsc_cnt)
        return NULL;

    return &obj->event_dsc[id];
}

static int event_send_core(zst_event_t *e)
{
    zst_event_dsc_t *event_dsc = zst_target_get_event_dsc(e->target, 0);

    while (event_dsc)
    {
        if (event_dsc->filter == e->code || event_dsc->filter == ZST_EVENT_ALL)
        {
            e->user_data = event_dsc->user_data;
            event_dsc->cb(e);
        }
        event_dsc = event_dsc->next;
    }
    return 0;
}



#endif /* ZST_USE_EVENT */
