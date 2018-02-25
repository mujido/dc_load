#ifndef KS_MCU_H
#define KS_MCU_H

// Instruction cycle frequency 
#define FCY (7370000ULL / 2)

// Define number of system clock ticks per second
#define HZ 200

#include <xc.h>

#define __debug_break __builtin_software_breakpoint

#endif