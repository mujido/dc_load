#ifndef KS_MCU_H
#define KS_MCU_H

#if defined(__dsPIC33FJ64GP202__)
    // Instruction cycle frequency 
#   define FCY (8000000ULL / 2)
#elif defined(__PIC24FJ128GB202__)
    // Instruction cycle frequency 
#   define FCY (8000000ULL / 2)
#else
#  error "Invalid platform"
#endif

// Define number of system clock ticks per second
#define HZ 128

#include <xc.h>

#define __debug_break __builtin_software_breakpoint

#endif