/*
 * File:   newmain.c
 * Author: kurt
 *
 * Created on January 27, 2018, 4:27 PM
 */

#include "config.h"
#include "IntTypes.h"
#include "CircularBuffer.h"
#include <xc.h>
#include <stdlib.h>

#define OSC_FREQ (32UL * 1000 * 1000)

// UART_SPBRG = OSC_FREQ / (4 * UART_BAUD) - 1
// The intermediate value is multiplied by 10 and added with 5 to
// round to the nearest integer value after dividing by 10 again.
#define UART_BAUD 115200UL
#define UART_SPBRG ((((OSC_FREQ) * 10) / (4 * (UART_BAUD)) + 5) / 10 - 1)

void initializeIO(void)
{
    // Initialize ports all set to output LOW, analog disabled
    PORTA = PORTB = PORTC = 0;
    LATA = LATB = LATC = 0;
    ANSELA = ANSELB = ANSELC = 0;
    TRISA = TRISB = TRISC = 0;
}

void initializePeripheralPower(void)
{
    // Disable all peripheral power
    PMD0 = PMD1 = PMD2 = PMD3 = PMD4 = PMD5 = 1;

    // Enable individual peripherals required
    PMD0bits.SYSCMD = 0;        // Peripheral use of FOSC
    PMD4bits.UART1MD = 0;       // Enable UART power
}

void initializeInterrupts(void)
{
    // Disable all interrupt enable flags
    PIE1 = PIE2 = PIE3 = PIE4 = 0;

    // Disable all interrupt flags
    PIR1 = PIR2 = PIR3 = PIR4 = 0;

    // Enable peripheral interrupts
    INTCONbits.PEIE = 1;

    // Allow interrupts globally
    INTCONbits.GIE = 1;
}

void initializeUart(void)
{
    RC1STAbits.SPEN = 0;            // Disable UART while configuring
    RC1STAbits.SREN = 0;            // Not using single-receive mode
    RC1STAbits.RX9 = 0;             // 8-bit mode

    TX1STAbits.BRGH = 1;            // High speed
    TX1STAbits.SYNC = 0;            // Asynchronous
    TX1STAbits.TX9 = 0;             // 8-bit mode

    BAUD1CONbits.ABDEN = 0;         // Disable auto baud detection
    BAUD1CONbits.WUE = 0;           // Turn off wake-up enable
    BAUD1CONbits.BRG16 = 1;         // 16-bit Baud rate generator
    BAUD1CONbits.SCKP = 0;          // Idle-high clock polarity

    // Set baud rate
    SP1BRGL = (uint8_t)UART_SPBRG;
    SP1BRGH = (uint8_t)(UART_SPBRG >> 8);

    RB6PPS = 0b10100;               // Assign TX to RB6
    RXPPS = 0b01101;                // Assign RX to RB5

    // Make RX pin an input
    TRISBbits.TRISB5 = 1;

    // Enable RX interrupts but disable TX interrupt until data is ready
    // to send
    PIE1bits.RCIE = 1;
    PIE1bits.TXIE = 0;

    // Enable the UART
    TX1STAbits.TXEN = 1;
    RC1STAbits.CREN = 1;
    RC1STAbits.SPEN = 1;
}

#define UART_CIRCULAR_BUFFER_SIZE 8
MAKE_CIRCULAR_BUFFER(uartRXBuffer, UART_CIRCULAR_BUFFER_SIZE);
MAKE_CIRCULAR_BUFFER(uartTXBuffer, UART_CIRCULAR_BUFFER_SIZE);

void interrupt interruptHandler(void)
{
    if (PIR1bits.TXIF)
    {
        int16_t ch = circularBufferReadByte(uartTXBuffer);
        if (ch != CIRCULAR_BUFFER_EMPTY)
            TX1REG = (uint8_t)ch;
        else
            PIE1bits.TXIE = 0;
    }

    if (PIR1bits.RCIF)
    {
        if (RC1STAbits.OERR)
        {
            RC1STAbits.CREN = 0;
            RC1STAbits.CREN = 1;
        } else
            circularBufferWriteByte(uartRXBuffer, RC1REG);
    }
}

void uartSendByte(uint8_t byte)
{
    di();
    circularBufferWriteByte(uartTXBuffer, byte);
    ei();

    PIE1bits.TXIE = 1;
}

int16_t uartReadByte(void)
{
    di();
    int16_t byte = circularBufferReadByte(uartRXBuffer);
    ei();

    return byte;
}

void main(void) {
    initializeIO();
    initializePeripheralPower();
    initializeInterrupts();
    initializeUart();

    for(;;)
    {
        int16_t ch = uartReadByte();
        if (ch != CIRCULAR_BUFFER_EMPTY)
        {
            switch ((uint8_t)ch)
            {
                case '\n':
                    // Ignore
                    break;

                case '\r':
                    uartSendByte('\r');
                    uartSendByte('\n');
                    break;

                default:
                    uartSendByte((uint8_t)ch);
                    break;
            }
        }
    }
}
