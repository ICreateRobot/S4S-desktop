#include "zst_soft_i2c.h"

#if (ZST_SOFT_I2C == 1)

#define sda_output(self)                              \
    do                                                \
    {                                                 \
        if (self->sda_output) self->sda_output(self); \
    } while (0)

#define sda_input(self)                               \
    do                                                \
    {                                                 \
        if (self->sda_input) self->sda_input(self);   \
    } while (0)

void zst_soft_i2c_init(zst_soft_i2c_t * self)
{
    self->delay_time_us = 5;
    self->waitack_delay_time_us = 5;
}


static void zst_soft_i2c_start(zst_soft_i2c_t * self)
{
    sda_output(self);

    self->sda_write(self, 1);
    self->scl_write(self, 1);
    self->delay(self, self->delay_time_us);
    self->sda_write(self, 0);
    self->delay(self, self->delay_time_us);
    self->scl_write(self, 0);
}

static void zst_soft_i2c_repeated_start(zst_soft_i2c_t * self)
{
    sda_output(self);
    
    // 1. 确保 SDA 为高电平
    self->sda_write(self, 1); 
    self->delay(self, self->delay_time_us);
    
    // 2. 拉高 SCL
    self->scl_write(self, 1);
    self->delay(self, self->delay_time_us);
    
    // 3. 在 SCL 为高时，将 SDA 从高拉低 (这就是 START 信号定义)
    self->sda_write(self, 0);
    self->delay(self, self->delay_time_us);
    
    // 4. 钳住 SCL，准备发送接下来的地址
    self->scl_write(self, 0);
    self->delay(self, self->delay_time_us);
}

static void zst_soft_i2c_stop(zst_soft_i2c_t * self)
{
    sda_output(self);

    self->sda_write(self, 0);
    self->scl_write(self, 1);
    self->delay(self, self->delay_time_us);
    self->sda_write(self, 1);
    self->delay(self, self->delay_time_us);
}


static void zst_soft_i2c_ack(zst_soft_i2c_t * self)
{
    sda_output(self);

    self->sda_write(self, ZST_SOFT_I2C_ACK);
    self->scl_write(self, 1);
    self->delay(self, self->delay_time_us);
    self->scl_write(self, 0);
    self->delay(self, self->delay_time_us);
}


static void zst_soft_i2c_nack(zst_soft_i2c_t * self)
{
    sda_output(self);

    self->sda_write(self, ZST_SOFT_I2C_NACK);
    self->scl_write(self, 1);
    self->delay(self, self->delay_time_us);
    self->scl_write(self, 0);
    self->delay(self, self->delay_time_us);
}


static uint8_t zst_soft_i2c_wait_ack(zst_soft_i2c_t * self)
{
    uint16_t timeout=0;
    uint8_t ack = ZST_SOFT_I2C_NACK;

    sda_input(self);
    self->sda_write(self, 1);
    self->delay(self, self->delay_time_us);
    self->scl_write(self, 1);
    self->delay(self, self->delay_time_us);
    do
    {
        ack = self->sda_read(self);
        if (ack == ZST_SOFT_I2C_ACK) break;
        self->delay(self, 1);
    } while ((++timeout) < self->waitack_delay_time_us);

    self->scl_write(self, 0);
    self->delay(self, self->delay_time_us);

    return ack;
}


static void zst_soft_i2c_write_byte(zst_soft_i2c_t * self, uint8_t data)
{
    sda_output(self);
    self->scl_write(self, 0);
    for (uint8_t i = 0; i < 8; i++)
    {
        self->sda_write(self, (data >> 7) & 0x01);
        data <<= 1;
        self->delay(self, self->delay_time_us);
        self->scl_write(self, 1);
        self->delay(self, self->delay_time_us);
        self->scl_write(self, 0);
        self->delay(self, self->delay_time_us);
    }
}


static uint8_t zst_soft_i2c_read_byte(zst_soft_i2c_t * self, uint8_t ack)
{
    uint8_t data = 0;
    sda_input(self);

    for (uint8_t i = 0; i < 8; i++)
    {
        data <<= 1;
        self->scl_write(self, 1);
        self->delay(self, self->delay_time_us);
        if (self->sda_read(self))
        {
            data |= 0x01;
        }
        self->scl_write(self, 0);
        self->delay(self, self->delay_time_us);
    }
    if (ack == ZST_SOFT_I2C_ACK)
        zst_soft_i2c_ack(self);
    else
        zst_soft_i2c_nack(self);

    return data;
}

void zst_soft_i2c_set_delay_time_us(zst_soft_i2c_t * self, uint16_t delay_time_us)
{
    self->delay_time_us = delay_time_us;
}


void zst_soft_i2c_set_waitack_delay_time_us(zst_soft_i2c_t * self, uint16_t waitack_delay_time_us)
{
    self->waitack_delay_time_us = waitack_delay_time_us;
}


uint16_t zst_soft_i2c_get_delay_time_us(zst_soft_i2c_t * self)
{
    return self->delay_time_us;
}


uint16_t zst_soft_i2c_get_waitack_delay_time_us(zst_soft_i2c_t * self)
{
    return self->waitack_delay_time_us;
}

/**
 * @description: 检查设备是否在线
 * @param {zst_soft_i2c_t *} self
 * @param {uint8_t} device_address
 * @return {int} 0: 设备在线; -1: 设备未响应
 */
