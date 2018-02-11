#ifndef KS_LINEEDIT_H
#define KS_LINEEDIT_H

#include "int_types.h"

#define LINE_EDIT_CAPACITY 80

typedef struct LineEdit_
{
    uint8_t lineBuf_[LINE_EDIT_CAPACITY];
    uint8_t length_;
} LineEdit;

typedef enum {
    LINE_EDIT_OK,
    LINE_EDIT_EOL
} LineEditStatus;

extern LineEdit lineContext;

LineEditStatus lineEditReadSerial(void);

void lineEditClear(void);

#endif