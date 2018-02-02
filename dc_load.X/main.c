/*
 * File:   newmain.c
 * Author: kurt
 *
 * Created on January 27, 2018, 4:27 PM
 */

#include "config.h"
#include "IntTypes.h"
#include "CircularBuffer.h"
#include "serial.h"
#include <xc.h>
#include <stdlib.h>
#include <string.h>

// UART_SPBRG = OSC_FREQ / (4 * UART_BAUD) - 1
// The intermediate value is multiplied by 10 and added with 5 to
// round to the nearest integer value after dividing by 10 again.
#define UART_BAUD 115200UL
#define UART_SPBRG ((((_XTAL_FREQ) * 10) / (4 * (UART_BAUD)) + 5) / 10 - 1)

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

void interrupt interruptHandler(void)
{
    serial1InterruptHandler();
}

void main(void) {
    initializeIO();
    initializePeripheralPower();
    initializeInterrupts();
    initializeUart();


    static uint8_t line[81];

    for(;;)
    {
        int16_t lineLength;

        lineLength = serial1ReadLineBlocking(line, sizeof(line) - 1);
        if (lineLength == UART_OVERFLOW)
        {
            // Read until end of line
            while (UART_OVERFLOW == serial1ReadLineBlocking(line, sizeof(line)))
                ;

            serial1SendLineBlocking("Input overflow");
            continue;
        }

        line[(uint8_t)lineLength] = 0;
        serial1SendStringBlocking("echo: ");
        serial1SendLineBlocking(line);
    }
}
