#pragma once
#include <Arduino.h>
#include <gfx.hpp>
#ifdef ESP_PLATFORM
#include <lcd_config.h>
#endif
#ifdef WIO_TERMINAL
#include <tft_spi.hpp>
#include <ili9341.hpp>
using namespace arduino;
using bus_t = tft_spi_ex<3,LCD_SS_PIN,SPI_MODE0>;
using lcd_t = ili9341<LCD_DC,LCD_RESET,LCD_BACKLIGHT,bus_t,3,true,400,200>;
#define LCD_DMA
#define LCD_BIT_DEPTH 16
#define LCD_X_ALIGN 1
#define LCD_Y_ALIGN 1
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define LCD_FRAME_ADAPTER gfx::bitmap<gfx::rgb_pixel<LCD_BIT_DEPTH>>
#endif
// define our transfer buffer(s) and initialize
// the main screen with it/them.
// for RGB interface screens we only use one
// because there is no DMA
#ifdef LCD_DMA
constexpr static const size_t lcd_buffer_size = 32*1024;
extern uint8_t lcd_buffer1[];
extern uint8_t lcd_buffer2[];
#else
constexpr static const size_t lcd_buffer_size = 32*1024*2;
extern uint8_t lcd_buffer1[];
static uint8_t* const lcd_buffer2 = nullptr;
#endif

#include <uix.hpp>

// declare the types for our controls and other things
using screen_t = uix::screen_ex<LCD_WIDTH,LCD_HEIGHT,
                            LCD_FRAME_ADAPTER,LCD_X_ALIGN,LCD_Y_ALIGN>;

extern screen_t* active_screen;

extern void display_init();
extern void display_update();
extern void display_screen(screen_t& new_screen);
