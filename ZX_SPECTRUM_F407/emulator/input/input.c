#include "input.h"
#include <string.h>
#include "logger.h"

#define KEYS_ROWS       (5)
#define KEYS_COLOMNS    (8)

uint8_t load = 0;
static uint8_t keys[KEYS_COLOMNS];
/*
    D0  D1  D2  D3  D4
#7F:SP  SS  M   N   B
#BF:EN  L   K   J   H
#DF:P   O   I   U   Y
#EF:0   9   8   7   6
#F7:1   2   3   4   5
#FB:Q   W   E   R   T
#FD:A   S   D   F   G
#FE:CS  Z   X   C   V

SP - SPACE
SS - SYMBOL SHIFT
EN - ENTER
CS - CAPS SHIFT
*/

void input_key_init(void)
{
    memset(keys, 0xFF, sizeof(keys));
}
#include "stm32f4xx_hal.h"
uint8_t input_key_get(uint8_t addr)
{
    keyboard_scan();
    
    uint8_t key = 0xFF;
    switch (addr)
    {
        case 0x7F: key = keys[0]; keys[0] = 0xFF; break;
        case 0xBF: key = keys[1]; keys[1] = 0xFF; break;
        case 0xDF: key = keys[2]; keys[2] = 0xFF; break;
        case 0xEF: key = keys[3]; keys[3] = 0xFF; break;
        case 0xF7: key = keys[4]; keys[4] = 0xFF; break;
        case 0xFB: key = keys[5]; keys[5] = 0xFF; break;
        case 0xFD: key = keys[6]; keys[6] = 0xFF; break;
        case 0xFE: key = keys[7]; keys[7] = 0xFF; break;

        case 0xE7: key = 0x00; break;
        case 0x00:
			{
				key =  keys[7];
				key &= keys[6];
				key &= keys[5];
				key &= keys[4];
				key &= keys[3];
				key &= keys[2];
				key &= keys[1];
				key &= keys[0];
                memset(keys, 0xFF, sizeof(keys));
                break;
			}
        default:
            {
                // logger("%s %02X\r\n", __func__, addr);
                break;
            }
    }
    // if (key != 0xFF)
    //     HAL_Delay(20);
    return key;
}

void input_key_set(keys_t key)
{
    zx_keys_t zx_key;
    switch (key)
    {
        case KEY_1: zx_key = ZX_KEY_1; break;
        case KEY_2: zx_key = ZX_KEY_2; break;
        case KEY_3: zx_key = ZX_KEY_3; break;
        case KEY_4: zx_key = ZX_KEY_4; break;
        case KEY_5: zx_key = ZX_KEY_5; break;
        case KEY_6: zx_key = ZX_KEY_6; break;
        case KEY_7: zx_key = ZX_KEY_7; break;
        case KEY_8: zx_key = ZX_KEY_8; break;
        case KEY_9: zx_key = ZX_KEY_9; break;
        case KEY_0: zx_key = ZX_KEY_0; break;

        case KEY_Q: zx_key = ZX_KEY_Q; break;
        case KEY_W: zx_key = ZX_KEY_W; break;
        case KEY_E: zx_key = ZX_KEY_E; break;
        case KEY_R: zx_key = ZX_KEY_R; break;
        case KEY_T: zx_key = ZX_KEY_T; break;
        case KEY_Y: zx_key = ZX_KEY_Y; break;
        case KEY_U: zx_key = ZX_KEY_U; break;
        case KEY_I: zx_key = ZX_KEY_I; break;
        case KEY_O: zx_key = ZX_KEY_O; break;
        case KEY_P: zx_key = ZX_KEY_P; break;
        case KEY_ENT: zx_key = ZX_KEY_ENTER; break;

        case KEY_A: zx_key = ZX_KEY_A; break;
        case KEY_S: zx_key = ZX_KEY_S; break;
        case KEY_D: zx_key = ZX_KEY_D; break;
        case KEY_F: zx_key = ZX_KEY_F; break;
        case KEY_G: zx_key = ZX_KEY_G; break;
        case KEY_H: zx_key = ZX_KEY_H; break;
        case KEY_J: zx_key = ZX_KEY_J; break;
        case KEY_K: zx_key = ZX_KEY_K; break;
        case KEY_L: zx_key = ZX_KEY_L; break;

        case KEY_LSHIFT: zx_key = ZX_KEY_SHIFT; break;
        case KEY_Z: zx_key = ZX_KEY_Z; break;
        case KEY_X: zx_key = ZX_KEY_X; break;
        case KEY_C: zx_key = ZX_KEY_C; break;
        case KEY_V: zx_key = ZX_KEY_V; break;
        case KEY_B: zx_key = ZX_KEY_B; break;
        case KEY_N: zx_key = ZX_KEY_N; break;
        case KEY_M: zx_key = ZX_KEY_M; break;

        case KEY_CTRL:  zx_key = ZX_KEY_SYM; break;
        case KEY_SPACE: zx_key = ZX_KEY_SPACE; break;

        case KEY_UP:    zx_key = ZX_KEY_7; break;
        case KEY_DOWN:  zx_key = ZX_KEY_6; break;
        case KEY_LEFT:  zx_key = ZX_KEY_5; break;
        case KEY_RIGHT: zx_key = ZX_KEY_8; break;

        case KEY_ALT: load = 1; return;
        case KEY_FN:  load = 2; return;
        case KEY_QUE: load = 3; return;
        case KEY_OPEN_SQ: load = 4; return;
        case KEY_CLOSE_SQ: load = 5; return;
    default:
        return;
    }

    uint8_t index = zx_key / KEYS_ROWS;
    if (index > KEYS_COLOMNS)
    {
        return;
    }

    keys[index] = ~(1 << (zx_key - (index * KEYS_ROWS)));
}