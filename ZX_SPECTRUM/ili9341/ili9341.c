/********************************************************************
*                         Standard headers                          *
********************************************************************/

/********************************************************************
*                           Local headers                           *
********************************************************************/
#include "ili9341.h"
#include "stm32f1xx_hal.h"
#include "logger.h"
/********************************************************************
*                       Constant macro defines                      *
********************************************************************/
/* | CS | WR | RD |D/C|     Function            |
            _
   | L  | _/ | H  | L | write command code      |
                 _
   | L  | H  | _/ | H | read internal status    |
            _
   | L  | _/ | H  | H | write data or parameter |
                 _
   | L  | H  | _/ | H | reads data or parameter |
*/

/* Level 1 Commands */
#define LCD_NOP                 0x00   /* No operation */
#define LCD_SWRESET             0x01   /* Software Reset */
#define LCD_READ_DISPLAY_ID     0x04   /* Read display identification information */
#define LCD_RDDST               0x09   /* Read Display Status */
#define LCD_RDDPM               0x0A   /* Read Display Power Mode */
#define LCD_RDDMADCTL           0x0B   /* Read Display MADCTL */
#define LCD_RDDCOLMOD           0x0C   /* Read Display Pixel Format */
#define LCD_RDDIM               0x0D   /* Read Display Image Format */
#define LCD_RDDSM               0x0E   /* Read Display Signal Mode */
#define LCD_RDDSDR              0x0F   /* Read Display Self-Diagnostic Result */
#define LCD_SPLIN               0x10   /* Enter Sleep Mode */
#define LCD_SLEEP_OUT           0x11   /* Sleep out register */
#define LCD_PTLON               0x12   /* Partial Mode ON */
#define LCD_NORMAL_MODE_ON      0x13   /* Normal Display Mode ON */
#define LCD_DINVOFF             0x20   /* Display Inversion OFF */
#define LCD_DINVON              0x21   /* Display Inversion ON */
#define LCD_GAMMA               0x26   /* Gamma register */
#define LCD_DISPLAY_OFF         0x28   /* Display off register */
#define LCD_DISPLAY_ON          0x29   /* Display on register */
#define LCD_COLUMN_ADDR         0x2A   /* Colomn address register */
#define LCD_PAGE_ADDR           0x2B   /* Page address register */
#define LCD_GRAM                0x2C   /* GRAM register */
#define LCD_RGBSET              0x2D   /* Color SET */
#define LCD_RAMRD               0x2E   /* Memory Read */
#define LCD_PLTAR               0x30   /* Partial Area */
#define LCD_VSCRDEF             0x33   /* Vertical Scrolling Definition */
#define LCD_TEOFF               0x34   /* Tearing Effect Line OFF */
#define LCD_TEON                0x35   /* Tearing Effect Line ON */
#define LCD_MAC                 0x36   /* Memory Access Control register*/
#define LCD_VSCRSADD            0x37   /* Vertical Scrolling Start Address */
#define LCD_IDMOFF              0x38   /* Idle Mode OFF */
#define LCD_IDMON               0x39   /* Idle Mode ON */
#define LCD_PIXEL_FORMAT        0x3A   /* Pixel Format register */
#define LCD_WRITE_MEM_CONTINUE  0x3C   /* Write Memory Continue */
#define LCD_READ_MEM_CONTINUE   0x3E   /* Read Memory Continue */
#define LCD_SET_TEAR_SCANLINE   0x44   /* Set Tear Scanline */
#define LCD_GET_SCANLINE        0x45   /* Get Scanline */
#define LCD_WDB                 0x51   /* Write Brightness Display register */
#define LCD_RDDISBV             0x52   /* Read Display Brightness */
#define LCD_WCD                 0x53   /* Write Control Display register*/
#define LCD_RDCTRLD             0x54   /* Read CTRL Display */
#define LCD_WRCABC              0x55   /* Write Content Adaptive Brightness Control */
#define LCD_RDCABC              0x56   /* Read Content Adaptive Brightness Control */
#define LCD_WRITE_CABC          0x5E   /* Write CABC Minimum Brightness */
#define LCD_READ_CABC           0x5F   /* Read CABC Minimum Brightness */
#define LCD_READ_ID1            0xDA   /* Read ID1 */
#define LCD_READ_ID2            0xDB   /* Read ID2 */
#define LCD_READ_ID3            0xDC   /* Read ID3 */

