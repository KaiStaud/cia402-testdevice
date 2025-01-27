#include <lcd/st7789.h>
#include <ugui/ugui.h>
#include <ui/ui.h>

#define LCD_WIDTH ST7789_WIDTH
#define LCD_HEIGHT ST7789_HEIGHT


typedef struct{
  int8_t spi_sz;
  int8_t dma_sz;
  int8_t dma_mem_inc;
}config_t;

config_t config = {
    .spi_sz = -1,
    .dma_sz = -1,
    .dma_mem_inc = -1,
};

#ifdef LCD_LOCAL_FB
static uint16_t fb[LCD_WIDTH*LCD_HEIGHT];
#endif

static UG_GUI gui;
static UG_DEVICE device = {
    .x_dim = LCD_WIDTH,
    .y_dim = LCD_HEIGHT,
#ifdef LCD_LOCAL_FB
    .pset = LCD_DrawPixelFB,
#else
    .pset = ST7789_DrawPixel,
#endif
    .flush = ST7789_Update,
};

void LCD_init(void)
{
  UG_Init(&gui, &device);
  UG_DriverRegister(DRIVER_DRAW_LINE, ST7789_DrawLine);
  UG_DriverRegister(DRIVER_FILL_FRAME, ST7789_Fill_Color);
  UG_DriverRegister(DRIVER_FILL_AREA, ST7789_Fill);
  UG_DriverRegister(DRIVER_DRAW_BMP, ST7789_DrawImage);
  UG_FontSetHSpace(0);
  UG_FontSetVSpace(0);
  UG_FillScreen(C_BLACK);               //  Clear screen
  UG_Update();
}