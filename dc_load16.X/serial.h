#ifndef KS_SERIAL_H
#define KS_SERIAL_H

#include "int_types.h"

typedef enum UartError_
{
    UART_OVERFLOW = -1
} UartError;

uint8_t serial1TxBufferIsFull(void);

void serial1RXInterrupt(void);

void serial1TXInterrupt(void);

void serial1SendByteBlocking(uint8_t byte);

void serial1SendBlocking(const void* buffer, uint16_t size);

void serial1SendStringBlocking(const void* str);

void serial1SendLineBlocking(const void* str);

int16_t serial1ReadByte(void);

uint16_t serial1Read(void* buffer, uint16_t maxSize);

void initSerial1(void);

#endif