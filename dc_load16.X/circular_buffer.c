#include "circular_buffer.h"
#include "mcu.h"
#include <stdlib.h>

#define CB_GET_PTR(circBuf) ((uint8_t*)(circBuf) + sizeof(CircularBuffer))
#define CB_GET_END_PTR(circBuf) (CB_GET_PTR(circBuf) + (circBuf)->capacity_)

uint16_t circularBufferWriteByte(CircularBuffer* circBuf, uint8_t byte)
{
    if (circBuf->length_ >= circBuf->capacity_)
        return 0;

    *circBuf->writePos_++ = byte;
    ++circBuf->length_;

    if (circBuf->writePos_ >= CB_GET_END_PTR(circBuf))
        circBuf->writePos_ = CB_GET_PTR(circBuf);

    return 1;
}

extern const char crlf[2];

uint16_t circularBufferWrite(CircularBuffer* circBuf, const void* buffer, uint16_t size)
{
    uint16_t writeSize = circBuf->capacity_ - circBuf->length_;

    if (writeSize == 0)
        return 0;

    writeSize = (writeSize <= size) ? writeSize : size;

    const uint8_t* pch = buffer;
    const uint8_t* bufferEnd = pch + writeSize;
    uint8_t* circBufEnd = CB_GET_END_PTR(circBuf);

    while (pch != bufferEnd)
    {
        *circBuf->writePos_++ = *pch++;

        if (circBuf->writePos_ >= circBufEnd)
            circBuf->writePos_ = CB_GET_PTR(circBuf);
    }

    circBuf->length_ += writeSize;
    return writeSize;
}

int16_t circularBufferReadByte(CircularBuffer* circBuf)
{
    if (circBuf->length_ == 0)
        return CIRCULAR_BUFFER_EMPTY;

    uint8_t ch = *circBuf->readPos_++;
    --circBuf->length_;

    if (circBuf->readPos_ >= CB_GET_END_PTR(circBuf))
        circBuf->readPos_ = CB_GET_PTR(circBuf);

    return ch;
}

uint16_t circularBufferRead(CircularBuffer* circBuf, void* buffer, uint16_t maxSize)
{
    if (circBuf->length_ == 0)
        return 0;

    uint16_t readSize = circBuf->length_ < maxSize ? circBuf->length_ : maxSize;
    uint8_t* pOut = buffer;
    uint8_t* bufferEnd = pOut + readSize;
    uint8_t* circBufEnd = CB_GET_END_PTR(circBuf);

    while (pOut != bufferEnd)
    {
        *pOut++ = *circBuf->readPos_++;

        if (circBuf->readPos_ >= circBufEnd)
            circBuf->readPos_ = CB_GET_PTR(circBuf);
    }

    circBuf->length_ -= readSize;
    return readSize;
}
