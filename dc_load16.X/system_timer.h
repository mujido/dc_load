#pragma once

#include "int_types.h"
#include "mcu.h"

void systemTimerInit(void);

uint64_t systemTimerGetCurrent(void);

#define systemTimerMillisecondsToTicks(ms) ((uint64_t)(ms) * HZ / 1000)
