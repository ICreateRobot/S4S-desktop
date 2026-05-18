#include <stdio.h>
#include "zs_tool.h"

void zst_event_cb(zst_event_t *e);

// 全局注册数据
reg_data_t reg_data = {
    .data_pack_type = DATA_PACK_TYPE_DOUBLE_ADDR,
};

typedef struct 
{
    uint8_t data0;
    uint8_t data1;
    uint8_t data2;
} device_t;

static device_t device = {0};
static device_t camp_device_buffer = {0};
static reg_data_element_t data_element[3] = {
    [0] = {
        .data = &device.data0,
        .data_size = sizeof(device.data0),
        .subscribe = 1,
    },
    [1] = {
        .data = &device.data1,
        .data_size = sizeof(device.data1),
    },
    [2] = {
        .data = &device.data2,
        .data_size = sizeof(device.data2),
    },
};
reg_data_pack_t data_pack = {0};

uint8_t camp_buffer[sizeof(data_element)] = {0};

int main()
{
    reg_data_init(&reg_data, 40);

    reg_data_pack_init_t data_pack_init = {
        .reg_data_pack = &data_pack,
        .element_array = data_element,
        .element_array_size = 3,
        .addr = 10,
        .comparison_buffer = (uint8_t *)&camp_device_buffer,
    };
    reg_data_pack_init(&reg_data, &data_pack_init);

    device.data0 = 1;

    zst_target_add_event_cb(&data_pack, zst_event_cb, 0, NULL);

    reg_data_core_run(&reg_data);
    reg_data_core_run(&reg_data);

    return 0;
}

void zst_event_cb(zst_event_t *e)
{
    printf("code : %d\n", zst_event_get_code(e));
}