/* Level 2 Commands */
#define LCD_RGB_INTERFACE       0xB0   /* RGB Interface Signal Control */
#define LCD_FRMCTR1             0xB1   /* Frame Rate Control (In Normal Mode) */
#define LCD_FRMCTR2             0xB2   /* Frame Rate Control (In Idle Mode) */
#define LCD_FRMCTR3             0xB3   /* Frame Rate Control (In Partial Mode) */
#define LCD_INVTR               0xB4   /* Display Inversion Control */
#define LCD_BPC                 0xB5   /* Blanking Porch Control register */
#define LCD_DFC                 0xB6   /* Display Function Control register */
#define LCD_ETMOD               0xB7   /* Entry Mode Set */
#define LCD_BACKLIGHT1          0xB8   /* Backlight Control 1 */
#define LCD_BACKLIGHT2          0xB9   /* Backlight Control 2 */
#define LCD_BACKLIGHT3          0xBA   /* Backlight Control 3 */
#define LCD_BACKLIGHT4          0xBB   /* Backlight Control 4 */
#define LCD_BACKLIGHT5          0xBC   /* Backlight Control 5 */
#define LCD_BACKLIGHT7          0xBE   /* Backlight Control 7 */
#define LCD_BACKLIGHT8          0xBF   /* Backlight Control 8 */
#define LCD_POWER1              0xC0   /* Power Control 1 register */
#define LCD_POWER2              0xC1   /* Power Control 2 register */
#define LCD_VCOM1               0xC5   /* VCOM Control 1 register */
#define LCD_VCOM2               0xC7   /* VCOM Control 2 register */
#define LCD_NVMWR               0xD0   /* NV Memory Write */
#define LCD_NVMPKEY             0xD1   /* NV Memory Protection Key */
#define LCD_RDNVM               0xD2   /* NV Memory Status Read */
#define LCD_READ_ID4            0xD3   /* Read ID4 */
#define LCD_PGAMMA              0xE0   /* Positive Gamma Correction register */
#define LCD_NGAMMA              0xE1   /* Negative Gamma Correction register */
#define LCD_DGAMCTRL1           0xE2   /* Digital Gamma Control 1 */
#define LCD_DGAMCTRL2           0xE3   /* Digital Gamma Control 2 */
#define LCD_INTERFACE           0xF6   /* Interface control register */

/* Extend register commands */
#define LCD_POWERA              0xCB   /* Power control A register */
#define LCD_POWERB              0xCF   /* Power control B register */
#define LCD_DTCA                0xE8   /* Driver timing control A */
#define LCD_DTCB                0xEA   /* Driver timing control B */
#define LCD_POWER_SEQ           0xED   /* Power on sequence register */
#define LCD_3GAMMA_EN           0xF2   /* 3 Gamma enable register */
#define LCD_PRC                 0xF7   /* Pump ratio control register */

/* GPIO pin control. */
/* TODO: Should be reworked to faster. (GPIOx->BSRR)*/
#define RESET_IDLE()            HAL_GPIO_WritePin(GPIOB, LCD_RST_Pin, GPIO_PIN_SET)
#define RESET_ACTIVE()          HAL_GPIO_WritePin(GPIOB, LCD_RST_Pin, GPIO_PIN_RESET)
#define CS_IDLE()               HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_SET)
#define CS_ACTIVE()             HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin, GPIO_PIN_RESET)
#define RS_DATA()               HAL_GPIO_WritePin(GPIOB, LCD_RS_Pin, GPIO_PIN_SET)
#define RS_COMMAND()            HAL_GPIO_WritePin(GPIOB, LCD_RS_Pin, GPIO_PIN_RESET)
#define WR_IDLE()               HAL_GPIO_WritePin(GPIOB, LCD_WR_Pin, GPIO_PIN_SET)
#define WR_ACTIVE()             HAL_GPIO_WritePin(GPIOB, LCD_WR_Pin, GPIO_PIN_RESET)
#define RD_IDLE()               HAL_GPIO_WritePin(GPIOB, LCD_RD_Pin, GPIO_PIN_SET)
#define RD_ACTIVE()             HAL_GPIO_WritePin(GPIOB, LCD_RD_Pin, GPIO_PIN_RESET)
#define WR_PAUSE()  
#define RD_PAUSE()    

