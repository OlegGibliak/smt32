#include "screen.h"
#include "logger.h"
#include "display.h"

#define SCREEN_DISPLAY_ATTRIBUTES_W     (32)
#define SCREEN_DISPLAY_ATTRIBUTES_H     (24)

#define SCREEN_DISPLAY_W                (256)
#define SCREEN_DISPLAY_H                (192)
#define SCREEN_DISPLAY_ADDR_START_X     (32)
#define SCREEN_DISPLAY_ADDR_END_X       (SCREEN_DISPLAY_ADDR_START_X + SCREEN_DISPLAY_W)
#define SCREEN_DISPLAY_ADDR_START_Y     (24)
#define SCREEN_DISPLAY_ADDR_END_Y       (SCREEN_DISPLAY_ADDR_START_Y + SCREEN_DISPLAY_H)


#define SCREEN_AREA_START    (0x4000)
#define SCREEN_AREA_END      (0x57FF)
#define SCREEN_ATTR_START    (SCREEN_AREA_END + 1)
#define SCREEN_ATTR_END      (0x5AFF)

#define SCREEN_THIRDS        (3)
#define SCREEN_CHARS_ROW     (8)
#define SCREEN_ROWS          (8)
#define SCREEN_BYTES_IN_LINE (32)

#define THIRDS(_addr_)       ((_addr_ >> 11) & 0b11)
#define CHAR(_addr_)         ((_addr_ >> 8)  & 0b111)
#define ROW(_addr_)          ((_addr_ >> 5)  & 0b111)
#define BYTE_IN_LINE(_addr_) (_addr_ & 0b11111)

#define SCREEN_SINCLAR_COLORS_NUM       (16)

/*
 * 7  6  5  4  3  2  1  0
 * F  B  ---P---  ---I---
 * F - FLASH
 * B - BRIGHT
 * P - PAPER color
 * I - INK color
 */
typedef union
{
    struct {
        uint8_t ink    : 3;
        uint8_t paper  : 3;
        uint8_t bright : 1;
        uint8_t flash  : 1;
    };
    uint8_t intr;
} char_attribute_t;

/*
 * 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 *  0  1  0  T  T  S  S  S  C  C  C  H  H  H  H  H
 * 010 - 0x4000
 * T   - THIRDS
 * S   - CHAR in thirds
 * C   - ROW in char
 * H   - 32 bytes of line (256 bits)
 */
static uint8_t          screen_area[SCREEN_THIRDS]
                                   [SCREEN_CHARS_ROW]
                                   [SCREEN_ROWS]
                                   [SCREEN_BYTES_IN_LINE];
static char_attribute_t char_attribute[SCREEN_THIRDS * SCREEN_CHARS_ROW * SCREEN_BYTES_IN_LINE];
static const uint16_t   colors_table[SCREEN_SINCLAR_COLORS_NUM] = 
    {0x0000, 0x001A, 0xD000, 0xD01A, 0x06A0, 0x06BA, 0xD6A0, 0xD6BA,
     0x0000, 0x001F, 0xF800, 0xF81F, 0x07E0, 0x07FF, 0xFFE0, 0xFFFF};
static char_attribute_t border;

void screen_mem_write(uint16_t addr, uint8_t data)
{
    if (addr >= SCREEN_AREA_START && addr <= SCREEN_AREA_END)
    {
        screen_area[THIRDS(addr)][CHAR(addr)][ROW(addr)][BYTE_IN_LINE(addr)] = data;
    }
    else if (addr >= SCREEN_ATTR_START && addr <= SCREEN_ATTR_END)
    {
        char_attribute[addr - SCREEN_ATTR_START].intr = data;
        // logger("ATTRIBUTE WR addr %04X data %02X\r\n", addr - SCREEN_ATTR_START, data);
    }
    else
    {
        logger("%s invalid addr %04X data %02X\r\n", __func__, addr, data);
    }
    
}