int zst_soft_i2c_is_device_ready(zst_soft_i2c_t * self, uint8_t device_address)
{
    int result = -1;

    // 1. 发送起始信号
    zst_soft_i2c_start(self);

    // 2. 发送写地址 (device_address << 1 | W)
    // 即使只是探测，通常也使用写模式
    zst_soft_i2c_write_byte(self, (device_address << 1) | ZST_SOFT_I2C_W);

    // 3. 检测是否有 ACK
    if (zst_soft_i2c_wait_ack(self) == ZST_SOFT_I2C_ACK)
    {
        result = 0; // 收到应答，设备在线
    }

    // 4. 发送停止信号释放总线
    zst_soft_i2c_stop(self);

    return result;
}


int zst_soft_i2c_write_bytes(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * data, uint16_t len)
{
    if (0 == len || NULL == data) return -1;

    zst_soft_i2c_start(self);
    zst_soft_i2c_write_byte(self, (device_address << 1) | ZST_SOFT_I2C_W);
    if (zst_soft_i2c_wait_ack(self) == ZST_SOFT_I2C_NACK)
    {
        zst_soft_i2c_stop(self);
        return -1;
    }

    for (uint16_t i=0; i<len; i++)
    {
        zst_soft_i2c_write_byte(self, data[i]);
        if (zst_soft_i2c_wait_ack(self) == ZST_SOFT_I2C_NACK)
        {
            zst_soft_i2c_stop(self);
            return -1;
        }
    }
    zst_soft_i2c_stop(self);
    return 0;
}

int zst_soft_i2c_read_bytes(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * data, uint16_t len)
{
    if (0 == len || NULL == data) return -1;
    
    zst_soft_i2c_start(self);
    zst_soft_i2c_write_byte(self, (device_address << 1) | ZST_SOFT_I2C_R);
    
    if (zst_soft_i2c_wait_ack(self) != ZST_SOFT_I2C_ACK) 
    {
        zst_soft_i2c_stop(self);
        return -1;
    }

    for (uint16_t i = 0; i < len; i++) 
    {
        // 最后一个字节发送 NACK，其余发送 ACK
        uint8_t ack_type = (i == len - 1) ? ZST_SOFT_I2C_NACK : ZST_SOFT_I2C_ACK;
        data[i] = zst_soft_i2c_read_byte(self, ack_type);
    }

    zst_soft_i2c_stop(self);
    return 0;
}

int zst_soft_i2c_write_double(zst_soft_i2c_t * self, uint8_t device_address, uint8_t * reg_data, uint16_t reg_data_len, uint8_t * data, uint16_t data_len)
{
    if (0 == reg_data_len || NULL == reg_data || 0 == data_len || NULL == data) return -1;

    zst_soft_i2c_start(self);
    zst_soft_i2c_write_byte(self, (device_address << 1) | ZST_SOFT_I2C_W);
    if (ZST_SOFT_I2C_NACK == zst_soft_i2c_wait_ack(self)) {
        zst_soft_i2c_stop(self);
        return -1;
    }

    // 写入寄存器地址部分
    for (uint16_t i = 0; i < reg_data_len; i++) {
        zst_soft_i2c_write_byte(self, reg_data[i]);
        if (ZST_SOFT_I2C_NACK == zst_soft_i2c_wait_ack(self)) {
            zst_soft_i2c_stop(self);
            return -1;
        }
    }

    // 写入数据部分
    for (uint16_t i = 0; i < data_len; i++) {
        zst_soft_i2c_write_byte(self, data[i]);
        if (ZST_SOFT_I2C_NACK == zst_soft_i2c_wait_ack(self)) {
            zst_soft_i2c_stop(self);
            return -1;
        }
    }

    zst_soft_i2c_stop(self);
    return 0;
}

int zst_soft_i2c_read_double(zst_soft_i2c_t *self, uint8_t device_address, uint8_t *reg_data, uint16_t reg_data_len, uint8_t *data, uint16_t data_len)
{
    if (0 == reg_data_len || NULL == reg_data || 0 == data_len || NULL == data) return -1;

    int result = 0;
    zst_soft_i2c_start(self);
    zst_soft_i2c_write_byte(self, (device_address << 1) | ZST_SOFT_I2C_W);

    // 使用 if 链或状态检查
    if (zst_soft_i2c_wait_ack(self) == ZST_SOFT_I2C_ACK)
    {
        // 1. 发送寄存器地址
        for (uint16_t i = 0; i < reg_data_len; i++)
        {
            zst_soft_i2c_write_byte(self, reg_data[i]);
            if (zst_soft_i2c_wait_ack(self) == ZST_SOFT_I2C_NACK)
            {
                result = -1;
                break;
            }
        }

        // 2. 只有当前面成功了才继续读
        if (result == 0)
        {
            zst_soft_i2c_repeated_start(self); // Repeated Start
            zst_soft_i2c_write_byte(self, (device_address << 1) | ZST_SOFT_I2C_R);
            if (zst_soft_i2c_wait_ack(self) == ZST_SOFT_I2C_ACK)
            {
                for (uint16_t i = 0; i < data_len; i++)
                {
                    // 最后一个字节给 NACK
                    uint8_t ack_type = (i == data_len - 1) ? ZST_SOFT_I2C_NACK : ZST_SOFT_I2C_ACK;
                    data[i] = zst_soft_i2c_read_byte(self, ack_type);
                }
            }
            else
            {
                result = -1;
            }
        }
    }
    else
    {
        result = -1;
    }

    zst_soft_i2c_stop(self);
    return result;
}

int zst_soft_i2c_write_reg(zst_soft_i2c_t * self, uint8_t device_address, uint8_t reg, uint8_t * data, uint16_t len)
{
    return zst_soft_i2c_write_double(self, device_address, &reg, 1, data, len);
}

int zst_soft_i2c_read_reg(zst_soft_i2c_t * self, uint8_t device_address, uint8_t reg, uint8_t * data, uint16_t len)
{
    return zst_soft_i2c_read_double(self, device_address, &reg, 1, data, len);
}

#endif