#define PIN_OUTPUT              (0x33333333UL)
#define PIN_INPUT               (0x88888888UL)

#define TFT_WIGTH               (239)
#define TFT_HEIGHT              (319)

/********************************************************************
*                             Typedefs                              *
********************************************************************/

/********************************************************************
*                  Static global data declarations                  *
********************************************************************/

/********************************************************************
*                     Functions implementations                     *
********************************************************************/
static inline void set_write_bus(void)
{
    GPIOA->CRL = PIN_OUTPUT;
}

static inline void set_read_bus(void)
{
    GPIOA->CRL  = PIN_INPUT;
    GPIOA->BSRR = 0xFF;
}

static inline void send_cmd(uint8_t cmd)
{
    RS_COMMAND();
    WR_ACTIVE();
    GPIOA->ODR = (GPIOA->ODR & 0xFF00) | cmd;
    WR_PAUSE();
    WR_IDLE();
    RS_DATA();
}
static void inline read_multi_data(uint8_t *data, uint8_t len)
{
    set_read_bus();

    for (size_t i = 0; i < len; i++)
    {
        RD_ACTIVE();
        data[i] = (uint8_t)(GPIOA->IDR);
        RD_IDLE();
    }

    set_write_bus();
}
static inline void transmition_open(void)
{
    CS_ACTIVE();
}

static inline uint8_t read_data()
{   
    set_read_bus();

    RD_ACTIVE();
    uint8_t data = (uint8_t)(GPIOA->IDR);
    RD_IDLE();
    set_write_bus();
    return data;
}

static inline void send_data8(const uint8_t data)
{
    WR_ACTIVE();
    GPIOA->ODR = (GPIOA->ODR & 0xFF00) | data;
    WR_IDLE();
}

static inline void send_multi_data(const uint8_t *data, uint16_t len)
{
    for(uint16_t i = 0; i < len; ++i)
    {
        WR_ACTIVE();
        GPIOA->ODR = (GPIOA->ODR & 0xFF00) | data[i];
        WR_IDLE();
    }
}

static inline void send_data16(uint16_t data)
{
    WR_ACTIVE();
    GPIOA->ODR = (GPIOA->ODR & 0xFF00) | (data >> 8);
    WR_IDLE();

    WR_ACTIVE();
    GPIOA->ODR = (GPIOA->ODR & 0xFF00) | (data & 0x00FF);
    WR_IDLE();
}

static inline void transmition_close(void)
{
    CS_IDLE();
}

static inline void set_col(uint16_t start, uint16_t end)
{
    transmition_open();
    send_cmd(LCD_COLUMN_ADDR);
    send_data16(start);
    send_data16(end);
    transmition_close();
}

static inline void set_row(uint16_t start, uint16_t end)
{
    transmition_open();
    send_cmd(LCD_PAGE_ADDR);
    send_data16(start);
    send_data16(end);
    transmition_close();
}

static void ili9341_fill_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, display_color_t color)
{
    uint16_t x = (uint16_t)((x2 - x1) + 1);
    uint16_t y = (uint16_t)((y2 - y1) + 1);

    set_col(x1, x2);
    set_row(y1, y2);

    transmition_open();
    send_cmd(LCD_GRAM);

    uint32_t xy = ((uint32_t)x) * ((uint32_t)y);
    for(uint32_t i=0; i < xy; i++)
    {
        send_data16(color);
    }
    transmition_close();
}

void ili9341_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, LCD_D0_Pin|LCD_D1_Pin|LCD_D2_Pin|LCD_D3_Pin 
                            |LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, LCD_RD_Pin|LCD_RST_Pin|LCD_CS_Pin|LCD_RS_Pin 
                            |LCD_WR_Pin, GPIO_PIN_SET);

    /*Configure GPIO pins : LCD_D0_Pin LCD_D1_Pin LCD_D2_Pin LCD_D3_Pin 
                            LCD_D4_Pin LCD_D5_Pin LCD_D6_Pin LCD_D7_Pin */
    GPIO_InitStruct.Pin = LCD_D0_Pin|LCD_D1_Pin|LCD_D2_Pin|LCD_D3_Pin 
                            |LCD_D4_Pin|LCD_D5_Pin|LCD_D6_Pin|LCD_D7_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : LCD_RD_Pin LCD_RST_Pin LCD_CS_Pin LCD_RS_Pin 
                            LCD_WR_Pin */
    GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_RST_Pin|LCD_CS_Pin|LCD_RS_Pin 
                            |LCD_WR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
