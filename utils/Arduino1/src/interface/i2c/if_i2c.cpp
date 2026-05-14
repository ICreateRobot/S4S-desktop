/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 16:55:24
 * @LastEditTime : 2026-04-28 14:25:00
 * @Description  : i2c 接口
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "if_i2c.h"
#include "platform/pf_rtos.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "if_i2c"


/********************
 * global functions
 *******************/
void if_i2c_init(void)
{
    Wire.begin();
    Wire1.begin();
    if_i2c_internal_handle.begin(&Wire);
    if_i2c_external_handle.begin(&Wire1);
}


/********************
 * class functions
 *******************/
if_i2c::if_i2c(void)
{}

if_i2c::~if_i2c(void)
{}

void if_i2c::begin(TwoWire *wire)
{
    if (NULL == wire)
    {
        // arduino
        Wire.begin();

        // esp32
        // Wire.begin(42, 41);
        this->_wire = &Wire;
    } else
    {
        this->_wire = wire;
    }
}

int if_i2c::write_bytes(uint8_t addr, uint8_t *buf, uint16_t len)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    uint8_t  ret_val     = 0;
    TwoWire *wire_handle = this->_wire;
    if (NULL == wire_handle)
    {
        ZST_LOGE(LOG_TAG, "wire handle is null");
        ret_val = -1;
        goto exit;
    }

    wire_handle->beginTransmission(addr);
    wire_handle->write(buf, len);
    if (0 != wire_handle->endTransmission())
    {
        ret_val = -2;
        goto exit;
    }

exit:
    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return ret_val;
}

int if_i2c::read_bytes(uint8_t addr, uint8_t *buf, uint16_t len)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    int      ret_val     = 0;
    TwoWire *wire_handle = this->_wire;
    if (NULL == wire_handle)
    {
        ZST_LOGE(LOG_TAG, "wire handle is null");
        ret_val = -1;
        goto exit;
    }

    wire_handle->requestFrom(addr, len);
    while (wire_handle->available())
    {
        *buf = wire_handle->read();
        buf++;
    }

exit:
    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
    return ret_val;
}

int if_i2c::write_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    int      ret_val     = 0;
    TwoWire *wire_handle = this->_wire;
    if (NULL == wire_handle)
    {
        ZST_LOGE(LOG_TAG, "wire handle is null");
        ret_val = -1;
        goto exit;
    }
    wire_handle->beginTransmission(addr);
    wire_handle->write(reg);
    wire_handle->write(buf, len);
    if (0 != wire_handle->endTransmission())
    {
        ret_val = -2;
        goto exit;
    }

exit:
    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return ret_val;
}

int if_i2c::read_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    int      ret_val     = 0;
    TwoWire *wire_handle = this->_wire;
    if (NULL == wire_handle)
    {
        ZST_LOGE(LOG_TAG, "wire handle is null");
        ret_val = -1;
        goto exit;
    }
    wire_handle->beginTransmission(addr);
    wire_handle->write(reg);
    if (0 != wire_handle->endTransmission(false))
    {
        ret_val = -2;
        goto exit;
    }

    wire_handle->requestFrom(addr, len);
    while (wire_handle->available())
    {
        *buf = wire_handle->read();
        buf++;
    }

exit:
    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);
    return ret_val;
}

std::vector<uint8_t> if_i2c::scan(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::I2C, -1);

    TwoWire             *wire_handle = this->_wire;
    std::vector<uint8_t> addr_list;
    int                  error = 0;
    for (uint8_t addr = 1; addr < 127; addr++)
    {
        wire_handle->beginTransmission(addr);
        error = wire_handle->endTransmission();
        if (0 == error) { addr_list.push_back(addr); }
    }

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::I2C);

    return addr_list;
}

TwoWire *if_i2c::get_wire(void)
{
    return this->_wire;
}

TwoWire &if_i2c::get_wire_ref(void)
{
    return *this->_wire;
}

if_i2c if_i2c_internal_handle;
if_i2c if_i2c_external_handle;
