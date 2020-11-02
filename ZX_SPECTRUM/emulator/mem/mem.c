#include <stdbool.h>
#include "mem.h"
#include "logger.h"
#ifdef SPECTRUM_ROM 
#include "rom.h"
#elif SPECTRUM_TEST_ROM
#include "zx_test_rom.h"
#elif SPECTRUM_TEST_DEMO
#include "zx_test_demo.h"
#elif SPECTRUM_TEST_DEMO2
#include "demo2.h"
#else
#error rom file not selected
#endif
#include "screen.h"
#include "ili9341.h"
#include <string.h>
#include "input.h"
// #define MEM_DBG

#define MEM_BLOCK_W_PIXELS  (7)
#define MEM_BLOCK_H_PIXELS  (3)

#define MEM_BLOCK_SIZE      (64)
#define MEM_BLOCK_NUMBER    ((0x10000 - 0x5B00) / MEM_BLOCK_SIZE)

typedef enum
{
    MEM_BLOCK_IDLE,
    MEM_BLOCK_MODIFIED,
    MEM_BLOCK_READED,
    MEM_BLOCK_UNKNOWN,
} mem_block_state_t;

typedef struct
{
    uint16_t          x;
    uint16_t          y;
} mem_block_t;

typedef struct
{
    mem_block_t       block[MEM_BLOCK_NUMBER];
    uint8_t           data[MEM_BLOCK_SIZE];
    uint16_t          block_id;
    mem_block_state_t state;
} mem_cache_t;


// extern const uint8_t __48_rom[SPECTRUM_ROM_SIZE];
static mem_cache_t m_cache = {.state = MEM_BLOCK_IDLE};
extern uint8_t indata[128];

static void mem_test(void)
{
    for(uint16_t i = 0; i < MEM_BLOCK_NUMBER; ++i)
	{
        for(uint16_t k = 0; k < MEM_BLOCK_SIZE; ++k)
        {
            m_cache.data[k] = ((i+k)^0x55)&0xff;
        }
        
        ili9341_64block_mem_write(m_cache.block[i].x, m_cache.block[i].y, m_cache.data);
    }
// screen_draw();
    for(uint16_t i = 0; i < MEM_BLOCK_NUMBER; ++i)
    {
        ili9341_64block_mem_read(m_cache.block[i].x, m_cache.block[i].y, m_cache.data);

        for(uint16_t k = 0; k < MEM_BLOCK_SIZE; ++k)
        {
            uint8_t btt = ((i + k) ^ 0x55) & 0xFF;
            if(m_cache.data[k] != btt)
            {
                logger("Memmory test error block %04X\r\n", i);
                return;
            }
        }
    }
}

static void mem_block_init(void)
{
    uint16_t x = 0;
    uint16_t y = 0;

    uint16_t i;
    for (i = 0; i < 200; ++i)
    {
        if (x > 320 - 8)
        {
            x = 0;
            y += 4;
        }
        m_cache.block[i].x = x;
        m_cache.block[i].y = y;
        // logger("0 mem rectangle block %04X x: %d y: %d\r\n", i, x, y);
        x += 8;
    }

    for (uint16_t a = 0; a < 150; ++a, ++i)
    {
        if (x > 24 - 8)
        {
            x = 0;
            y += 4;
        }
        m_cache.block[i].x = x;
        m_cache.block[i].y = y;
        // logger("1 mem rectangle block %04X x: %d y: %d\r\n", i, x, y);
        x += 8;
    }

    x = 0;
    y += 4;

    for (uint16_t a = 0; a < 200; ++a, ++i)
    {
        if (i > MEM_BLOCK_NUMBER)
        {
            break;
        }
        if (x > 320 - 8)
        {
            x = 0;
            y += 4;
        }
        m_cache.block[i].x = x;
        m_cache.block[i].y = y;
        // logger("2 mem rectangle block %04X x: %d y: %d\r\n", i, x, y);
        x += 8;
    }

    x = 296;
    y = 20;
    for (uint16_t a = 0; a < 150; ++a, ++i)
    {
        if (i > MEM_BLOCK_NUMBER)
        {
            break;
        }
        if (x > 320 - 8)
        {
            x = 296;
            y += 4;
        }
        m_cache.block[i].x = x;
        m_cache.block[i].y = y;
        // logger("3 mem rectangle block %04X x: %d y: %d\r\n", i, x, y);
        x += 8;
    }
    // 660 blocks
    logger("initialized %d blocks\r\n", i);
}