/********************************************************************
*                                 Api                               *
********************************************************************/
void ili9341_init(void)
{
    ili9341_gpio_init();

    CS_IDLE();
    RESET_IDLE();           //relese reset pin
    RS_DATA();
    WR_IDLE();
    RD_IDLE();
    set_write_bus();

    RESET_ACTIVE();
    RESET_IDLE();

    transmition_open();
    send_cmd(LCD_SWRESET);       //Software Reset
    transmition_close();
    HAL_Delay(1000);

    transmition_open();
    //Power Control 1
	//задаём градацию серого цвета
    send_cmd(LCD_POWER1);
    send_data8(0x25);


	//Power Control 2
    //настройка step up преобразователя
    send_cmd(LCD_POWER2);
  	send_data8(0x11);


	//VCOM Control 1
    //контрастность определяется разностью VCOMH - VCOML = 5.2V
    //VCOMH = 3.825 //VCOML = -1.375
  	send_cmd(LCD_VCOM1);
    send_data8(0x2B);
    send_data8(0x2B);

	//VCOM Control 2
  	//на Vcom по сути ШИМ, а тут мы задаем offset для него
    //VML=58 VMH=58
    send_cmd(LCD_VCOM2);
    send_data8(0x86);

	//COLMOD: Pixel Format Set
  	//один пиксель будет кодироваться 16 битами
    send_cmd(LCD_PIXEL_FORMAT);
    send_data8(0x05);

	//Frame Rate Control
    //Frame Rate 79Hz
    send_cmd(LCD_FRMCTR1);
    send_data8(0x00);
    send_data8(0x18);


	//Display Function Control
    //восьмой бит определяет нормальный цвет кристала белый - 1, черный - 0,
    send_cmd(LCD_DFC);
    send_data8(0x0A);
    send_data8(0x82);
    send_data8(0x27);

	// Sleep Out
  	send_cmd(LCD_SLEEP_OUT);
    transmition_close();

	HAL_Delay(120);

    //Memory Access Control
	ili9341_set_orientation(ILI9341_ORIENT_270);

	//Display On
    transmition_open();
  	send_cmd(LCD_DISPLAY_ON);
    transmition_close();

    ili9341_fill_area(0, 0, TFT_HEIGHT, TFT_WIGTH, TFT_BLACK);
}

void ili9341_set_orientation(ili9341_orient_t orient)
{
    transmition_open();
    send_cmd(LCD_MAC);

    switch (orient)
    {
        case ILI9341_ORIENT_0:   send_data8(0x48); break;
        case ILI9341_ORIENT_90:  send_data8(0x28); break;
        case ILI9341_ORIENT_180: send_data8(0x88); break;
        case ILI9341_ORIENT_270: send_data8(0xE8); break;
        default: send_data8(LCD_NOP); break;
    }

    transmition_close();
}

void ili9341_set_address_windows(uint16_t x, uint16_t w, uint16_t y, uint16_t h)
{
    transmition_open();
    send_cmd(LCD_COLUMN_ADDR);

    send_data16(x);
    send_data16(x + w);


    send_cmd(LCD_PAGE_ADDR);
    send_data16(y);
    send_data16(y + h);

    transmition_close();
}

void ili9341_mem_read(uint8_t *data, uint16_t len, bool skip_first_byte)
{
    transmition_open();
    send_cmd(LCD_RAMRD);

    if (skip_first_byte)
    {
        (void)read_data();
    }

    uint16_t *data_u16 = (uint16_t *)data;
    for (uint16_t i = 0; i < len / 2; i++)
    {
        uint8_t raw[3];
        read_multi_data(raw, sizeof(raw));
        data_u16[i]  = (raw[0] >> 3) << 11;
        data_u16[i] |= (raw[1] >> 2) << 5;
        data_u16[i] |= (raw[2] >> 3);
    }
    
    transmition_close();
}

void ili9341_mem_write(const uint8_t *data, uint16_t len)
{
    transmition_open();
    send_cmd(LCD_GRAM);

    uint16_t *data_u16 = (uint16_t *) data;

    for (uint16_t i = 0; i < 64 / sizeof(uint16_t); ++i)
    {
        send_data16(*data_u16++);
    }

    transmition_close();
}

