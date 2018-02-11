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

    uint8_t buf[80];
    uint16_t bufLength = 0;

    serial1SendLineBlocking("Starting...");

    for (;;)
    {
        bufLength += serial1Read(buf + bufLength, sizeof(buf) - bufLength);
        if (bufLength == sizeof(buf))
        {
            serial1SendLineBlocking("Buffer overflow!");
            bufLength = 0;
        }


        uint8_t* eol = memchr((char*)buf, '\r', bufLength);
        if (eol != NULL)
        {
            serial1SendStringBlocking("echo: ");
            serial1SendBlocking(buf, eol - buf);
            serial1SendBlocking(crlf, sizeof(crlf));

            bufLength -= eol - buf + 1;
            if (bufLength && buf[bufLength - 1] == '\n')
            {
                --bufLength;
                ++eol;
            }

            if (bufLength > 0)
                memmove(buf, eol + 1, bufLength);
        }
    }
    return 0;
}
