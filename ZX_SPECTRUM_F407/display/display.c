#include "display.h"
#include "logger.h"
#include "stm32f4xx_hal.h"

#define DISPLAY_CMD_ADDR        *(volatile uint8_t*)0x60000000
#define DISPLAY_DATA_ADDR       *(volatile uint8_t*)0x60040000

#define RESET_IDLE()            HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET)
#define RESET_ACTIVE()          HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET)

#if 0
static void display_delay(uint32_t __IO micros)
{        
    micros *=(SystemCoreClock/1000000)/168;
	while(micros--);
}
#else
#define display_delay(...)
#endif

static inline void display_write_cmd(uint8_t cmd)
{
    DISPLAY_CMD_ADDR = cmd;
    display_delay(1);
}

static inline void display_write_data(uint8_t data)
{
    DISPLAY_DATA_ADDR = data;
    display_delay(1);
}

static inline uint8_t display_read_data(void)
{
    uint8_t data = DISPLAY_DATA_ADDR;
    display_delay(1);
    return data;
}

static inline void display_write_data16(uint16_t data)
{
    display_write_data(data >> 8);
	display_write_data(data & 0xFF);
}

static void display_set_window(uint16_t x, uint16_t w, uint16_t y, uint16_t h)
{
	display_write_cmd(LCD_COLUMN_ADDR);
    display_write_data16(x);
    display_write_data16(x + (w - 1));

	display_write_cmd(LCD_PAGE_ADDR);
    display_write_data16(y);
    display_write_data16(y + (h - 1));
}

void display_reset(void)
{
    RESET_ACTIVE();
    HAL_Delay(2);
    RESET_IDLE();
    display_write_cmd(0x01); //Software Reset
} 

void display_init(void)
{
    display_reset();
    HAL_Delay(1000);

    //Power Control 1
	//задаём градацию серого цвета
    display_write_cmd(LCD_POWER1);
    display_write_data(0x25);

	//Power Control 2
    //настройка step up преобразователя
    display_write_cmd(LCD_POWER2);
  	display_write_data(0x11);

	//VCOM Control 1
    //контрастность определяется разностью VCOMH - VCOML = 5.2V
    //VCOMH = 3.825 //VCOML = -1.375
  	display_write_cmd(LCD_VCOM1);
    display_write_data(0x2B);
    display_write_data(0x2B);

	//VCOM Control 2
  	//на Vcom по сути ШИМ, а тут мы задаем offset для него
    //VML=58 VMH=58
    display_write_cmd(LCD_VCOM2);
    display_write_data(0x86);

	//COLMOD: Pixel Format Set
  	//один пиксель будет кодироваться 16 битами
    display_write_cmd(LCD_PIXEL_FORMAT);
    display_write_data(0x55);

	//Frame Rate Control
    //Frame Rate 79Hz
    display_write_cmd(LCD_FRMCTR1);
    display_write_data(0x00);
    display_write_data(0x18);


	//Display Function Control
    //восьмой бит определяет нормальный цвет кристала белый - 1, черный - 0,
    display_write_cmd(LCD_DFC);
    display_write_data(0x0A);
    display_write_data(0x82);
    display_write_data(0x27);

	// Sleep Out
  	display_write_cmd(LCD_SLEEP_OUT);

	HAL_Delay(120);

    //Memory Access Control
	display_set_orientation(ILI9341_ORIENT_270);

	//Display On
  	display_write_cmd(LCD_DISPLAY_ON);
    HAL_Delay(100);
    display_fill_area(0, 320, 0, 240, TFT_BLACK);
}

uint32_t display_read_id(void)
{
    display_write_cmd(0xD3);

    uint32_t id = display_read_data() << 24;
    id |= display_read_data() << 16;
    id |= display_read_data() << 8;
    id |= display_read_data() & 0xFF;
    return id;
}

void display_set_orientation(ili9341_orient_t orient)
{
    display_write_cmd(LCD_MAC);

    switch (orient)
    {
        case ILI9341_ORIENT_0:   display_write_data(0x48); break;
        case ILI9341_ORIENT_90:  display_write_data(0x28); break;
        case ILI9341_ORIENT_180: display_write_data(0x88); break;
        case ILI9341_ORIENT_270: display_write_data(0xE8); break;
        default: display_write_data(LCD_NOP); break;
    }
}

void display_draw_pixel(int x, int y, display_color_t color)
{
	display_set_window(x,y,x,y);
	display_write_cmd(0x2C);
	display_write_data16(color);
}

// 0 50 0 50
void display_fill_area(uint16_t x, uint16_t w, uint16_t y, uint16_t h, uint16_t color)
{
    display_set_window(x, w, y, h);
    display_write_cmd(LCD_GRAM);

    for (uint32_t i = 0; i < (w * h); ++i)
    {
        display_write_data16(color);
    }
}

void display_8x8block_draw(uint16_t x, uint16_t y, const uint16_t *data)
{
    display_set_window(x, 8, y, 8);
    display_write_cmd(LCD_GRAM);

    for (uint16_t i = 0; i < (8 * 8); ++i)
    {
        display_write_data16(*data++);
    }
}

// void HAL_SRAM_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma)
// {
//     if (dma_cplt_callback != NULL)
//     {
//         dma_cplt_callback();
//     }
// }

// void display_dma_draw(uint16_t x, uint16_t w, uint16_t y, uint16_t h, uint8_t *buff, uint32_t len,
//                       void (*display_dma_cplt_cb)(void))
// {
//     dma_cplt_callback = display_dma_cplt_cb;
//     display_set_window(x, w, y, h);
//     display_write_cmd(LCD_GRAM);
    
//     uint32_t *addr = (uint32_t *)0x60040000;

//     HAL_StatusTypeDef status = HAL_SRAM_Write_DMA(&hsram1, addr, (uint32_t*)buff, len);
    
//     if (status != HAL_OK)
//     {
//         logger("%s %d error %02X\r\n", __func__, __LINE__ , status);
//     }
// }

