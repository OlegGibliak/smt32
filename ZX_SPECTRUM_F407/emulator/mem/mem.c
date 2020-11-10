#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "mem.h"
#include "logger.h"
#ifdef SPECTRUM_ROM 
#include "rom.h"
#elif SPECTRUM_TEST_ROM
#include "zx_test_rom.h"
#elif SPECTRUM_128_ROM
#include "128.h"
#else
#error rom file not selected
#endif
#include "screen.h"
#include "display.h"
#include "input.h"
// #define MEM_DBG

static uint8_t ram[0x10000-0x4000];

uint8_t mem_readbyte(uint16_t addr)
{
    uint8_t ret_data;

    if (addr >= 0x5B00)
    {
        ret_data = ram[addr - 0x5B00];
    }
    else if (addr >= 0x4000)
	{
        ret_data = screen_mem_read(addr);
	}
    else
    {
        ret_data = ROM[addr];
    }
#ifdef MEM_DBG
    logger("%s A: %04X D: %02X\r\n", __func__, addr, ret_data);
#endif /* MEM_DBG */
    return ret_data;
}

uint16_t mem_readword(uint16_t addr)
{
    uint16_t ret_data;
    ret_data = (mem_readbyte(addr + 1) << 8) | mem_readbyte(addr);
#ifdef MEM_DBG
    logger("%s A: %04X D: %04X\r\n", __func__, addr, ret_data);
#endif /* MEM_DBG */
    
    return ret_data;
}

void mem_writebyte(uint16_t addr, uint8_t b)
{
#ifdef MEM_DBG
    logger("%s A: %04X D: %02X\r\n", __func__, addr, b);
#endif /* MEM_DBG */

    if (addr >= 0x5B00)
    {
        ram[addr - 0x5B00] = b;
    }
    else if (addr >= 0x4000)
	{
        screen_mem_write(addr, b);
	}
    else
    {
        // logger("b ERROR write rom addr %04X\r\n", addr);
    }
}

void mem_writeword(uint16_t addr, uint16_t w)
{
#ifdef MEM_DBG
    logger("%s A: %04X D: %04X\r\n", __func__, addr, w);
#endif /* MEM_DBG */
    mem_writebyte(addr, (uint8_t)w);
    mem_writebyte(addr + 1, w >> 8);
}

uint8_t mem_input(uint8_t portLow, uint8_t portHigh)
{
    uint8_t ret = 0xFF;
    switch (portLow)
    {
        case 0xFE:
        {
            /* keyboar, border, type, beeper */
            ret = input_key_get(portHigh);
            break;
        }
        case 0xFF:
        {
            /* display attr */
            logger("%s display attr %02X %02X\r\n", __func__, portHigh, portLow);
            break;
        }
        case 0x1F:
        {
            /* joystick */
            logger("%s joystick %02X %02X\r\n", __func__, portHigh, portLow);
            break;
        }
        default:
            logger("%s unk %02X %02X\r\n", __func__, portHigh, portLow);
            break;
    }
#ifdef MEM_DBG
    logger("%s portHigh %02X portLow %02X d: %02X\r\n", __func__, portHigh,  portLow, ret);
#endif /* MEM_DBG */
    return ret;
}
#include "main.h"
void mem_output(uint8_t portLow, uint8_t portHigh, uint8_t data)
{
#ifdef MEM_DBG
    logger("%s portHigh %02X  portLow %02X data %02X\r\n", __func__, portHigh, portLow, data);
#endif /* MEM_DBG */

    switch (portLow)
    {
        case 0xFE: /*Keyboar, border, type, beeper */
        {
            // (data & (1 << 3)) /* Type */
            // (data & (1 << 4)) /* Beeper */

            /* border color (no bright colors) */
            screen_draw_border(data & 0x07);
            // logger("type %d\r\n", data & (1 << 3));
            if (data & (1 << 4))
            {
                // BEEPER_GPIO_Port->BSRR = (1 << BEEPER_Pin) << 16;
                // HAL_GPIO_WritePin(BEEPER_GPIO_Port, BEEPER_Pin, GPIO_PIN_RESET);
            }
            else
            {
                // logger("beeper\r\n");
                // BEEPER_GPIO_Port->BSRR = (1 << BEEPER_Pin);
                // HAL_GPIO_WritePin(BEEPER_GPIO_Port, BEEPER_Pin, GPIO_PIN_SET);
            }
        }
        break;
        default:
            // logger("%s unk %02X %02X D:%02X\r\n", __func__, portHigh, portLow, data);
            break;
    }
}