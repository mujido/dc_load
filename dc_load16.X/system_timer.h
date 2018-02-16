#pragma once

#include "int_types.h"
#include "mcu.h"

typedef uint32_t Tick;

void systemTimerInit(void);

Tick systemTimerGetCurrent(void);

#define systemTimerMillisecondsToTicks(ms) ((Tick)(ms) * HZ / 1000)
