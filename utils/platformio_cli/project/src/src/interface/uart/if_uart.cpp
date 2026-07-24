/*
 * @Author       : 蔡雅超 (ZIShen)
 * @LastEditors  : zishen
 * @Date         : 2025-11-26 15:26:37
 * @LastEditTime : 2026-07-15 11:38:44
 * @Description  : 交互串口
 * Copyright (c) 2025 Author 蔡雅超 email: 2672632650@qq.com, All Rights Reserved.
 */
#include "if_uart.h"
#include "stdio.h"
#include "platform/pf_rtos.h"


/******************
 * data struct
 *****************/
#define LOG_TAG "if_comm_uart"


/****************************
 * function declaration
 ***************************/


/********************
 * static variables
 *******************/


/********************
 * global variables
 *******************/


/********************
 * global functions
 *******************/
void if_uart_init(void)
{
    /******************
     * 初始化串口
     *****************/
    Serial.begin(115200);
    Serial2.begin(115200);
    if_uart_comm.begin(&Serial);
    if_uart_audio.begin(&Serial2);
}


/***************
 * 重定向printf
 ***************/

// 文件描述符，通常不检查，但最好处理一下
int _write(int file, char *ptr, int len)
{
    (void)file; // 避免未使用的变量警告
    int i;
    for (i = 0; i < len; i++)
    {
        // 核心重定向逻辑：将每个字符写入串口
        Serial.write(ptr[i]);
    }
    return len;
}

// 在某些系统上，可能需要重写 fputc
int __io_putchar(int ch)
{
    Serial.write((uint8_t)ch);
    return ch;
}

/********************
 * class functions
 *******************/
if_uart::if_uart()
{}

if_uart::~if_uart()
{}

void if_uart::begin(UART *serial)
{
    this->_serial = serial;
}

void if_uart::config_baud(uint32_t baudrate)
{
    this->_serial->begin(baudrate);
}

void if_uart::send_bytes(uint8_t *buffer, uint16_t size)
{
    if (0 == size) return;
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::UART, -1);

    this->_serial->write(buffer, size);
    this->_serial->flush();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::UART);
}

void if_uart::send_bytes(const char *str)
{
    send_bytes((uint8_t *)str, (uint16_t)strlen(str));
}

void if_uart::send_bytes(const std::string &str)
{
    send_bytes(str.c_str());
}

void if_uart::send_bytes(const String &str)
{
    send_bytes(str.c_str());
}

size_t if_uart::read_bytes(uint8_t *buffer, uint16_t size)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::UART, -1);

    size_t read_size = 0;
    read_size        = this->_serial->readBytes(buffer, size);

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::UART);

    return read_size;
}

String if_uart::read_string(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::UART, -1);

    String str = this->_serial->readString();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::UART);

    return str;
}

String if_uart::read_bytes_until(char end)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::UART, -1);

    String str;
    static char buffer[50] = {0};
    size_t read_size = 0;
    read_size        = this->_serial->readBytesUntil(end, buffer, sizeof(buffer));
    for (int i = 0; i < read_size; i++)
    {
        str += buffer[i];
    }
    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::UART);

    return str;
}

void if_uart::clear(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::UART, -1);

    while (this->_serial->available() > 0) this->_serial->read();
    
    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::UART);
}

int if_uart::available(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::UART, -1);

    int available = this->_serial->available();

    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::UART);

    return available;
}

int if_uart::read(void)
{
    pf_rtos_semaphore_take(pf_rtos_binary_semaphore::UART, -1);

    int data = this->_serial->read();
    pf_rtos_semaphore_give(pf_rtos_binary_semaphore::UART);

    return data;
}

int if_uart::print(const std::string &str, char end)
{
    return print(str.c_str(), end);
}

int if_uart::print(const String &str, char end)
{
    return print(str.c_str(), end);
}

int if_uart::print(const char *str, char end)
{
    uint16_t size = 0;
    send_bytes(str);
    size = (uint16_t)strlen(str);
    if (end > 0) 
    {
        send_bytes((uint8_t *)&end, 1);
        size++;
    }
    return size;
}

int if_uart::print(int value, char end)
{
    char buffer[20] = {0};
    snprintf(buffer, sizeof(buffer), "%d", value);
    return print(buffer, end);
}

int if_uart::print(float value, char end)
{
    char buffer[20] = {0};
    snprintf(buffer, sizeof(buffer), "%f", value);
    return print(buffer, end);
}


/****************************
 * static function
 ***************************/


if_uart if_uart_comm;
if_uart if_uart_audio;
