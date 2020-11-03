#include <stdbool.h>
#include "mem.h"
#include "logger.h"
#ifdef SPECTRUM_ROM 
#include "rom.h"
#elif SPECTRUM_TEST_ROM
#include "zx_test_rom.h"
#else
#error rom file not selected
#endif
#include "screen.h"
#include "ili9341.h"
#include <string.h>
#include "input.h"
// #define MEM_DBG

#define MEM_PAGE_NUM_SIZE           (0x2000)
#define MEM_PAGE_NUM_5_SIZE         (0x500)

#define MEM_CACHE_SIZE              (MEM_PAGE_NUM_0_4_SIZE)
#define MEM_ADDR_TO_PAGE(_addr_)    ((0xFFFF - _addr_) / MEM_PAGE_NUM_SIZE)

typedef enum
{
    MEM_PAGE_NUM_0,
    MEM_PAGE_NUM_1,
    MEM_PAGE_NUM_2,
    MEM_PAGE_NUM_3,
    MEM_PAGE_NUM_4,
    // MEM_PAGE_NUM_5,
    MEM_PAGE_NUM
} mem_page_num_t;

typedef enum
{
    MEM_PAGE_STATE_IDLE,
    MEM_PAGE_STATE_MODIFIED,
    MEM_PAGE_STATE_READED
} mem_page_state_t;

typedef struct
{
    const uint16_t   x, w, y, h;
    const uint16_t   size;
} mem_page_t;

typedef struct
{
    mem_page_num_t   num;
    mem_page_state_t state;
    uint8_t          cache[MEM_PAGE_NUM_SIZE];
} memory_t;

static memory_t memory = {.num = MEM_PAGE_NUM, .state = MEM_PAGE_STATE_IDLE};
static const mem_page_t pages[MEM_PAGE_NUM] = { [MEM_PAGE_NUM_0].x = 0,
                                                [MEM_PAGE_NUM_0].w = 320,
                                                [MEM_PAGE_NUM_0].y = 0,
                                                [MEM_PAGE_NUM_0].h = 13,
                                                [MEM_PAGE_NUM_0].size  = MEM_PAGE_NUM_SIZE,
                                                
                                                [MEM_PAGE_NUM_1].x = 0,
                                                [MEM_PAGE_NUM_1].w = 320,
                                                [MEM_PAGE_NUM_1].y = 13,
                                                [MEM_PAGE_NUM_1].h = 13,
                                                [MEM_PAGE_NUM_1].size  = MEM_PAGE_NUM_SIZE,

                                                [MEM_PAGE_NUM_2].x = 0,
                                                [MEM_PAGE_NUM_2].w = 320,
                                                [MEM_PAGE_NUM_2].y = 26,
                                                [MEM_PAGE_NUM_2].h = 13,
                                                [MEM_PAGE_NUM_2].size  = MEM_PAGE_NUM_SIZE,
                                                
                                                [MEM_PAGE_NUM_3].x = 0,
                                                [MEM_PAGE_NUM_3].w = 64,
                                                [MEM_PAGE_NUM_3].y = 112,
                                                [MEM_PAGE_NUM_3].h = 64,
                                                [MEM_PAGE_NUM_3].size  = MEM_PAGE_NUM_SIZE,
                                                
                                                [MEM_PAGE_NUM_4].x = 0,
                                                [MEM_PAGE_NUM_4].w = 64,
                                                [MEM_PAGE_NUM_4].y = 176,
                                                [MEM_PAGE_NUM_4].h = 64,
                                                [MEM_PAGE_NUM_4].size  = MEM_PAGE_NUM_SIZE,};
                                                
                                                // [MEM_PAGE_NUM_5].x = 0,
                                                // [MEM_PAGE_NUM_5].w = 320,
                                                // [MEM_PAGE_NUM_5].y = 39,
                                                // [MEM_PAGE_NUM_5].h = 2,
                                                // [MEM_PAGE_NUM_5].size  = MEM_PAGE_NUM_5_SIZE};

static inline uint8_t mem_ram_read8(uint16_t addr);
static inline void    mem_ram_write8(uint16_t addr, uint8_t data);