static inline uint8_t mem_ram_read8(uint16_t block, uint8_t addr)
{
    if (m_cache.block_id != block)
    {
        if (m_cache.state == MEM_BLOCK_MODIFIED)
        {
            ili9341_64block_mem_write(m_cache.block[m_cache.block_id].x,
                                    m_cache.block[m_cache.block_id].y,
                                    m_cache.data);
        }

        ili9341_64block_mem_read(m_cache.block[block].x,
                                 m_cache.block[block].y,
                                 m_cache.data);
        m_cache.block_id = block;
        m_cache.state    = MEM_BLOCK_READED;
    }

    return m_cache.data[addr];
}

static inline void mem_ram_write8(uint16_t block, uint8_t addr, uint8_t data)
{
    if (m_cache.block_id != block)
    {
        if (m_cache.state == MEM_BLOCK_MODIFIED)
        {
            ili9341_64block_mem_write(m_cache.block[m_cache.block_id].x,
                                    m_cache.block[m_cache.block_id].y,
                                    m_cache.data);
        }

        ili9341_64block_mem_read(m_cache.block[block].x,
                                 m_cache.block[block].y,
                                 m_cache.data);
        m_cache.block_id = block;
        m_cache.state    = MEM_BLOCK_READED;
    }

    m_cache.data[addr] = data;
    m_cache.state      = MEM_BLOCK_MODIFIED;
}

void mem_init(void)
{
    mem_block_init();
    // mem_test();
}

uint8_t  inline mem_readbyte(uint16_t addr)
{
#ifdef MEM_DBG
    logger("%s A: %04X D: %02X\r\n", __func__, addr, __48_rom[addr]);
#endif /* MEM_DBG */

    uint8_t ret_data;

    if (addr >= 0x5B00)
    {
        uint16_t block = (addr - 0x5B00) >> 6;
        ret_data = mem_ram_read8(block, addr & 0x3F);
    }
    else if (addr >= 0x4000)
	{
        ret_data = screen_mem_read(addr);
	}
    else
    {
        ret_data = __48_rom[addr];
    }

    return ret_data;
}

uint16_t  inline mem_readword(uint16_t addr)
{
    uint16_t ret_data;

    ret_data = ((mem_readbyte(addr + 1) << 8) | mem_readbyte(addr));
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
        uint16_t block = (addr - 0x5B00) >> 6;
        mem_ram_write8(block, addr & 0x3F, b);
    }
    else if (addr >= 0x4000)
	{
        screen_mem_write(addr, b);
	}
    else
    {
        logger("b ERROR write rom addr %04X\r\n", addr);
    }
}

void mem_writeword(uint16_t addr, uint16_t w)
{
#ifdef MEM_DBG
    logger("%s A: %04X D: %04X\r\n", __func__, addr, w);
#endif /* MEM_DBG */

    mem_writebyte(addr, (uint8_t)w);
    mem_writebyte(addr + 1, (uint8_t)(w >> 8));
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
// #ifdef MEM_DBG
    logger("%s portHigh %02X portLow %02X d: %02X\r\n", __func__, portHigh,  portLow, ret);
// #endif /* MEM_DBG */
    return ret;
}

void mem_output(uint8_t portLow, uint8_t portHigh, uint8_t data)
{
// #ifdef MEM_DBG
    logger("%s portHigh %02X  portLow %02X data %02X\r\n", __func__, portHigh, portLow, data);
// #endif /* MEM_DBG */

    switch (portLow)
    {
        case 0xFE: /*Keyboar, border, type, beeper */
        {
            // (data & (1 << 3)) /* Type */
            // (data & (1 << 4)) /* Beeper */

            /* border color (no bright colors) */
            screen_draw_border(data & 0x07);
            // logger("type %d\r\n", data & (1 << 3));
        }
        break;
        default:
            logger("%s unk %02X %02X D:%02X\r\n", __func__, portHigh, portLow, data);
            break;
    }
}