/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 15:26:46
 * @LastEditTime : 2026-04-23 17:59:41
 * @Description  : 串口通信
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#ifndef __IF_UART_H__
#define __IF_UART_H__

#include "main.h"

class if_uart
{
public:
    if_uart();
    ~if_uart();

public:
    void   begin(UART *serial);
    void   config_baud(uint32_t baudrate);
    void   send_bytes(uint8_t *buffer, uint16_t size);
    void   send_bytes(const std::string &str);
    void   send_bytes(const char *str);
    size_t read_bytes(uint8_t *buffer, uint16_t size);
    String read_string(void);
    String read_bytes_until(char end);
    void   clear(void);
    int    available(void);
    int    read(void);

private:
    UART *_serial;
};

void if_uart_init(void);


extern if_uart if_uart_comm;
extern if_uart if_uart_audio;

#endif /* __IF_COMM_UART_H__ */
