#include "keyboard.h"
#include "stm32f407xx.h"
#include "stm32f4xx_hal.h"
#include "logger.h"
#include "input.h"

/*
 * SCAN PORTA 0 ... 5  6 bits
 * SCAN PORTE 0 ... 6  7 bits
 * 
 * SIGN PORTC 0 ... 7  8 bits
*/
#define KEYBOARD_ROWS       (8)
#define KEYBOARD_COLUMNS    (13)
#define KEYB_SIGN_Port      (GPIOC)
#define KEYB_SCAN_PortA     (GPIOA)
#define KEYB_SCAN_PortE     (GPIOE)

typedef union
{
    struct
    {
        uint16_t addr :13;
        uint16_t row  :3;
    };
    keys_t key;
} key_info_t;

// void keyboard_init(void)
// {
//     KEYB_SIGN_Port->MODER &= 0xFFFF;
// }
// static uint16_t key_arr[8];

/*
    UP    - 7
    DOWN  - 6
    LEFT  - 5
    RIGHT - 8
    FIRE  - 0
*/
void keyboard_scan(void)
{
    for (uint8_t row = 0; row < KEYBOARD_ROWS; ++row)
    {
        KEYB_SIGN_Port->BSRR = 0xFF;
        KEYB_SIGN_Port->BSRR = (1 << row) << 16;
        uint32_t delay = 10;
        while (delay--)
            asm("nop");
        // HAL_Delay(1);
        key_info_t data;
        data.addr = (KEYB_SCAN_PortE->IDR & 0x7F) << 6;
        data.addr |= (KEYB_SCAN_PortA->IDR & 0x3F);

        if (data.addr != 0x1FFF)
        {
            data.row = row;
            // logger("%s row %02X key %04X, int %04X key %04X\r\n", __func__,
            // data.row, data.addr, data.key, (0x1FFF - data.addr));
            input_key_set(data.key);
        }
    }
}