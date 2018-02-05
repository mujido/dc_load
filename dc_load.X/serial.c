#include "serial.h"
#include "CircularBuffer.h"
#include <string.h>

MAKE_CIRCULAR_BUFFER(serial1TxBuffer, 40);
MAKE_CIRCULAR_BUFFER(serial1RxBuffer, 40);

void serial1InterruptHandler(void)
{
    if (PIR1bits.TXIF)
    {
        int16_t ch = circularBufferReadByte(serial1TxBuffer);
        if (ch != CIRCULAR_BUFFER_EMPTY)
        {
//            uint8_t ch2 = (uint8_t)ch;
//
//            if (((uint8_t)ch2 > 127U || (uint8_t)ch2 < ' ') && ch2 != '\r' && ch2 != '\n')
//            {
//                __debug_break();
//                ch = '-';
//            }
            TX1REG = (uint8_t)ch;
        }
        else
            PIE1bits.TXIE = 0;
    }

    if (PIR1bits.RCIF)
    {
        if (RC1STAbits.OERR)
        {
            RC1STAbits.CREN = 0;
            RC1STAbits.CREN = 1;
        }
        else
        {
            uint8_t ch = RC1REG;
            if ((ch > 127 || ch < 20) && ch != '\r' && ch != '\n')
                __debug_break();
            circularBufferWriteByte(serial1RxBuffer, ch);
        }
    }
}

void serial1SendByteBlocking(uint8_t byte)
{
    uint8_t sent = 0;

    while (!sent)
    {
        di();
        sent = circularBufferWriteByte(serial1TxBuffer, byte);
        ei();

    }

    PIE1bits.TXIE = 1;
}

void serial1SendBlocking(const void* buffer, uint8_t size)
{
    const uint8_t* pos = buffer;
    uint8_t remaining = size;

    while (remaining > 0)
    {
        di();
        uint8_t bytesWritten = circularBufferWrite(serial1TxBuffer, pos, remaining);
        ei();

        remaining -= bytesWritten;

        if (bytesWritten > 0)
            PIE1bits.TXIE = 1;
    }
}

void serial1SendStringBlocking(const void* str)
{
    if (!str)
        return;

    serial1SendBlocking(str, strlen(str));
}

void serial1SendLineBlocking(const void* str)
{
    serial1SendStringBlocking(str);
    serial1SendBlocking("\r\n", 2);
}

int16_t serial1ReadByte(void)
{
    di();
    int16_t byte = circularBufferReadByte(serial1RxBuffer);
    ei();

    return byte;
}

uint8_t serial1Read(void __ram* buffer, uint8_t maxSize)
{
    if (*(uint8_t*)0x20 == 0x61U)
        __debug_break();
    if (buffer == (void*)0x017e)
        __debug_break();
    if (buffer == (void*)0x2000)
        __debug_break();

    di();
    if (*(uint8_t*)0x20 == 0x61U)
        __debug_break();
    if (buffer == (void*)0x017e)
        __debug_break();
    if (buffer == (void*)0x2000)
        __debug_break();
    uint8_t readLength = circularBufferRead(serial1RxBuffer, buffer, maxSize);
    if (*(uint8_t*)0x20 == 0x61U)
        __debug_break();
    ei();

    return readLength;
}

