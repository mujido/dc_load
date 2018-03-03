#include "lineedit.h"
#include "serial.h"
#include <string.h>

#define BACKSPACE (uint8_t)0x08
#define DEL (uint8_t)0x7f
#define ESC (uint8_t)0x1b

LineEdit lineContext = {
    "", 0
};

static const uint8_t escapeReplaceBytes[2] = { '^', '[' };
static const uint8_t crLf[2] = { '\r', '\n' };

static uint16_t lineEditAppend(const uint8_t* bytes, uint16_t length)
{
    uint16_t copySize = LINE_EDIT_CAPACITY - lineContext.length_;
    if (copySize > length)
        copySize = length;

    switch (copySize)
    {
    case 0:
        break;

    case 2:
        *(uint16_t*)(lineContext.lineBuf_ + lineContext.length_) = *((uint16_t*)bytes);
        lineContext.length_ += 2;
        break;

    case 1:
        lineContext.lineBuf_[lineContext.length_] = *bytes;
        ++lineContext.length_;
        break;

    default:
        memcpy(lineContext.lineBuf_ + lineContext.length_, bytes, copySize);
        break;
    }

    return copySize;
}

LineEditStatus lineEditReadSerial(void)
{
    uint8_t serialBuf[8];
    uint8_t state = LINE_EDIT_OK;
    uint16_t readSize = serial1Read(serialBuf, sizeof(serialBuf));

    for (uint16_t pos = 0; pos < readSize; ++pos)
    {
        uint8_t ch = serialBuf[pos];
        uint16_t writeSize;

        switch (ch)
        {
        case BACKSPACE:
        case DEL:
            if (lineContext.length_ > 0)
            {
                --lineContext.length_;
                serial1SendByteBlocking(ch);
            }
            break;

        case ESC:
            writeSize = lineEditAppend(escapeReplaceBytes, sizeof(escapeReplaceBytes));
            serial1SendBlocking(escapeReplaceBytes, writeSize);
            break;

        case '\n':
            // ignore
            break;

        case '\r':
            serial1SendBlocking("\n", 1);
            state = LINE_EDIT_EOL;
            goto end;

        default:
            writeSize = lineEditAppend(&ch, 1);
            if (writeSize)
                serial1SendByteBlocking(ch);
            break;
        }
    }

end:
    lineContext.lineBuf_[lineContext.length_] = '\0';
    return state;
}

void lineEditClear()
{
    lineContext.length_ = 0;
}