void ili9341_8x8block_draw(uint16_t x, uint16_t y, const uint16_t *data)
{
    transmition_open();
    send_cmd(LCD_COLUMN_ADDR);

    send_data16(x);
    send_data16(x + 7);


    send_cmd(LCD_PAGE_ADDR);
    send_data16(y);
    send_data16(y + 7);

    send_cmd(LCD_GRAM);

    for (uint16_t i = 0; i < (8 * 8); ++i)
    {
        send_data16(*data);
        data++;
    }
    transmition_close();
}

#if 0
void ili9341_64block_mem_read(uint16_t x, uint16_t y, uint8_t *data)
{
    // logger("%s x %d y %d\r\n", __func__, x, y);
    transmition_open();
    send_cmd(LCD_COLUMN_ADDR);

    send_data16(x);
    send_data16(x + 7);


    send_cmd(LCD_PAGE_ADDR);
    send_data16(y);
    send_data16(y + 3);

    send_cmd(LCD_RAMRD);
    (void)read_data();  /* Dummy data. */
    uint16_t *data_u16 = (uint16_t *)data;
    for (size_t i = 0; i < 64 / sizeof(uint16_t); i++)
    {
        uint8_t raw[3];
        read_multi_data(raw, sizeof(raw));
        data_u16[i]  = (raw[0] >> 3) << 11;
        data_u16[i] |= (raw[1] >> 2) << 5;
        data_u16[i] |= (raw[2] >> 3);
    }

    transmition_close();
}

void ili9341_8kblock_mem_write(uint16_t x, uint16_t y, const uint8_t *data)
{
    // logger("%s x %d y %d\r\n", __func__, x, y);
    transmition_open();
    send_cmd(LCD_COLUMN_ADDR);

    send_data16(x);
    send_data16(x + 7);


    send_cmd(LCD_PAGE_ADDR);
    send_data16(y);
    send_data16(y + 3);

    send_cmd(LCD_GRAM);

    uint16_t *data_u16 = (uint16_t *) data;

    for (uint16_t i = 0; i < 64 / sizeof(uint16_t); ++i)
    {
        send_data16(*data_u16++);
    }
    transmition_close();
}
#else

void ili9341_mem_read_rect(uint16_t x, uint16_t w, uint16_t y, uint16_t h, uint8_t *data, uint16_t len)
{
    transmition_open();
    send_cmd(LCD_COLUMN_ADDR);

    send_data16(x);
    send_data16(x + w-1);


    send_cmd(LCD_PAGE_ADDR);
    send_data16(y);
    send_data16(y + h-1);

    send_cmd(LCD_RAMRD);
    (void)read_data();  /* Dummy data. */
    uint16_t *data_u16 = (uint16_t *)data;

    len = len / sizeof(uint16_t);
    
    for (size_t i = 0; i < len; i++)
    {
        uint8_t raw[3];
        read_multi_data(raw, sizeof(raw));
        data_u16[i]  = (raw[0] >> 3) << 11;
        data_u16[i] |= (raw[1] >> 2) << 5;
        data_u16[i] |= (raw[2] >> 3);
    }

    transmition_close();
}

void ili9341_mem_write_rect(uint16_t x, uint16_t w, uint16_t y, uint16_t h, const uint8_t *data, uint16_t len)
{
    transmition_open();
    send_cmd(LCD_COLUMN_ADDR);

    send_data16(x);
    send_data16(x + w-1);

    send_cmd(LCD_PAGE_ADDR);
    send_data16(y);
    send_data16(y + h-1);

    send_cmd(LCD_GRAM);

    uint16_t *data_u16 = (uint16_t *) data;
    len = len / sizeof(uint16_t);
    while (len--)
    {
        send_data16(*data_u16++);
    }
    transmition_close();
}
#endif
void ili9341_fill_rect(uint16_t x, uint16_t w, uint16_t y, uint16_t h, uint16_t color)
{
    transmition_open();
    send_cmd(LCD_COLUMN_ADDR);

    send_data16(x);
    send_data16(x + w-1);


    send_cmd(LCD_PAGE_ADDR);
    send_data16(y);
    send_data16(y + h-1);

    send_cmd(LCD_GRAM);

    for (uint16_t i = 0; i < (w * h); ++i)
    {
        send_data16(color);
    }
    transmition_close();
}