uint8_t screen_mem_read(uint16_t addr)
{
    if (addr >= SCREEN_AREA_START && addr <= SCREEN_AREA_END)
    {
        return screen_area[THIRDS(addr)][CHAR(addr)][ROW(addr)][BYTE_IN_LINE(addr)];
    }
    else if (addr >= SCREEN_ATTR_START && addr <= SCREEN_ATTR_END)
    {
        // logger("ATTRIBUTE RD addr %04X data %02X\r\n", addr, char_attribute[addr - SCREEN_ATTR_START].intr);
        return char_attribute[addr - SCREEN_ATTR_START].intr;
    }
    else
    {
        logger("%s invalid addr %04X\r\n", __func__, addr);
    }

    return 0;
}

void screen_char_get(uint8_t char_row, uint8_t char_clomn, uint8_t *bitmap, char_color_565_t *color)
{
    uint8_t thirds = char_row / SCREEN_CHARS_ROW;
    uint8_t row    = char_row - (thirds * 8);

    for (uint8_t i = 0; i < 8; i++)
    {
        bitmap[i] = screen_area[thirds][i][row][char_clomn];
    }
    
    uint16_t attribute_offset = (char_row * SCREEN_DISPLAY_ATTRIBUTES_W) + char_clomn;
    
    uint8_t color_index = char_attribute[attribute_offset].ink;
    color_index |= (char_attribute[attribute_offset].bright << 3);
    color->inc = colors_table[color_index];  

    color_index = char_attribute[attribute_offset].paper;
    color_index |= (char_attribute[attribute_offset].bright << 3);
    color->paper = colors_table[color_index];
}

void screen_draw(void)
{
    uint8_t bitmap[8];
    char_color_565_t color;
    
    for (uint16_t y = 0; y < 24; ++y)
    {
        for (uint16_t x = 0; x < 32; ++x)
        {
            screen_char_get(y, x, bitmap, &color);

            uint16_t c[8][8];
            for (uint8_t b = 0; b < 8; ++b)
            {
                for (uint8_t n = 0; n < 8; ++n)
                {
                    c[b][n] = (bitmap[b] & (1 << (7-n))) ? color.inc : color.paper; 
                }
            } 

            display_8x8block_draw(SCREEN_DISPLAY_ADDR_START_X + (x * 8),
                                  SCREEN_DISPLAY_ADDR_START_Y + (y * 8),
                                  &c[0][0]);
        }
    }
}

void screen_draw_border(uint8_t data)
{
    char_attribute_t color = {.intr = data};
    if (border.ink != color.ink)
    {
        border.intr = color.intr;
        logger("border %04X\r\n", colors_table[border.ink]);
        display_fill_area(0, 320, 0, 24, colors_table[border.ink]);

        display_fill_area(0,
                          32,
                          SCREEN_DISPLAY_ADDR_START_Y,
                          SCREEN_DISPLAY_H,
                          colors_table[border.ink]);

        display_fill_area(0, 320, 216, 24, colors_table[border.ink]);

        display_fill_area(288,
                          32,
                          SCREEN_DISPLAY_ADDR_START_Y,
                          SCREEN_DISPLAY_H,
                          colors_table[border.ink]);
    }
}
#if 0
#include <string.h>
static uint16_t dma_buff[8][8];

static uint8_t line;
static uint8_t place;
uint8_t draw_cplt = 0;
void dma_cb(void)
{
    // if (draw_cplt == 0)
        screen_draw_run();
}
void screen_draw_run(void)
{
    
    uint8_t bitmap[8];
    char_color_565_t color;
    
    screen_char_get(line, place, bitmap, &color);

    for (uint8_t b = 0; b < 8; ++b)
    {
        for (uint8_t n = 0; n < 8; ++n)
        {
            dma_buff[b][n] = (bitmap[b] & (1 << (7-n))) ? color.inc : color.paper; 
        }
    } 

    // display_8x8block_draw(SCREEN_DISPLAY_ADDR_START_X + (x * 8),
    //                         SCREEN_DISPLAY_ADDR_START_Y + (y * 8),
    //                         &c[0][0]);

    display_dma_draw(SCREEN_DISPLAY_ADDR_START_X + (place * 8),
                     8,
                     SCREEN_DISPLAY_ADDR_START_Y + (line * 8),
                     8, 
                     (uint8_t *)dma_buff,
                     sizeof(dma_buff),
                     dma_cb);
    
    place++;
    if (place >= 32)
    {
        place = 0;
        line++;
        if (line >= 24)
        {
            line = 0;
            draw_cplt = 1;
        }
    }

    

    
}
#endif