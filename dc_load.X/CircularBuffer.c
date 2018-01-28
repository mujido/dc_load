#include "CircularBuffer.h"
#include <stdlib.h>

__reentrant uint8_t circularBufferGetLength(CircularBuffer* circBuf)
{
    if (circBuf->empty_)
        return 0;

    int8_t diff = circBuf->writePos_ - circBuf->readPos_;
    return (uint8_t)((diff > 0) ? diff : circBuf->capacity_ + diff);
}

__reentrant uint8_t circularBufferWriteByte(CircularBuffer* circBuf, uint8_t byte)
{
    uint8_t length = circularBufferGetLength(circBuf);

    if (length >= circBuf->capacity_)
        return 0;

    circBuf->buffer_[circBuf->writePos_] = byte;

    ++circBuf->writePos_;
    if (circBuf->writePos_ >= circBuf->capacity_)
        circBuf->writePos_ = 0;

    circBuf->empty_ = 0;

    return 1;
}

__reentrant int16_t circularBufferReadByte(CircularBuffer* circBuf)
{
    uint8_t length = circularBufferGetLength(circBuf);

    if (length == 0)
        return CIRCULAR_BUFFER_EMPTY;

    uint8_t ch = circBuf->buffer_[circBuf->readPos_];

    ++circBuf->readPos_;
    if (circBuf->readPos_ >= circBuf->capacity_)
        circBuf->readPos_ = 0;

    circBuf->empty_ = (uint8_t)(length <= 1);

    return ch;
}
