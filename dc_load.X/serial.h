#ifndef KS_SERIAL_H
#define KS_SERIAL_H

#include "config.h"
#include "IntTypes.h"

typedef enum UartError_
{
    UART_OVERFLOW = -1
} UartError;

#pragma warning push
#pragma warning disable 210

void serial1InterruptHandler(void);

void serial11SendByte(uint8_t byte);

void serial1SendBlocking(const void* buffer, uint8_t size);

void serial1SendStringBlocking(const void* str);

void serial1SendLineBlocking(const void* str);

int16_t serial1ReadByte(void);

uint8_t serial1Read(void* buffer, uint8_t maxSize);

int16_t serial1ReadLineBlocking(void* buffer, uint8_t maxSize);

#pragma warning pop

#endif