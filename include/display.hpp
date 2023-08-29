#pragma once
#if __has_include(<Arduino.h>)
#include <Arduino.h>
#endif
#include <gfx.hpp>
#ifndef E_PAPER
#ifdef ESP_PLATFORM
#include <lcd_config.h>
#endif
#endif
#ifdef WIO_TERMINAL
#include <tft_spi.hpp>
#include <ili9341.hpp>
using namespace arduino;
using bus_t = tft_spi_ex<3,LCD_SS_PIN,SPI_MODE0>;
using lcd_t = ili9341<LCD_DC,LCD_RESET,LCD_BACKLIGHT,bus_t,3,true,400,200>;
#define LCD_BIT_DEPTH 16
#define LCD_X_ALIGN 1
#define LCD_Y_ALIGN 1
#define LCD_WIDTH 320
#define LCD_HEIGHT 240
#define LCD_FRAME_ADAPTER gfx::bitmap<gfx::rgb_pixel<LCD_BIT_DEPTH>>
#endif // WIO_TERMINAL
#ifdef T5_4_7
#include <lilygot54in7.hpp>
#ifdef ARDUINO
extern arduino::lilygot54in7 epd;
#else
extern esp_idf::lilygot54in7 epd;
#endif
#define LCD_BIT_DEPTH 4
#define LCD_X_ALIGN 1
#define LCD_Y_ALIGN 1
#define LCD_WIDTH 960
#define LCD_HEIGHT 540
#define LCD_ROTATION 1
#define LCD_FRAME_ADAPTER gfx::bitmap<gfx::gsc_pixel<LCD_BIT_DEPTH>>
#endif // T5_4_7

#define LCD_TRANSFER_KB 64

// here we compute how many bytes are needed in theory to store the total screen.
constexpr static const size_t lcd_screen_total_size = gfx::bitmap<typename LCD_FRAME_ADAPTER::pixel_type>::sizeof_buffer(LCD_WIDTH,LCD_HEIGHT);
// define our transfer buffer(s) 
// For devices with no DMA we only use one buffer.
// Our total size is either LCD_TRANSFER_KB 
// Or the lcd_screen_total_size - whatever
// is smaller
// Note that in the case of DMA the memory
// is divided between two buffers.
#ifdef LCD_DMA
constexpr static const size_t lcd_buffer_size = (LCD_TRANSFER_KB*512)>lcd_screen_total_size?lcd_screen_total_size:(LCD_TRANSFER_KB*512);
extern uint8_t lcd_buffer1[];
extern uint8_t lcd_buffer2[];
#else
constexpr static const size_t lcd_buffer_size = (LCD_TRANSFER_KB*1024)>lcd_screen_total_size?lcd_screen_total_size:(LCD_TRANSFER_KB*1024);
extern uint8_t lcd_buffer1[];
static uint8_t* const lcd_buffer2 = nullptr;
#endif

#include <uix.hpp>

// declare the screen type
using screen_t = uix::screen_ex<LCD_FRAME_ADAPTER,LCD_X_ALIGN,LCD_Y_ALIGN>;

// the active screen pointer
extern screen_t* active_screen;

// initializes the display
extern void display_init();
// updates the display, redrawing as necessary
extern void display_update();
// switches the active screen
extern void display_screen(screen_t& new_screen);
