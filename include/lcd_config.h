#ifndef LCD_CONFIG_H
#define LCD_CONFIG_H

#ifdef TTGO_T1
#define LCD_SPI_HOST    SPI3_HOST
#define LCD_DMA
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_MOSI 19
#define LCD_PIN_NUM_CLK 18
#define LCD_PIN_NUM_CS 5
#define LCD_PIN_NUM_DC 16
#define LCD_PIN_NUM_RST 23
#define LCD_PIN_NUM_BCKL 4
#define LCD_PANEL esp_lcd_new_panel_st7789
#define LCD_HRES 135
#define LCD_VRES 240
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_RGB
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_GAP_X 40
#define LCD_GAP_Y 52
#define LCD_MIRROR_X false
#define LCD_MIRROR_Y true
#define LCD_INVERT_COLOR true
#define LCD_SWAP_XY true
#endif // TTGO_T1

#ifdef ESP_WROVER_KIT
#include <esp_lcd_panel_ili9341.h>
#define LCD_BCKL_ON_LEVEL 0
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_SPI_HOST    HSPI_HOST
#define LCD_DMA
#define LCD_PIN_NUM_MISO 25
#define LCD_PIN_NUM_MOSI 23
#define LCD_PIN_NUM_CLK  19
#define LCD_PIN_NUM_CS   22
#define LCD_PIN_NUM_DC   21
#define LCD_PIN_NUM_RST  18
#define LCD_PIN_NUM_BCKL 5
#define LCD_PANEL esp_lcd_new_panel_ili9341
#define LCD_HRES 240
#define LCD_VRES 320
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 0
#define LCD_MIRROR_X false
#define LCD_MIRROR_Y false
#define LCD_INVERT_COLOR false
#define LCD_SWAP_XY true
#endif // ESP_WROVER_KIT

