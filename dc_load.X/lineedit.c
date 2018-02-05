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

static uint8_t lineEditAppend(const uint8_t* bytes, uint8_t length)
{
    uint8_t copySize = (uint8_t)(LINE_EDIT_CAPACITY - lineContext.length_);
    if (copySize > length)
        copySize = length;

    switch (copySize)
    {
    case 0:
        break;

    case 2:
        *(unsigned*)(lineContext.lineBuf_ + lineContext.length_) = *((unsigned*)bytes);
        lineContext.length_ += 2;
        break;

    case 1:
        lineContext.lineBuf_[lineContext.length_] = *bytes;
        ++lineContext.length_;
        break;

    default:
        for (uint8_t i = 0; i < copySize; ++i)
        {
            uint8_t ch = bytes[i];
            lineContext.lineBuf_[lineContext.length_] = ch;
            ++lineContext.length_;
        }
        break;
    }

    return copySize;
}

uint8_t lineEditReadSerial(void)
{
    if (*(uint8_t*)0x20 == 0x61U)
        __debug_break();

    uint8_t serialBuf[8];
    uint8_t readSize = 0;
    if (!*(uint8_t*)0x2700)
        readSize = serial1Read(serialBuf, sizeof(serialBuf));
#if 0
    di();
    asm("movlw 246");
    asm("addwf 6, w");
    asm("movwf 126");
    asm("movlw 255");
    asm("addwfc 7,w");
    asm("movwf 127");
    asm("movf 126,w");
    ei();
    asm("movlb 0");
    asm("movwf serial1Read@buffer");
    asm("movf 127,w");
    asm("movwf serial1Read@buffer+1");
    asm("movlw 8");
    asm("movwf serial1Read@maxSize");
    asm("movlw 0x61");
    asm("xorwf 0x20, w");
    asm("btfsc STATUS, 2");
    asm("trap");
//    asm("movlw 0x7d")
//    asm("xorwf serial1Read@buffer, w");
//    asm("btfsc STATUS, 2");
//    asm("goto callit");
//    asm("movlw 0x20");
//    asm("xorwf serial1Read@buffer+1, w");
//    asm("btfsc STATUS, 2");
//    asm("trap");
//    asm("goto callit");
    if (*(unsigned*)0x004c != 0x207e)
        __debug_break();
    if (*(unsigned*)0x004c == 0x017e)
        __debug_break();

    asm("callit:");
    asm("fcall _serial1Read");
    asm("movwi [-1]fsr1");
#endif
    uint8_t state = LINE_EDIT_OK;

    if (*(uint8_t*)0x20 == 0x61U)
        __debug_break();
    for (uint8_t pos = 0; pos < readSize; ++pos)
    {
        uint8_t ch = serialBuf[pos];
        uint8_t writeSize;

        switch (ch)
        {
        case BACKSPACE:
        case DEL:
            if (lineContext.length_ > 0)
            {
                --lineContext.length_;
                serial1SendByteBlocking(ch);
                if (*(uint8_t*)0x20 == 0x61U)
                    __debug_break();
            }
            break;

        case ESC:
            writeSize = lineEditAppend(escapeReplaceBytes, sizeof(escapeReplaceBytes));
            if (*(uint8_t*)0x20 == 0x61U)
                __debug_break();
            serial1SendBlocking(escapeReplaceBytes, writeSize);
            if (*(uint8_t*)0x20 == 0x61U)
                __debug_break();
            break;

        case '\n':
            // ignore
            break;

        case '\r':
            serial1SendBlocking(crLf, sizeof(crLf));
            state = LINE_EDIT_EOL;
            goto end;

        default:
//            if ((ch > 127 || ch < 20) && ch != '\r' && ch != '\n')
//                __debug_break();
            writeSize = lineEditAppend(&ch, 1);
            if (*(uint8_t*)0x20 == 0x61U)
                __debug_break();
            if (writeSize)
                serial1SendByteBlocking(ch);
            break;
        }
    }

end:
    if (*(uint8_t*)0x20 == 0x61U)
        __debug_break();
    return state;
}

void lineEditClear()
{
    lineContext.length_ = 0;
}