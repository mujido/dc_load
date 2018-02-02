#ifndef CIRCULAR_BUFFER_H
#define	CIRCULAR_BUFFER_H

#include "IntTypes.h"

typedef struct {
    uint8_t readPos_;
    uint8_t writePos_;
    unsigned capacity_ : 7;
    unsigned empty_ : 1;
    char buffer_[1];
} CircularBuffer;

#define MAKE_CIRCULAR_BUFFER(name, size) \
    struct CircularBuffer_##name##_holder { \
        union { \
            CircularBuffer circBuf_; \
            uint8_t bufferSpace_[sizeof(CircularBuffer) + (size) - 1]; \
        } v_; \
    }; \
    struct CircularBuffer_##name##_holder name##_memory = {\
        { 0, 0, (size), 1 } \
    }; \
    CircularBuffer* name = &name##_memory.v_.circBuf_

#define CIRCULAR_BUFFER_EMPTY -1

#pragma warning push
#pragma warning disable 520     // Disable the function not used warning

static inline __reentrant uint8_t circularBufferIsEmpty(CircularBuffer* circBuf)
{
    return circBuf->empty_;
}

static inline uint8_t circularBufferIsFull(CircularBuffer* circBuf)
{
    return (uint8_t)(!circBuf->empty_ && circBuf->readPos_ == circBuf->writePos_);
}

__reentrant uint8_t circularBufferGetLength(CircularBuffer* circBuf);

__reentrant uint8_t circularBufferWriteByte(CircularBuffer* circBuf, uint8_t byte);

__reentrant uint8_t circularBufferWrite(CircularBuffer* circBuf, const void* buffer, uint8_t size);

__reentrant int16_t circularBufferReadByte(CircularBuffer* circBuf);

__reentrant uint8_t circularBufferRead(CircularBuffer* circBuf, const void* buffer, uint8_t maxSize);

#pragma warning pop

#endif

