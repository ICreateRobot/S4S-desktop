#ifndef __ZS_EVENT_H__
#define __ZS_EVENT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "zst_conf.h"
#if (ZST_USE_EVENT == 1)
/*
*  #include "zst_event.h"
*  struct targat_obj
*  {
*      _zst_obj_spec_attr_t spec_attr; // must be the first element
*  };
*
*  creat file ../../lib/zst_event_conf.h
*  write your event code like this:
*  ZS_EVENT_0,
*  ZS_EVENT_1,
*  ZS_EVENT_2,
*  ...
*/


/**
 * Type of event being sent to the object.
 */
typedef enum
{
    ZST_EVENT_ALL = 0,
    ZST_EVENT_LAST,
} zst_event_code_t;

typedef struct _zst_event_t
{
    void *target;
    uint16_t code;
    void *user_data;
    void *param;
} zst_event_t;

/**
 * @brief Event callback.
 * Events are used to notify the user of some action being taken on the object.
 * For details, see ::zst_event_t.
 */
typedef void (*zst_event_cb_t)(zst_event_t *e);

typedef struct _zst_event_dsc_t
{
    zst_event_cb_t cb;
    void *user_data;
    struct _zst_event_dsc_t *next;
    uint16_t filter;
    uint8_t is_alloc : 1; // 0 no alloc, 1 alloc
} zst_event_dsc_t;

typedef struct
{
    zst_event_dsc_t *event_dsc; /** Dynamically allocated event callback and user data array*/

    uint8_t event_dsc_cnt;
} _zst_obj_spec_attr_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

int zst_event_send_exe_now(void *target_obj, uint16_t event_code, void *param);

/**
 * Get the object originally targeted by the event. It's the same even if the event is bubbled.
 * @param e     pointer to the event descriptor
 * @return      the target of the event_code
 */
void *zst_event_get_target(zst_event_t *e);

/**
 * Get the event code of an event
 * @param e     pointer to the event descriptor
 * @return      the event code.
 */
uint16_t zst_event_get_code(zst_event_t *e);

/**
 * Get the parameter passed when the event was sent
 * @param e     pointer to the event descriptor
 * @return      pointer to the parameter
 */
void *zst_event_get_param(zst_event_t *e);

/**
 * Get the user_data passed when the event was registered on the object
 * @param e     pointer to the event descriptor
 * @return      pointer to the user_data
 */
void *zst_event_get_user_data(zst_event_t *e);

/**
 * Add an event handler function for an object.
 * Used by the user to react on event which happens with the object.
 * An object can have multiple event handler. They will be called in the same order as they were added.
 * @param event_dsc  pointer to an event descriptor(staticly allocated)
 * @param target_obj pointer to an target_obj
 * @param filter     and event code
 * @param event_cb   the new event function
 * @param user_data custom data data will be available in `event_cb`
 * @return           a pointer the event descriptor.
 */
zst_event_dsc_t *zst_target_add_event_cb_static(void *target_obj, zst_event_dsc_t *event_dsc, zst_event_cb_t event_cb,
                                                uint16_t filter, void *user_data);

/**
 * Add an event handler function for an object.
 * Used by the user to react on event which happens with the object.
 * An object can have multiple event handler. They will be called in the same order as they were added.
 * @param target_obj      pointer to an target_obj
 * @param filter          event code
 * @param event_cb        the new event function
 * @param user_data       custom data data will be available in `event_cb`
 * @return                a pointer the event descriptor. 
 */
zst_event_dsc_t * zst_target_add_event_cb(void * target_obj, zst_event_cb_t event_cb, uint16_t filter, void * user_data);

/**
 * Remove an event handler function for an object.
 * @param target_obj pointer to an target_obj
 * @param event_cb   the event function to remove, or `NULL` to remove the firstly added event callback
 * @return           true if any event handlers were removed
 */
int zst_pack_remove_event_cb(void * target_obj, zst_event_cb_t event_cb);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ZS_EVENT_H__ */
