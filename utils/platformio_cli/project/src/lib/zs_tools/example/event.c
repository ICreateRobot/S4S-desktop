#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "esp_task_wdt.h"

#include "../lib/zs_tools/zs_tool.h"

enum test_enum_t
{
    _test_event_header = _ZST_EVENT_LAST, // 不能使用此值

    test_event_0,
    test_event_1,
};

struct 
{
    _zst_obj_spec_attr_t spec_attr;
    int a;
} test;


void zst_timer_cb(zst_timer_t *timer)
{
    static int i = 0;
    uint16_t evelt_list[] = {ZST_EVENTR_1, test_event_1};
    zst_event_send_exe_now((void *)&test, evelt_list[i], NULL); 
    i = (i + 1) % 2;
}

void zst_all_event_cb(zst_event_t *e)
{
    Serial.println("zst_all_event_cb");
}

void zst_public_event_cb(zst_event_t *e)
{
    Serial.println("zst_public_event_cb");
}

void zst_test_event_cb(zst_event_t *e)
{
    Serial.println("zst_test_event_cb");
}

void setup()
{
    Serial.begin(115200);
    zst_init();
    zst_timer_create(zst_timer_cb, 1000, NULL);

    
    zst_target_add_event_cb((void *)&test, zst_all_event_cb,    ZST_EVENT_ALL, NULL);
    zst_target_add_event_cb((void *)&test, zst_public_event_cb, ZST_EVENTR_1, NULL);
    zst_target_add_event_cb((void *)&test, zst_test_event_cb,   test_event_1, NULL);
}

void loop()
{
    zst_task_handler();
    delay(100);
}

