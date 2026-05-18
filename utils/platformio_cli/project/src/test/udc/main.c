#include <stdio.h>
#include "udc.h"

udc_pack_group_t udcPackGroup = {0};
udc_pack_t pack = {0};

static uint8_t pack_send_buf[100];
void udc_init(void);

int main()
{
    udc_init();
    udc_pack_set_buffer_static(&pack, UDC_PACK_TRANSMIT, pack_send_buf, sizeof(pack_send_buf));

    char *target_name = "\"vision\"";
    char *function_name = "\"card_detected\"";
    char *param[] = {"\"EVENT_TARGET\"", "1"};


    udc_pack_append_data(&pack, 10, strlen(target_name), target_name);
    udc_pack_append_data(&pack, 11, strlen(function_name), function_name);

    for (uint8_t param_pos=0; param_pos<sizeof(param)/sizeof(param[0]); param_pos++)
    {
        udc_pack_append_data(&pack, 12+param_pos, strlen(param[param_pos]), param[param_pos]);
    }

    udc_pack_task(&udcPackGroup);

    return 0;
}



static int send_bytes(const struct _udc_pack_t *pack, const uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%02x ", buf[i]);
    }
    return len;
}

static int calculate_verify(const struct _udc_pack_t *pack, const uint8_t *buf, uint16_t len, uint8_t *verify)
{
    *verify = 0x55;
    return 0;
}

void udc_init(void)
{
    udc_pack_init_t pack_init = {
        .pack_group = &udcPackGroup,
        .pack = &pack,
        .header = 
        {
            .header = "\xAA\x01",
            .header_len = 2,
        },
        .verify = 
        {
            .calculate_verify = calculate_verify,
            .verify_len = 1
        }
    };
    udc_pack_init(&pack_init);
    udc_pack_set_buffer_static(&pack, UDC_PACK_TRANSMIT, pack_send_buf, sizeof(pack_send_buf));
    udc_pack_set_send_bytes_func(&pack, send_bytes);
}
