#ifndef SCREEN_H__
#define SCREEN_H__

#include <stdint.h>

typedef struct
{
    uint16_t inc;
    uint16_t paper;
} char_color_565_t;

void screen_mem_write(uint16_t addr, uint8_t data);

uint8_t screen_mem_read(uint16_t addr);

void screen_char_get(uint8_t char_row, uint8_t char_clomn, uint8_t *bitmap, char_color_565_t *color);

void screen_draw(void);

void screen_draw_border(uint8_t data);

#endif /* SCREEN_H__ */