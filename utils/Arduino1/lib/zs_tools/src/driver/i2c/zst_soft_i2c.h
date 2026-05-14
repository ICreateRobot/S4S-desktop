#ifndef __zst_soft_i2c_H__
#define __zst_soft_i2c_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "zst_conf.h"
#if (ZST_SOFT_I2C == 1)

#define ZST_SOFT_I2C_W     0
#define ZST_SOFT_I2C_R     1 

#define ZST_SOFT_I2C_ACK   0
#define ZST_SOFT_I2C_NACK  1


typedef struct zst_soft_i2c zst_soft_i2c_t;


struct zst_soft_i2c
{
    void (*sda_input)(zst_soft_i2c_t * self);  // 对于开漏输出，可以忽略
    void (*sda_output)(zst_soft_i2c_t * self); // 对于开漏输出，可以忽略
    void (*delay)(zst_soft_i2c_t *self, uint16_t time_us);
    void (*scl_write)(zst_soft_i2c_t *self, uint8_t);
    void (*sda_write)(zst_soft_i2c_t *self, uint8_t);
    uint8_t (*sda_read)(zst_soft_i2c_t *self);

    uint16_t delay_time_us; // i2c 通讯时钟延时；默认5us，即i2c速度为100k
    uint16_t waitack_delay_time_us; //等待应答时间

    void * user_data;
};

void zst_soft_i2c_init(zst_soft_i2c_t * self);

void zst_soft_i2c_set_delay_time_us(zst_soft_i2c_t * self, uint16_t delay_time_us);
void zst_soft_i2c_set_waitack_delay_time_us(zst_soft_i2c_t * self, uint16_t waitack_delay_time_us);

uint16_t zst_soft_i2c_get_delay_time_us(zst_soft_i2c_t * self);
uint16_t zst_soft_i2c_get_waitack_delay_time_us(zst_soft_i2c_t * self);


int zst_soft_i2c_is_device_ready(zst_soft_i2c_t * self, uint8_t device_address);
int zst_soft_i2c_write_bytes(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * data, uint16_t len);
int zst_soft_i2c_read_bytes(zst_soft_i2c_t * self,  uint8_t device_address, uint8_t * data, uint16_t len);
int zst_soft_i2c_write_double(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * reg_data, uint16_t reg_data_len, uint8_t * data, uint16_t data_len);
int zst_soft_i2c_read_double(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * reg_data, uint16_t reg_data_len, uint8_t * data, uint16_t data_len);
int zst_soft_i2c_write_reg(zst_soft_i2c_t * self, uint8_t device_address, uint8_t reg, uint8_t * data, uint16_t len);
int zst_soft_i2c_read_reg(zst_soft_i2c_t * self, uint8_t device_address, uint8_t reg, uint8_t * data, uint16_t len);

#endif
#ifdef __cplusplus
}
#endif

#endif /* __zst_soft_i2c_H__ */
