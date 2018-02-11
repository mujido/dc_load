/*
 * File:   main.c
 * Author: kurt
 *
 * Created on February 9, 2018, 11:40 AM
 */

#include "config.h"
#include "mcu.h"
#include "circular_buffer.h"
#include "serial.h"
#include "lineedit.h"
#include <string.h>
#include <libpic30.h>

void initIO(void)
{
    TRISA = TRISB = 0;
    LATA = LATB = 0;
    AD1PCFGL = 0xFFFF;
}

extern const char crlf[];
const char crlf[2] = { '\r', '\n' };

int main(void)
{
    RCONbits.SWDTEN = 0;        // Disable watchdog timer

    initIO();
    initSerial1();

    __delay_ms(10);     // Allow time for system to startup

    serial1SendLineBlocking("Starting...");

    for (;;)
    {
        LineEditStatus leStatus = lineEditReadSerial();
        if (leStatus == LINE_EDIT_EOL)
        {
            serial1SendStringBlocking("echo: ");
            serial1SendBlocking(lineContext.lineBuf_, lineContext.length_);
            serial1SendBlocking(crlf, sizeof(crlf));
            lineEditClear();
        }
    }
    return 0;
}
