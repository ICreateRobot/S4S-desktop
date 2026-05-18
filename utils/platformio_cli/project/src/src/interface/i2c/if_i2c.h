/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 16:55:32
 * @LastEditTime : 2026-04-17 16:04:12
 * @Description  : i2c 接口
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __IF_I2C_H__
#define __IF_I2C_H__

#include "main.h"
#include "Wire.h"
#include "vector"

class if_i2c
{
public:
    if_i2c();
    ~if_i2c();

public:
    void begin(TwoWire *wire = NULL);
    int  write_bytes(uint8_t addr, uint8_t *buf, uint16_t len);
    int  read_bytes(uint8_t addr, uint8_t *buf, uint16_t len);
    int  write_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
    int  read_reg(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len);
    std::vector<uint8_t>scan(void);

    TwoWire *get_wire(void);
    TwoWire &get_wire_ref(void);

private:
    TwoWire *_wire;
};

extern if_i2c if_i2c_internal_handle; // 内部与主板通讯
extern if_i2c if_i2c_external_handle; // 外部与传感器通讯

void if_i2c_init(void);

#endif /* __IF_I2C_H__ */
