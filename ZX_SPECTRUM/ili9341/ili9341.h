#ifndef ILI9341_H__
#define ILI9341_H__

#include <stdint.h>
#include <stdbool.h>

#define LCD_D0_Pin GPIO_PIN_0
#define LCD_D0_GPIO_Port GPIOA
#define LCD_D1_Pin GPIO_PIN_1
#define LCD_D1_GPIO_Port GPIOA
#define LCD_D2_Pin GPIO_PIN_2
#define LCD_D2_GPIO_Port GPIOA
#define LCD_D3_Pin GPIO_PIN_3
#define LCD_D3_GPIO_Port GPIOA
#define LCD_D4_Pin GPIO_PIN_4
#define LCD_D4_GPIO_Port GPIOA
#define LCD_D5_Pin GPIO_PIN_5
#define LCD_D5_GPIO_Port GPIOA
#define LCD_D6_Pin GPIO_PIN_6
#define LCD_D6_GPIO_Port GPIOA
#define LCD_D7_Pin GPIO_PIN_7
#define LCD_D7_GPIO_Port GPIOA
#define LCD_RD_Pin GPIO_PIN_11
#define LCD_RD_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_12
#define LCD_RST_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_13
#define LCD_CS_GPIO_Port GPIOB
#define LCD_RS_Pin GPIO_PIN_14
#define LCD_RS_GPIO_Port GPIOB
#define LCD_WR_Pin GPIO_PIN_15
#define LCD_WR_GPIO_Port GPIOB

typedef enum
{
    TFT_BLACK       = 0x0000,      /*   0,   0,   0 */
    TFT_NAVY        = 0x000F,      /*   0,   0, 128 */
    TFT_DARKGREEN   = 0x03E0,      /*   0, 128,   0 */
    TFT_DARKCYAN    = 0x03EF,      /*   0, 128, 128 */
    TFT_MAROON      = 0x7800,      /* 128,   0,   0 */
    TFT_PURPLE      = 0x780F,      /* 128,   0, 128 */
    TFT_OLIVE       = 0x7BE0,      /* 128, 128,   0 */
    TFT_LIGHTGREY   = 0xC618,      /* 192, 192, 192 */
    TFT_DARKGREY    = 0x7BEF,      /* 128, 128, 128 */
    TFT_BLUE        = 0x001F,      /*   0,   0, 255 */
    TFT_GREEN       = 0x07E0,      /*   0, 255,   0 */
    TFT_CYAN        = 0x07FF,      /*   0, 255, 255 */
    TFT_RED         = 0xF800,      /* 255,   0,   0 */
    TFT_MAGENTA     = 0xF81F,      /* 255,   0, 255 */
    TFT_YELLOW      = 0xFFE0,      /* 255, 255,   0 */
    TFT_WHITE       = 0xFFFF,      /* 255, 255, 255 */
    TFT_ORANGE      = 0xFD20,      /* 255, 165,   0 */
    TFT_GREENYELLOW = 0xAFE5,      /* 173, 255,  47 */
    TFT_PINK        = 0xF81F
} display_color_t;

typedef enum
{
    ILI9341_ORIENT_0,
    ILI9341_ORIENT_90,
    ILI9341_ORIENT_180,
    ILI9341_ORIENT_270
} ili9341_orient_t;

void ili9341_init(void);
void ili9341_set_orientation(ili9341_orient_t orient);

void ili9341_set_address_windows(uint16_t x, uint16_t w, uint16_t y, uint16_t h);
void ili9341_mem_read(uint8_t *data, uint16_t len, bool skip_first_byte);
void ili9341_mem_write(const uint8_t *data, uint16_t len);

void ili9341_64block_mem_write(uint16_t x, uint16_t y, const uint8_t *data);
void ili9341_64block_mem_read(uint16_t x, uint16_t y, uint8_t *data);

void ili9341_8x8block_draw(uint16_t x, uint16_t y, const uint16_t *data);

void ili9341_fill_rect(uint16_t x, uint16_t w, uint16_t y, uint16_t h, uint16_t color);
#endif /* ILI9341_H__ */