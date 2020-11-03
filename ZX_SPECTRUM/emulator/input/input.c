#include "input.h"
#include <string.h>

#define KEYS_ROWS       (5)
#define KEYS_COLOMNS    (8)

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
*/

#include "stm32f1xx_hal.h"
#include "logger.h"
static uint8_t uart_rx_buff[10];
extern UART_HandleTypeDef huart1;
void input_key_init(void)
{
    memset(keys, 0xFF, sizeof(keys));
    // HAL_UART_Receive_IT(&huart1, &uart_rx_buff, 1);
    HAL_UART_Receive_DMA(&huart1, uart_rx_buff, 10);
}
void parse(void);
uint8_t input_key_get(uint8_t addr)
{
    parse();
    // TODO: improve it
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
        default:
            {
                logger("%s %02X\r\n", __func__, addr);
                break;
            }
    }
    // logger("%s %02X key %02X\r\n", __func__, addr, key);
    return key;
}

void input_key_set(input_keys_t key, bool preset)
{
    uint8_t index = key / KEYS_ROWS;
    if (index > KEYS_COLOMNS)
    {
        return;
    }

    if (preset)
    {
        keys[index] = ~(1 << (key - (index * KEYS_ROWS)));
    }
    else
    {
        keys[index] = 0xFF;
    }
}


//TEST keyboard

#include "logger.h"
extern bool load;
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
void parse()
{
    for (uint8_t i = 0; i < 10; ++i)
    {
        if (uart_rx_buff[i] == 0x00)
        {
            continue;
        }
        switch (uart_rx_buff[i])
        {
            case '0': input_key_set(ZX_KEY_0, true); break;
            case '1': input_key_set(ZX_KEY_1, true); break;
            case '2': input_key_set(ZX_KEY_2, true); break;
            case '3': input_key_set(ZX_KEY_3, true); break;
            case '4': input_key_set(ZX_KEY_4, true); break;
            case '5': input_key_set(ZX_KEY_5, true); break;
            case '6': input_key_set(ZX_KEY_6, true); break;
            case '7': input_key_set(ZX_KEY_7, true); break;
            case '8': input_key_set(ZX_KEY_8, true); break;
            case '9': input_key_set(ZX_KEY_9, true); break;
            case 'a': input_key_set(ZX_KEY_A, true); break;
            case 'b': input_key_set(ZX_KEY_B, true); break;
            case 'w': input_key_set(ZX_KEY_W, true); break;

            case 'm': input_key_set(ZX_KEY_M, true); break;
            case 'n': input_key_set(ZX_KEY_N, true); break;
            case 'l': input_key_set(ZX_KEY_L, true); break;
            case 'k': input_key_set(ZX_KEY_K, true); break;
            case 'j': input_key_set(ZX_KEY_J, true); break;
            case 'h': input_key_set(ZX_KEY_H, true); break;
            case 'p': input_key_set(ZX_KEY_P, true); break;
            case 'o': input_key_set(ZX_KEY_O, true); break;
            case 'i': input_key_set(ZX_KEY_I, true); break;
            case 'u': input_key_set(ZX_KEY_U, true); break;
            case 'y': input_key_set(ZX_KEY_Y, true); break;
            case 'q': input_key_set(ZX_KEY_Q, true); break;
            case 'e': input_key_set(ZX_KEY_E, true); break;
            case 'r': input_key_set(ZX_KEY_R, true); break;
            case 't': input_key_set(ZX_KEY_T, true); break;
            case 's': input_key_set(ZX_KEY_S, true); break;
            case 'd': input_key_set(ZX_KEY_D, true); break;
            case 'f': input_key_set(ZX_KEY_F, true); break;
            case 'g': input_key_set(ZX_KEY_G, true); break;
            case 'z': input_key_set(ZX_KEY_Z, true); break;
            case 'x': input_key_set(ZX_KEY_X, true); break;
            case 'c': input_key_set(ZX_KEY_C, true); break;
            case 'v': input_key_set(ZX_KEY_V, true); break;

            case 0x20: input_key_set(ZX_KEY_SPACE, true); break;
            case 0x7F: input_key_set(ZX_KEY_SYM, true); break;
            case 0x0D: input_key_set(ZX_KEY_ENTER, true); break;
            case 0x60: input_key_set(ZX_KEY_SHIFT, true); break;
            
            case 0x5C: /* '\' */
            {
                load = true;
                break;
            }
            default:
                // logger("UNKNOWN RX %02X\r\n", uart_rx_buff);
                break;
        }
        uart_rx_buff[i] = 0xFF;
    }
    HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
    // HAL_UART_Receive_IT(&huart1, &uart_rx_buff, 1);
}