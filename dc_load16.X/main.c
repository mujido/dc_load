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
#include "system_timer.h"
#include <string.h>
#include <libpic30.h>
#include <stdio.h>

#define ADC_AVERAGE_SAMPLES 4

void initIO(void)
{
    // Set all PORTA pins to output LOW
    TRISA = 0;
    LATA = 0;

    // Set B0-RB2,B4-B15 to output LOW. RB3 = ADC input
    TRISB = 1 << 3;
    LATB = 0;

    AD1PCFGL = 0xFFFF;      // make all IO ports digital except RB5/AN3
    AD1PCFGLbits.PCFG5 = 0;
}

void initADC(void)
{
    AD1CON1bits.ADON = 0;       // disable ADC
    AD1CON1bits.SAMP = 0;       // reset any pending sample
    AD1CON1bits.ASAM = 0;       // Manual sample start mode
    AD1CON1bits.SSRC = 0;       // Manual transition between sample and convert
    AD1CON1bits.FORM = 0b00;    // Integer conversion mode
    AD1CON1bits.AD12B = 1;      // 12-bit mode
    AD1CON1bits.ADSIDL = 0;     // Module runs in IDLE mode

    // Sample A only, CH0 only, AVdd/AVss for volt references
    AD1CON2 = 0;

    AD1CON3bits.ADCS = 0;       // TAD = 1 TCY
    AD1CON3bits.ADRC = 0;       // ADC clock is FCY

    AD1CHS0bits.CH0SA = 5;      // CH0+ = AD5
    AD1CHS0bits.CH0NA = 0;      // CH0- = VREFL

    AD1CSSL = 0;                // Disable input scanning

    AD1CON1bits.ADON = 1;       // Enable ADC module
}

enum ADCState
{
    ADC_INACTIVE,
    ADC_SAMPLING,
    ADC_CONVERTING
};

extern const char crlf[];
const char crlf[2] = { '\r', '\n' };

static char stdoutBuf[80];

int main(void)
{
    RCONbits.SWDTEN = 0;        // Disable watchdog timer

    initIO();
    systemTimerInit();
    initSerial1();
    initADC();

    __delay_ms(10);     // Allow time for system to startup
    setvbuf(stdout, stdoutBuf, _IOLBF, sizeof(stdoutBuf));

    puts("Starting...");

    Tick blinkInterval = systemTimerMillisecondsToTicks(500);
    Tick nextBlink = systemTimerGetCurrent() + blinkInterval;

    uint16_t adcSum = 0;
    uint8_t adcSamples = 0;
    Tick adcSampleInterval = systemTimerMillisecondsToTicks((2500 / ADC_AVERAGE_SAMPLES + 5) / 10);
    Tick adcSampleTicks = 1;    // sample and conversion time are short
    Tick adcConvertTicks = 1;   // compared to tick period
    Tick nextAdcEvent = systemTimerGetCurrent() + adcSampleInterval;
    uint8_t adcState = ADC_INACTIVE;

    LATBbits.LATB3 = 1;

    for (;;)
    {
        Tick currentTime = systemTimerGetCurrent();

        if (currentTime >= nextBlink)
        {
            LATA ^= 1;
            nextBlink = currentTime + blinkInterval;
        }

        if (currentTime >= nextAdcEvent)
        {
            uint16_t adcValue;

            switch (adcState)
            {
            case ADC_INACTIVE:
                AD1CON1bits.SAMP = 1;  // start sampling
                nextAdcEvent = currentTime + adcSampleTicks;
                adcState = ADC_SAMPLING;
                break;

            case ADC_SAMPLING:
                AD1CON1bits.SAMP = 0;  // start conversion
                nextAdcEvent = currentTime + adcConvertTicks;
                adcState = ADC_CONVERTING;
                break;

            case ADC_CONVERTING:
                nextAdcEvent = currentTime + adcSampleInterval;
                adcState = ADC_INACTIVE;

                if (!AD1CON1bits.DONE)
                {
                    puts("ADC not done");
                    break;
                }

                adcValue = ADC1BUF0;
                adcSum += adcValue;
                ++adcSamples;

                if (adcSamples >= ADC_AVERAGE_SAMPLES)
                {
                    printf("%-8lu: %u\n", currentTime, (uint16_t)(adcSum / ADC_AVERAGE_SAMPLES));
                    adcSamples = 0;
                    adcSum = 0;
                }
                break;
            }
        }

        LineEditStatus leStatus = lineEditReadSerial();
        if (leStatus == LINE_EDIT_EOL)
        {
            if (lineContext.lineBuf_[0] == '\0')
            {
                // Ignore
            }
            else if (strcmp(lineContext.lineBuf_, "adc pause") == 0)
            {
                adcState = ADC_INACTIVE;
                nextAdcEvent = UINT32_MAX;
                puts("ADC paused");
            }
            else if (strcmp(lineContext.lineBuf_, "adc run") == 0)
            {
                adcState = ADC_INACTIVE;
                nextAdcEvent = 0;
                puts("ADC restarting");
            }
            else
            {
                printf("Unknown command: %s\n", lineContext.lineBuf_);
            }

            lineEditClear();
        }

        Idle();
    }
    return 0;
}
