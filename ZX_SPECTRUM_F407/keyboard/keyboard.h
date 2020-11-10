#ifndef KEYBOARD_H__
#define KEYBOARD_H__

#include <stdint.h>

typedef enum
{
    KEY_ESC = 0x1FFE,
    KEY_1   = 0x1FBF,
    KEY_2   = 0x1FEF,
    KEY_3   = 0xBDFF,
    KEY_4   = 0x5FEF,
    KEY_5   = 0x5FFB,
    KEY_6   = 0x1FFB,
    KEY_7   = 0x7DFF,
    KEY_8   = 0x9FFB,
    KEY_9   = 0x9F7F,
    KEY_0   = 0x3F7F,
    KEY__   = 0x3FEF,
    KEY_DEL = 0x3FFE,

    KEY_TAB = 0xFFEF,
    KEY_Q   = 0xDFBF,
    KEY_W   = 0xDEFF,
    KEY_E   = 0xBEFF,
    KEY_R   = 0x5FBF,
    KEY_T   = 0x5F7F,
    KEY_Y   = 0xFFFB,
    KEY_U   = 0x7EFF,
    KEY_I   = 0x9FEF,
    KEY_O   = 0x9EFF,
    KEY_P   = 0x3EFF,
    KEY_EQUAL = 0x1FFB,
    KEY_ENT = 0xFFFE,

    KEY_TIL = 0x5DFF,
    KEY_A   = 0xDFEF,
    KEY_S   = 0xDF7F,
    KEY_D   = 0xBF7F,
    KEY_F   = 0xBFBF,
    KEY_G   = 0x5EFF,
    KEY_H   = 0x1DFF,
    KEY_J   = 0x7F7F,
    KEY_K   = 0x9FBF,
    KEY_L   = 0x9DFF,
    KEY_SEMI   = 0x3DFF,
    KEY_QUOTES = 0x7FEF,

    KEY_LSHIFT  = 0xF7FF,
    KEY_Z   = 0xDFFB,
    KEY_X   = 0xDDFF,
    KEY_C   = 0xBFFB,
    KEY_V   = 0xBFEF,
    KEY_B   = 0x5DFF,
    KEY_N   = 0xFDFF,
    KEY_M   = 0x7FFB,
    KEY_OPEN_SQ  = 0x7FBF,
    KEY_CLOSE_SQ = 0x1F7F,
    KEY_UP  = 0x1EFF,
    KEY_RSHIFT   = 0x3FFD,

    KEY_CTRL  = 0x8FFF,
    KEY_ALT   = 0xBFFE,
    KEY_FN    = 0x7FDF,
    KEY_SPACE = 0x5FFE,
    KEY_QUE   = 0xBFF7,
    KEY_LEFT  = 0xFF7F,
    KEY_DOWN  = 0xFEFF,
    KEY_RIGHT = 0xFFBF
} keys_t;

void keyboard_scan(void);

#endif /* KEYBOARD_H__ */