#ifdef ESP_DISPLAY_S3
#include <esp_lcd_panel_ili9488.h>
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_DMA
#define LCD_PIN_NUM_CS 37
#define LCD_PIN_NUM_WR 35
#define LCD_PIN_NUM_RD 48
#define LCD_PIN_NUM_RS 36
#define LCD_PIN_NUM_D00 47
#define LCD_PIN_NUM_D01 21
#define LCD_PIN_NUM_D02 14
#define LCD_PIN_NUM_D03 13
#define LCD_PIN_NUM_D04 12
#define LCD_PIN_NUM_D05 11
#define LCD_PIN_NUM_D06 10
#define LCD_PIN_NUM_D07 9
#define LCD_PIN_NUM_D08 3
#define LCD_PIN_NUM_D09 8
#define LCD_PIN_NUM_D10 16
#define LCD_PIN_NUM_D11 15
#define LCD_PIN_NUM_D12 7
#define LCD_PIN_NUM_D13 6
#define LCD_PIN_NUM_D14 5
#define LCD_PIN_NUM_D15 4
#define LCD_PIN_NUM_BCKL 45
#define LCD_PANEL esp_lcd_new_panel_ili9488
#define LCD_HRES 320
#define LCD_VRES 480
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_PIXEL_CLOCK_HZ (20 * 1000 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 0
#define LCD_MIRROR_X true
#define LCD_MIRROR_Y true
#define LCD_INVERT_COLOR false
#define LCD_SWAP_XY true
#define LCD_SWAP_COLOR_BYTES true
#endif // ESP_DISPLAY_S3

#ifdef M5STACK_CORE2
#include <esp_lcd_panel_ili9342.h>
#define LCD_SPI_HOST    SPI3_HOST
#define LCD_DMA
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_MOSI 23
#define LCD_PIN_NUM_CLK 18
#define LCD_PIN_NUM_CS 5
#define LCD_PIN_NUM_DC 15
#define LCD_PANEL esp_lcd_new_panel_ili9342
#define LCD_HRES 320
#define LCD_VRES 240
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 0
#define LCD_MIRROR_X false
#define LCD_MIRROR_Y false
#define LCD_INVERT_COLOR true
#define LCD_SWAP_XY false
#endif // M5STACK_CORE2

#ifdef M5STACK_FIRE
#include <esp_lcd_panel_ili9342.h>
#define LCD_SPI_HOST    SPI3_HOST
#define LCD_DMA
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_MOSI 23
#define LCD_PIN_NUM_CLK 18
#define LCD_PIN_NUM_CS 14
#define LCD_PIN_NUM_DC 27
#define LCD_PIN_NUM_RST 33
#define LCD_PIN_NUM_BCKL 32
#define LCD_PANEL esp_lcd_new_panel_ili9342
#define LCD_HRES 240
#define LCD_VRES 320
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 0
#define LCD_MIRROR_X false
#define LCD_MIRROR_Y false
#define LCD_INVERT_COLOR true
#define LCD_SWAP_XY true
#endif // M5STACK_FIRE

#ifdef ESP_DISPLAY_4INCH
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_CS 1
#define LCD_PIN_NUM_SCK 12
#define LCD_PIN_NUM_SDA 11 
#define LCD_PIN_NUM_DE 45
#define LCD_PIN_NUM_VSYNC 4
#define LCD_PIN_NUM_HSYNC 5
#define LCD_PIN_NUM_CLK 21
#define LCD_PIN_NUM_D00 6
#define LCD_PIN_NUM_D01 7
#define LCD_PIN_NUM_D02 15
#define LCD_PIN_NUM_D03 16
#define LCD_PIN_NUM_D04 8
#define LCD_PIN_NUM_D05 0
#define LCD_PIN_NUM_D06 9
#define LCD_PIN_NUM_D07 14
#define LCD_PIN_NUM_D08 47
#define LCD_PIN_NUM_D09 48
#define LCD_PIN_NUM_D10 3
#define LCD_PIN_NUM_D11 39
#define LCD_PIN_NUM_D12 40
#define LCD_PIN_NUM_D13 41
#define LCD_PIN_NUM_D14 42
#define LCD_PIN_NUM_D15 2
#define LCD_PIN_NUM_BCKL -1
#define LCD_HSYNC_POLARITY 0
#define LCD_HSYNC_FRONT_PORCH 10
#define LCD_HSYNC_PULSE_WIDTH 8
#define LCD_HSYNC_BACK_PORCH 50
#define LCD_VSYNC_POLARITY 0
#define LCD_VSYNC_FRONT_PORCH 10
#define LCD_VSYNC_PULSE_WIDTH 8
#define LCD_VSYNC_BACK_PORCH 20
#define LCD_CLK_IDLE_HIGH 0
#define LCD_DE_IDLE_HIGH 1
#define LCD_BIT_DEPTH 16
#define LCD_PANEL esp_lcd_new_panel_st7701
#define LCD_HRES 480
#define LCD_VRES 480
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_SWAP_COLOR_BYTES false
#ifdef CONFIG_SPIRAM_MODE_QUAD
    #define LCD_PIXEL_CLOCK_HZ (6 * 1000 * 1000)
#else
    #define LCD_PIXEL_CLOCK_HZ (14 * 1000 * 1000)
#endif
#endif // ESP_DISPLAY_4INCH

#ifdef ESP_DISPLAY_4_3INCH
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
//#define LCD_PIN_NUM_CS 1
//#define LCD_PIN_NUM_SCK 12
//#define LCD_PIN_NUM_SDA 11 
#define LCD_PIN_NUM_DE 40
#define LCD_PIN_NUM_VSYNC 41
#define LCD_PIN_NUM_HSYNC 39
#define LCD_PIN_NUM_CLK 42
#define LCD_PIN_NUM_D00 8
#define LCD_PIN_NUM_D01 2
#define LCD_PIN_NUM_D02 46
#define LCD_PIN_NUM_D03 9
#define LCD_PIN_NUM_D04 1
#define LCD_PIN_NUM_D05 5
#define LCD_PIN_NUM_D06 6
#define LCD_PIN_NUM_D07 7
#define LCD_PIN_NUM_D08 15
#define LCD_PIN_NUM_D09 16
#define LCD_PIN_NUM_D10 4
#define LCD_PIN_NUM_D11 45
#define LCD_PIN_NUM_D12 48
#define LCD_PIN_NUM_D13 47
#define LCD_PIN_NUM_D14 21
#define LCD_PIN_NUM_D15 14
#define LCD_PIN_NUM_BCKL -1
#define LCD_HSYNC_POLARITY 0
#define LCD_HSYNC_FRONT_PORCH 8
#define LCD_HSYNC_PULSE_WIDTH 4
#define LCD_HSYNC_BACK_PORCH 8
#define LCD_VSYNC_POLARITY 0
#define LCD_VSYNC_FRONT_PORCH 8
#define LCD_VSYNC_PULSE_WIDTH 4
#define LCD_VSYNC_BACK_PORCH 8
#define LCD_CLK_IDLE_HIGH 1
#define LCD_DE_IDLE_HIGH 0
#define LCD_BIT_DEPTH 16

//#define LCD_PANEL esp_lcd_new_panel_st7701
#define LCD_HRES 800
#define LCD_VRES 480
#define LCD_SWAP_HRES_VRES_TIMING
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_SWAP_COLOR_BYTES false
#ifdef CONFIG_SPIRAM_MODE_QUAD
    #define LCD_PIXEL_CLOCK_HZ (6 * 1000 * 1000)
#else
    #define LCD_PIXEL_CLOCK_HZ (16 * 1000 * 1000)
#endif
#endif // ESP_DISPLAY_4INCH

#ifdef T_DISPLAY_S3
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_DMA
#define LCD_PIN_NUM_CS 6
#define LCD_PIN_NUM_RST 5
#define LCD_PIN_NUM_WR 8
#define LCD_PIN_NUM_RD 9
#define LCD_PIN_NUM_RS 7
#define LCD_PIN_NUM_D00 39
#define LCD_PIN_NUM_D01 40
#define LCD_PIN_NUM_D02 41
#define LCD_PIN_NUM_D03 42
#define LCD_PIN_NUM_D04 45
#define LCD_PIN_NUM_D05 46
#define LCD_PIN_NUM_D06 47
#define LCD_PIN_NUM_D07 48
#define LCD_PIN_NUM_BCKL 38
#define LCD_PANEL esp_lcd_new_panel_st7789
#define LCD_HRES 170
#define LCD_VRES 320
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_RGB
#define LCD_PIXEL_CLOCK_HZ (6528 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 35
#define LCD_MIRROR_X false
#define LCD_MIRROR_Y true
#define LCD_INVERT_COLOR true
#define LCD_SWAP_XY true
#endif // T_DISPLAY_S3

#ifdef S3_T_QT
#define LCD_SPI_HOST    SPI3_HOST
#define LCD_DMA
#define LCD_BCKL_ON_LEVEL 0
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_MOSI 2
#define LCD_PIN_NUM_CLK 3
#define LCD_PIN_NUM_CS 5
#define LCD_PIN_NUM_DC 6
#define LCD_PIN_NUM_RST 1
#define LCD_PIN_NUM_BCKL 10
#define LCD_PANEL esp_lcd_new_panel_st7789
#define LCD_HRES 128
#define LCD_VRES 128
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_GAP_X 2
#define LCD_GAP_Y 1
#define LCD_MIRROR_X true
#define LCD_MIRROR_Y true
#define LCD_INVERT_COLOR true
#define LCD_SWAP_XY false
#endif // S3_T_QT

#ifdef M5STACK_S3_ATOM
#define LCD_SPI_HOST    SPI3_HOST
#define LCD_DMA
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_MOSI 21
#define LCD_PIN_NUM_CLK 17
#define LCD_PIN_NUM_CS 15
#define LCD_PIN_NUM_DC 33
#define LCD_PIN_NUM_RST 34
#define LCD_PIN_NUM_BCKL 16
#define LCD_PANEL esp_lcd_new_panel_st7789
#define LCD_HRES 128
#define LCD_VRES 128
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_GAP_X 2
#define LCD_GAP_Y 1
#define LCD_MIRROR_X true
#define LCD_MIRROR_Y true
#define LCD_INVERT_COLOR true
#define LCD_SWAP_XY false
#endif // M5STACK_S3_ATOM

#ifdef T_RGB
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL

#define LCD_PIN_NUM_DE 45
#define LCD_PIN_NUM_VSYNC 41
#define LCD_PIN_NUM_HSYNC 47
#define LCD_PIN_NUM_CLK 42
#define LCD_PIN_NUM_D00 21
#define LCD_PIN_NUM_D01 18
#define LCD_PIN_NUM_D02 17
#define LCD_PIN_NUM_D03 16
#define LCD_PIN_NUM_D04 15
#define LCD_PIN_NUM_D05 14
#define LCD_PIN_NUM_D06 13
#define LCD_PIN_NUM_D07 12
#define LCD_PIN_NUM_D08 11
#define LCD_PIN_NUM_D09 10
#define LCD_PIN_NUM_D10 9
#define LCD_PIN_NUM_D11 7
#define LCD_PIN_NUM_D12 6
#define LCD_PIN_NUM_D13 5
#define LCD_PIN_NUM_D14 3
#define LCD_PIN_NUM_D15 2
#define LCD_PIN_NUM_BCKL 46
#define LCD_HSYNC_POLARITY 0
#define LCD_HSYNC_FRONT_PORCH 50
#define LCD_HSYNC_PULSE_WIDTH 1
#define LCD_HSYNC_BACK_PORCH 30
#define LCD_VSYNC_POLARITY 0
#define LCD_VSYNC_FRONT_PORCH 20
#define LCD_VSYNC_PULSE_WIDTH 1
#define LCD_VSYNC_BACK_PORCH 30
#define LCD_CLK_IDLE_HIGH 0
#define LCD_DE_IDLE_HIGH 0
#define LCD_BIT_DEPTH 16
#define LCD_PANEL esp_lcd_new_panel_st7701_t_rgb
#define LCD_HRES 480
#define LCD_VRES 480
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_SWAP_COLOR_BYTES false
#ifdef CONFIG_SPIRAM_MODE_QUAD
    #define LCD_PIXEL_CLOCK_HZ (6 * 1000 * 1000)
#else
    #define LCD_PIXEL_CLOCK_HZ (8 * 1000 * 1000)
#endif
#endif // T_RGB

#ifdef SUNTON_7INCH
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_DE 41
#define LCD_PIN_NUM_VSYNC 40
#define LCD_PIN_NUM_HSYNC 39
#define LCD_PIN_NUM_CLK 42
#define LCD_PIN_NUM_D00 14
#define LCD_PIN_NUM_D01 21
#define LCD_PIN_NUM_D02 47
#define LCD_PIN_NUM_D03 48
#define LCD_PIN_NUM_D04 45
#define LCD_PIN_NUM_D05 9
#define LCD_PIN_NUM_D06 46
#define LCD_PIN_NUM_D07 3
#define LCD_PIN_NUM_D08 8
#define LCD_PIN_NUM_D09 16
#define LCD_PIN_NUM_D10 1
#define LCD_PIN_NUM_D11 15
#define LCD_PIN_NUM_D12 7
#define LCD_PIN_NUM_D13 6
#define LCD_PIN_NUM_D14 5
#define LCD_PIN_NUM_D15 4
#define LCD_PIN_NUM_BCKL 2
#define LCD_HSYNC_POLARITY 0
#define LCD_HSYNC_FRONT_PORCH 210
#define LCD_HSYNC_PULSE_WIDTH 30
#define LCD_HSYNC_BACK_PORCH 16
#define LCD_VSYNC_POLARITY 0
#define LCD_VSYNC_FRONT_PORCH 22
#define LCD_VSYNC_PULSE_WIDTH 13
#define LCD_VSYNC_BACK_PORCH 10
#define LCD_CLK_ACTIVE_NEG 1
#define LCD_CLK_IDLE_HIGH 0
#define LCD_DE_IDLE_HIGH 1
#define LCD_BIT_DEPTH 16
//#define LCD_PANEL esp_lcd_new_panel_st7701
#define LCD_HRES 800
#define LCD_VRES 480
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_BGR
#define LCD_SWAP_COLOR_BYTES false
#ifdef CONFIG_SPIRAM_MODE_QUAD
    #define LCD_PIXEL_CLOCK_HZ (6 * 1000 * 1000)
#else
    #define LCD_PIXEL_CLOCK_HZ (16 * 1000 * 1000)
#endif
#endif // SUNTON_7INCH

#ifdef HELTEC_WIFI_KIT_V2
#include <ssd1306_surface_adapter.hpp>
#define LCD_I2C_HOST    0
#define LCD_DMA
#define LCD_I2C_ADDR 0x3C
#define LCD_CONTROL_PHASE_BYTES 1
#define LCD_DC_BIT_OFFSET 6
#define LCD_BIT_DEPTH 1
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_SCL 15
#define LCD_PIN_NUM_SDA 4
#define LCD_PIN_NUM_RST 16
#define LCD_PANEL esp_lcd_new_panel_ssd1306
#define LCD_HRES 128
#define LCD_VRES 64
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_MONOCHROME
#define LCD_PIXEL_CLOCK_HZ (400 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 0
#define LCD_MIRROR_X true
#define LCD_MIRROR_Y true
#define LCD_INVERT_COLOR false
#define LCD_SWAP_XY false
#define LCD_FRAME_ADAPTER ssd1306_surface_adapter
#define LCD_Y_ALIGN 8
#endif // HELTEC_WIFI_KIT_V2

#ifdef ESP_USB_OTG
#define LCD_SPI_HOST    SPI2_HOST
#define LCD_DMA
#define LCD_BCKL_ON_LEVEL 1
#define LCD_BCKL_OFF_LEVEL !LCD_BCKL_ON_LEVEL
#define LCD_PIN_NUM_MOSI 7
#define LCD_PIN_NUM_CLK 6
#define LCD_PIN_NUM_CS 5
#define LCD_PIN_NUM_DC 4
#define LCD_PIN_NUM_RST 8
#define LCD_PIN_NUM_BCKL 9
#define LCD_PANEL esp_lcd_new_panel_st7789
#define LCD_HRES 240
#define LCD_VRES 240
#define LCD_COLOR_SPACE ESP_LCD_COLOR_SPACE_RGB
#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_GAP_X 0
#define LCD_GAP_Y 0
#define LCD_MIRROR_X false
#define LCD_MIRROR_Y false
#define LCD_INVERT_COLOR true
#define LCD_SWAP_XY false
#endif // ESP_USB_OTG

#ifndef LCD_WIDTH
#ifdef LCD_SWAP_XY
#if LCD_SWAP_XY
#define LCD_WIDTH LCD_VRES
#define LCD_HEIGHT LCD_HRES
#else
#define LCD_WIDTH LCD_HRES
#define LCD_HEIGHT LCD_VRES
#endif
#else
#define LCD_WIDTH LCD_HRES
#define LCD_HEIGHT LCD_VRES
#endif
#endif
#ifndef LCD_BIT_DEPTH
#define LCD_BIT_DEPTH 16
#endif
#ifndef LCD_X_ALIGN
#define LCD_X_ALIGN 1
#endif
#ifndef LCD_Y_ALIGN
#define LCD_Y_ALIGN 1
#endif
#ifndef LCD_FRAME_ADAPTER
#define LCD_FRAME_ADAPTER gfx::bitmap<gfx::rgb_pixel<LCD_BIT_DEPTH>>
#endif
#ifndef LCD_DC_BIT_OFFSET
#define LCD_DC_BIT_OFFSET 0
#endif
#endif // LCD_CONFIG_H