#ifndef KS_CIRCULAR_BUFFER_H
#define	KS_CIRCULAR_BUFFER_H

#include "int_types.h"

typedef struct {
    uint8_t* readPos_;
    uint8_t* writePos_;
    uint16_t capacity_;
    volatile uint16_t length_;
} CircularBuffer;

#define MAKE_CIRCULAR_BUFFER(name, size) \
    __attribute__((packed)) struct CircularBuffer_##name##_holder { \
        CircularBuffer circBuf_; \
        uint8_t bufferSpace_[(size)]; \
    }; \
    struct CircularBuffer_##name##_holder name##_memory = {\
        { \
            &(name##_memory).bufferSpace_[0], \
            &(name##_memory).bufferSpace_[0], \
            (size), \
            0 \
        } \
    }; \
    CircularBuffer* name = &name##_memory.circBuf_

#define CIRCULAR_BUFFER_EMPTY -1

uint16_t circularBufferWriteByte(CircularBuffer* circBuf, uint8_t byte);

uint16_t circularBufferWrite(CircularBuffer* circBuf, const void* buffer, uint16_t size);

int16_t circularBufferReadByte(CircularBuffer* circBuf);

uint16_t circularBufferRead(CircularBuffer* circBuf, void* buffer, uint16_t maxSize);

#endif

