#include <Arduino.h>
#include "zs_tool.h"

static zst_button_t button = {0};

static uint8_t button_read(void)
{
    if (digitalRead(GPIO_NUM_21) == 0)
    {
        return ZST_BTN_ACTIVE_LEVEL;
    }
    return !ZST_BTN_ACTIVE_LEVEL;
}

static void event_cb(zst_event_t *e)
{
    zst_button_t * btn = (zst_button_t *)zst_event_get_target(e);
    uint8_t button_event = zst_event_get_code(e);
    uint8_t count = (uint32_t)zst_event_get_param(e);
    Serial.printf("button event: %d, count: %d\n", button_event, count);
}

void setup(void)
{
    Serial.begin(115200);
    pinMode(GPIO_NUM_21, INPUT_PULLUP);

    zst_button_init_t button_i = {
        .btn = &button,
        .read_pin = button_read,
    };

    zst_button_init(&button_i);
    zst_target_add_event_cb(&button, event_cb, ZST_EVENT_ALL, NULL);
}

void loop(void)
{
    zst_button_irq_handler(&button);
    zst_button_process(&button);
}
