#include "mcu.h"
#include "serial.h"
#include "circular_buffer.h"
#include <string.h>
#include <libpic30.h>

// UART_SPBRG = FCY / (16 * UART_BAUD) - 1
// The intermediate value is multiplied by 10 and added with 5 to
// round to the nearest integer value after dividing by 10 again.
#define UART_BAUD 9600ULL
#define UART_BRG (((FCY * 10) / (16 * (UART_BAUD)) + 5) / 10 - 1)

typedef enum
{
    TX_INT_EMPTY = 0b10 << 14,
    TX_INT_COMPLETE = 0b01 << 14,
    TX_INT_ANY_SPACE = 0b00 << 14
} TransmitInterruptMode;

MAKE_CIRCULAR_BUFFER(serial1TxBuffer, 40);
MAKE_CIRCULAR_BUFFER(serial1RxBuffer, 40);

extern inline void serial1DisableTxInt(void)
{
    IEC0bits.U1TXIE = 0;
}

extern inline void serial1EnableTxInt(void)
{
    IEC0bits.U1TXIE = 1;
}

extern inline void serial1DisableRxInt(void)
{
    IEC0bits.U1RXIE = 0;
}

extern inline void serial1EnableRxInt(void)
{
    IEC0bits.U1RXIE = 1;
}

void serial1SetTXInterruptMode(TransmitInterruptMode mode)
{
    uint16_t tmp = U1STA;
    tmp = (tmp & 0x3FFF) | mode;
    U1STA = tmp;
}

extern inline void serial1StartTransmit(void)
{
    if (U1STAbits.TRMT == 1)
        IFS0bits.U1TXIF = 1;
}

uint8_t serial1TxBufferIsFull(void)
{
    return serial1TxBuffer->length_ == serial1TxBuffer->capacity_;
}

void serial1SendByteBlocking(uint8_t byte)
{
    uint16_t sent = 0;

    do
    {
        serial1DisableTxInt();
        sent = circularBufferWriteByte(serial1TxBuffer, byte);
        serial1EnableTxInt();
        if (sent)
            serial1StartTransmit();
        else
            __delay_us(20);
    } while (!sent);
}

void _serial1SendBlocking(const void* buffer, uint16_t size)
{
    const uint8_t* pos = buffer;
    uint16_t remaining = size;

    while (remaining > 0)
    {
        while (serial1TxBufferIsFull())
            __delay_us(20);

        serial1DisableTxInt();

        uint16_t bytesWritten = circularBufferWrite(serial1TxBuffer, pos, remaining);
        if (bytesWritten > 0)
            serial1StartTransmit();

        serial1EnableTxInt();

        remaining -= bytesWritten;
        pos += bytesWritten;

        if (remaining > 0)
            __delay_us(20);
    }
}

void serial1SendBlocking(const void* buffer, uint16_t size)
{
    const uint8_t* eol = memchr(buffer, '\n', size);
    if (eol == NULL)
    {
        _serial1SendBlocking(buffer, size);
        return;
    }

    const uint8_t* pos = buffer;
    const uint8_t* end = pos + size;

    while (pos != end && eol != NULL)
    {
        uint16_t blockSize = eol - pos;
        _serial1SendBlocking(pos, blockSize);
        _serial1SendBlocking("\r\n", 2);
        pos += blockSize + 1;
        eol = memchr(pos, '\n', end - pos);
    }

    if (pos != end)
        _serial1SendBlocking(pos, end - pos);
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
    serial1DisableRxInt();
    int16_t byte = circularBufferReadByte(serial1RxBuffer);
    serial1EnableRxInt();

    return byte;
}

uint16_t serial1Read(void* buffer, uint16_t maxSize)
{
    serial1DisableRxInt();
    uint16_t readLength = circularBufferRead(serial1RxBuffer, buffer, maxSize);
    serial1EnableRxInt();

    return readLength;
}

void initSerial1(void)
{
    U1MODEbits.STSEL = 0;       // 1 Stop bit
    U1MODEbits.PDSEL = 0b00;    // 8 data bits, no parity
    U1MODEbits.BRGH = 0;        // Low speed mode
    U1MODEbits.ABAUD = 0;       // No auto baud detection
    U1MODEbits.RTSMD = 1;       // No flow control

    U1BRG = UART_BRG;           // Set baud rate

    U1STAbits.URXISEL = 0b00;   // RX interrupt when any chars available
    serial1SetTXInterruptMode(TX_INT_ANY_SPACE);

    IFS0bits.U1TXIF = 0;        // Clear TX interrupt bit
    IFS0bits.U1RXIF = 0;        // Clear RX interrupt

    U1MODEbits.UARTEN = 1;      // Enable UART module
    U1STAbits.UTXEN = 1;        // Enable UART TX

    RPOR7bits.RP14R = 0b00011;  // Set UART1 TX to pin RP14 (pin 25)
    RPINR18bits.U1RXR = 13;     // Set UART1 RX to pin RP15 (pin 26)
    TRISBbits.TRISB13 = 1;      // Set RB15 (pin 26) as input

    __delay_us(1000);           // Allow UART to finish initializing

    serial1EnableRxInt();
}

void __attribute__((interrupt(auto_psv))) _U1RXInterrupt(void)
{
    IFS0bits.U1RXIF = 0;

    // Ignore errors for now
    if (U1STAbits.FERR == 1)
        return;

    if (U1STAbits.OERR == 1)
    {
        U1STAbits.OERR = 0;
        return;
    }

    if (U1STAbits.URXDA == 1)
        circularBufferWriteByte(serial1RxBuffer, U1RXREG);
}

void __attribute__((interrupt(auto_psv))) _U1TXInterrupt(void)
{
    IFS0bits.U1TXIF = 0;

    while (!U1STAbits.UTXBF)
    {
        int16_t ch = circularBufferReadByte(serial1TxBuffer);
        if (ch != CIRCULAR_BUFFER_EMPTY)
        {
            U1TXREG = (uint8_t)ch;
        }
        else
        {
            serial1DisableRxInt();    // disable TX interrupt until more bytes ready
            break;
        }
    }
}