static void mem_test(void)
{
    logger("memmory test start\r\n");
    for (uint32_t addr = 0x5B00; addr < 0x10000; ++addr)
    {
        mem_ram_write8(addr, ((addr)^0x55)&0xff);
    }

    for (uint32_t addr = 0x5B00; addr < 0x10000; ++addr)
    {
        uint8_t btt = ((addr) ^ 0x55) & 0xFF;
        if (mem_ram_read8(addr) != btt)
        {
            logger("Memmory test error block %04X\r\n", addr);
            break;
        }
    }
    logger("memmory test has finished\r\n");
}

static inline void mem_flash(mem_page_num_t page)
{
    // logger("%s page %d\r\n", __func__, page);
    ili9341_mem_write_rect(pages[page].x, pages[page].w, pages[page].y, pages[page].h,
                           memory.cache + (MEM_PAGE_NUM_SIZE - pages[page].size),
                           pages[page].size);
}

static inline uint8_t mem_ram_read8(uint16_t addr)
{
    const mem_page_num_t page_num = MEM_ADDR_TO_PAGE(addr);
    addr &= 0x1FFF;
    
    if (memory.num != page_num)
    {
        // logger("%s memory.num %d != page_num %d in cache %d\r\n", __func__, memory.num, page_num, page_num);
        if (memory.state == MEM_PAGE_STATE_MODIFIED)
        {
            
            mem_flash(memory.num);
        }

        ili9341_mem_read_rect(pages[page_num].x, pages[page_num].w, pages[page_num].y, pages[page_num].h,
                              memory.cache +(MEM_PAGE_NUM_SIZE - pages[page_num].size),
                              pages[page_num].size);

        memory.num   = page_num;
        memory.state = MEM_PAGE_STATE_READED;
    }

    return memory.cache[addr];
}

static inline void mem_ram_write8(uint16_t addr, uint8_t data)
{
    // if (addr < 0x6000)
        // logger("%s addr %04X\r\n", __func__, addr);
    const mem_page_num_t page_num = MEM_ADDR_TO_PAGE(addr);
    addr &= 0x1FFF;

    if (memory.num != page_num)
    {
        // logger("%s memory.num %d != page_num %d in cache %d\r\n", __func__, memory.num, page_num, page_num);
        if (memory.state == MEM_PAGE_STATE_MODIFIED)
        {
            
            mem_flash(memory.num);
        }

        ili9341_mem_read_rect(pages[page_num].x, pages[page_num].w, pages[page_num].y, pages[page_num].h,
                              memory.cache +(MEM_PAGE_NUM_SIZE - pages[page_num].size),
                              pages[page_num].size);
        memory.num   = page_num;
        memory.state = MEM_PAGE_STATE_READED;
    }

    memory.cache[addr] = data;
    memory.state       = MEM_PAGE_STATE_MODIFIED;
}

void mem_init(void)
{
    // mem_test();
}
static uint8_t ram[0x500];
uint8_t  inline mem_readbyte(uint16_t addr)
{
#ifdef MEM_DBG
    logger("%s A: %04X D: %02X\r\n", __func__, addr, __48_rom[addr]);
#endif /* MEM_DBG */

    uint8_t ret_data;

    if (addr >= 0x6000)
    {
        ret_data = mem_ram_read8(addr);
    }
    else if (addr >= 0x5B00)
    {
        ret_data = ram[addr - 0x5B00];
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

void mem_writebyte(uint16_t addr, uint8_t b)
{
#ifdef MEM_DBG
    logger("%s A: %04X D: %02X\r\n", __func__, addr, b);
#endif /* MEM_DBG */

    if (addr >= 0x6000)
    {
        mem_ram_write8(addr, b);
    }
    else if (addr >= 0x5B00)
    {
        ram[addr - 0x5B00] = b;
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

uint16_t  inline mem_readword(uint16_t addr)
{
    uint16_t ret_data;

    ret_data = ((mem_readbyte(addr + 1) << 8) | mem_readbyte(addr));
#ifdef MEM_DBG
    logger("%s A: %04X D: %04X\r\n", __func__, addr, ret_data);
#endif /* MEM_DBG */
    
    return ret_data;
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
#ifdef MEM_DBG
    logger("%s portHigh %02X portLow %02X d: %02X\r\n", __func__, portHigh,  portLow, ret);
#endif /* MEM_DBG */
    return ret;
}

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
            // screen_draw_border(data & 0x07);
            // logger("type %d\r\n", data & (1 << 3));
        }
        break;
        default:
            logger("%s unk %02X %02X D:%02X\r\n", __func__, portHigh, portLow, data);
            break;
    }
}