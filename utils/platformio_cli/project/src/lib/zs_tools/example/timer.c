#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "esp_task_wdt.h"

#include "zst_core.h"

void zst_timer(zst_timer_t *timer)
{
    ZST_LOGI("123", "timer expired");
}

void setup()
{
    Serial.begin(115200);
    zst_init();
    zst_timer_create(&zst_ztimer, zst_timer, 1000, NULL);
}

void loop()
{
    zst_task_handler();
}

