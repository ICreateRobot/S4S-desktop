/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 15:26:46
 * @LastEditTime : 2026-07-15 11:39:56
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
    void   send_bytes(const String &str);
    void   send_bytes(const char *str);
    size_t read_bytes(uint8_t *buffer, uint16_t size);
    String read_string(void);
    String read_bytes_until(char end);
    void   clear(void);
    int    available(void);
    int    read(void);

    int    print(const std::string &str, char end = 0);
    int    print(const String &str, char end = 0);
    int    print(const char *str, char end = 0);
    int    print(int value, char end = 0);
    int    print(float value, char end = 0);
private:
    UART *_serial;
};

void if_uart_init(void);


extern if_uart if_uart_comm;
extern if_uart if_uart_audio;

#endif /* __IF_COMM_UART_H__ */
