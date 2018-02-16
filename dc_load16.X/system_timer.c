#include "system_timer.h"
#include "mcu.h"

#define MICROSECS_PER_HZ (uint32_t)(1000000ULL / HZ)

static uint64_t systemTimerTicks = 0;

void systemTimerInit(void)
{
    T1CONbits.TON = 0;          // Disable timer
    T1CONbits.TCS = 0;          // Set clock source to FCY
    T1CONbits.TGATE = 0;        // No gate accumulation
    T1CONbits.TSIDL = 0;        // Don't stop in IDLE
    T1CONbits.TCKPS = 0b00;     // 1:1 prescaler
    TMR1 = 0;                   // Reset clock register
    PR1 = FCY / HZ;             // Set timer period

    IFS0bits.T1IF = 0;          // Clear timer interrupt flag
    IEC0bits.T1IE = 1;          // Enable the timer interrupt

    T1CONbits.TON = 1;          // Start timer
}

uint64_t systemTimerGetCurrent(void)
{
    IEC0bits.T1IE = 0;
    uint64_t tmp = systemTimerTicks;
    IEC0bits.T1IE = 1;
    return tmp;
}

void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    ++systemTimerTicks;
}