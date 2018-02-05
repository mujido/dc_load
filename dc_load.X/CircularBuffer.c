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

__reentrant uint8_t circularBufferWrite(CircularBuffer* circBuf, const void* buffer, uint8_t size)
{
    uint8_t writeSize = (uint8_t)(circBuf->capacity_ - circularBufferGetLength(circBuf));

    if (writeSize == 0)
        return 0;

    writeSize = (uint8_t)((writeSize <= size) ? writeSize : size);

    uint8_t pos = circBuf->writePos_;
    uint8_t capacity = circBuf->capacity_;
    const uint8_t* end = (const uint8_t*)buffer + writeSize;

    for (const uint8_t* pch = buffer; pch != end; ++pch)
    {
        circBuf->buffer_[pos] = *pch;

        ++pos;
        if (pos >= capacity)
            pos = 0;
    }

    circBuf->writePos_ = pos;
    circBuf->empty_ = 0;

    return writeSize;
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

    if (length <= 1)
        circBuf->empty_ = 1;

    return ch;
}

__reentrant uint8_t circularBufferRead(CircularBuffer* circBuf, const void __ram* buffer, uint8_t maxSize)
{
    if (buffer == (void*)0x017e)
        __debug_break();

    uint8_t length = circularBufferGetLength(circBuf);

    if (buffer == (void*)0x017e)
        __debug_break();

    if (length == 0)
        return 0;

    uint8_t empty;
    if (length > maxSize)
    {
        empty = 0;
        length = maxSize;
    }
    else
        empty = 1;

    if (buffer == (void*)0x017e)
        __debug_break();

    uint8_t pos = circBuf->readPos_;
    uint8_t capacity = circBuf->capacity_;

    if (buffer == (void*)0x017e)
        __debug_break();
    if (buffer == (void*)0x2000)
        __debug_break();

    for (uint8_t i = 0; i < length; ++i)
    {
        uint8_t ch = circBuf->buffer_[pos];
        ((uint8_t*)buffer)[i] = ch;

        ++pos;
        if (pos >= capacity)
            pos = 0;
    }

    circBuf->readPos_ = pos;

    if (empty)
        circBuf->empty_ = 1;

//    for (uint8_t i = 0; i < length; ++i)
//    {
//        uint8_t ch = ((uint8_t*)buffer)[i];
//        if ((ch > 127 || ch < 20) && ch != '\r' && ch != '\n')
//           __debug_break();
//    }


    return length;
}