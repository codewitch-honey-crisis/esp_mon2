#ifndef LCD_INIT_H
#define LCD_INIT_H
#include "lcd_config.h"
#include <esp_lcd_panel_io.h>
#include <driver/i2c.h>
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
esp_err_t lcd_panel_draw_bitmap(int x1, int y1, int x2, int y2, void* bitmap);
#ifdef LCD_PIN_NUM_HSYNC
bool lcd_panel_init();
esp_err_t esp_lcd_new_panel_st7701();
#endif
#ifndef LCD_PIN_NUM_HSYNC
// global so it can be used after init
bool lcd_panel_init(size_t max_transfer_size, esp_lcd_panel_io_color_trans_done_cb_t done_callback);
#endif  // LCD_PIN_NUM_HSYNC
#ifdef __cplusplus
}
#endif  // __cplusplus



esp_lcd_panel_handle_t lcd_handle;
#ifdef LCD_IMPLEMENTATION
#include <string.h>

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>

// extra drivers:
#if __has_include(<esp_lcd_panel_ili9341.h>)
#include <esp_lcd_panel_ili9341.h>
#endif
#if __has_include(<esp_lcd_panel_ili9342.h>)
#include <esp_lcd_panel_ili9342.h>
#endif
#if __has_include(<esp_lcd_panel_ili9488.h>)
#include <esp_lcd_panel_ili9488.h>
#endif
#endif  // LCD_IMPLEMENTATION

#ifdef LCD_PIN_NUM_HSYNC
#if !defined(LCD_IMPLEMENTATION)
extern esp_lcd_panel_handle_t lcd_handle;
#else
#include <driver/gpio.h>
#include <esp_lcd_panel_interface.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_rom_gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <hal/gpio_hal.h>
#include <hal/gpio_ll.h>
#include <rom/gpio.h>
#include <soc/gpio_periph.h>
#include <soc/gpio_reg.h>
#include <soc/gpio_sig_map.h>
#include <soc/gpio_struct.h>

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
// currently requires an ESP32-S3 for RGB
#if __has_include(<esp32s3/rom/cache.h>)
#include <esp32s3/rom/cache.h>
extern int Cache_WriteBack_Addr(uint32_t addr, uint32_t size);
#endif
#include <driver/i2c.h>
#include <driver/rtc_io.h>
#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <esp_pm.h>
#include <esp_private/gdma.h>
#include <esp_timer.h>
#include <hal/dma_types.h>
#include <hal/gdma_ll.h>
#include <hal/lcd_hal.h>
#include <hal/lcd_ll.h>
#include <math.h>
#include <sdkconfig.h>
#include <soc/apb_ctrl_reg.h>
#include <soc/efuse_reg.h>
#include <soc/gdma_channel.h>
#include <soc/gdma_reg.h>
#include <soc/gdma_struct.h>
#include <soc/i2c_reg.h>
#include <soc/i2c_struct.h>
#include <soc/i2s_reg.h>
#include <soc/lcd_cam_reg.h>
#include <soc/lcd_cam_struct.h>
#include <soc/lcd_periph.h>
#include <soc/rtc.h>
#include <soc/soc.h>
#include <soc/spi_reg.h>
#include <string.h>

#if defined(SOC_GDMA_SUPPORTED)  // for C3/S3
#include <soc/gdma_reg.h>
// S3とC3で同じレジスタに異なる定義名がついているため、ここで統一;
#if !defined(DMA_OUT_PERI_SEL_CH0_REG)
#define DMA_OUT_PERI_SEL_CH0_REG GDMA_OUT_PERI_SEL_CH0_REG
#endif
#endif
#if !defined(REG_SPI_BASE)
// #define REG_SPI_BASE(i) (DR_REG_SPI0_BASE - (i) * 0x1000)
#define REG_SPI_BASE(i) (DR_REG_SPI2_BASE)
#endif
#endif

#endif  // LCD_PIN_NUM_HSYNC

#ifdef LCD_PIN_NUM_HSYNC
#if __has_include(<esp32s3/rom/cache.h>)
#include <esp32s3/rom/cache.h>
extern int Cache_WriteBack_Addr(uint32_t addr, uint32_t size);
#endif

typedef struct lcd_pin_backup {
    uint32_t _io_mux_gpio_reg;
    uint32_t _gpio_func_out_reg;
    gpio_num_t _pin_num;
} lcd_pin_backup_t;
static void lcd_init_pin_backup(gpio_num_t pin_num, lcd_pin_backup_t* in_out_pin_backup) {
    in_out_pin_backup->_io_mux_gpio_reg = *(uint32_t*)(GPIO_PIN_MUX_REG[pin_num]);
    in_out_pin_backup->_gpio_func_out_reg = *(uint32_t*)(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_num * 4));
    in_out_pin_backup->_pin_num = pin_num;
}
static void lcd_pin_backup_restore(lcd_pin_backup_t* pin_backup) {
    if ((uint32_t)pin_backup->_pin_num < GPIO_NUM_MAX) {
        *(uint32_t*)(GPIO_PIN_MUX_REG[pin_backup->_pin_num]) = pin_backup->_io_mux_gpio_reg;
        *(uint32_t*)(GPIO_FUNC0_OUT_SEL_CFG_REG + (pin_backup->_pin_num * 4)) = pin_backup->_gpio_func_out_reg;
    }
}
static inline volatile uint32_t* lcd_get_gpio_hi_reg(int_fast8_t pin) { return (pin & 32) ? &GPIO.out1_w1ts.val : &GPIO.out_w1ts; }
static inline volatile uint32_t* lcd_get_gpio_lo_reg(int_fast8_t pin) { return (pin & 32) ? &GPIO.out1_w1tc.val : &GPIO.out_w1tc; }
static inline void lcd_gpio_hi(int_fast8_t pin) {
    //printf("pin %d high\n",pin);
    if (pin >= 0) *lcd_get_gpio_hi_reg(pin) = 1 << (pin & 31);
}
static inline void lcd_gpio_lo(int_fast8_t pin) {
    //printf("pin %d low\n",pin);
    if (pin >= 0) *lcd_get_gpio_lo_reg(pin) = 1 << (pin & 31);
}
#ifdef LCD_PIN_NUM_SDA
static void lcd_rgb_write_swspi(uint32_t data, uint8_t bits) {
    uint_fast8_t mask = 1 << (bits - 1);
    do {
        lcd_gpio_lo(LCD_PIN_NUM_SCK);
        if (data & mask) {
            lcd_gpio_hi(LCD_PIN_NUM_SDA);
        } else {
            lcd_gpio_lo(LCD_PIN_NUM_SDA);
        }
        lcd_gpio_hi(LCD_PIN_NUM_SCK);
    } while (mask >>= 1);
}
static void lcd_rgb_write_command(uint32_t data, uint_fast8_t len) {
    do {
        lcd_rgb_write_swspi(data & 0xFF, 9);
        data >>= 8;
    } while (--len);
}

static void lcd_rgb_write_data(uint32_t data, uint_fast8_t len) {
    do {
        lcd_rgb_write_swspi(data | 0x100, 9);
        data >>= 8;
    } while (--len);
}

esp_err_t esp_lcd_new_panel_st7701() {
    int32_t pin_mosi = LCD_PIN_NUM_SDA;
    int32_t pin_sclk = LCD_PIN_NUM_SCK;
    if (pin_mosi >= 0 && pin_sclk >= 0) {
        lcd_pin_backup_t backup_pins[2];
        lcd_init_pin_backup((gpio_num_t)pin_mosi,&backup_pins[0]);
        lcd_init_pin_backup((gpio_num_t)pin_sclk,&backup_pins[1]);
        lcd_gpio_lo(pin_mosi);
        gpio_set_direction((gpio_num_t)pin_mosi, GPIO_MODE_OUTPUT);
        lcd_gpio_lo(pin_sclk);
        gpio_set_direction((gpio_num_t)pin_sclk, GPIO_MODE_OUTPUT);
#ifdef LCD_PIN_NUM_CS
#if LCD_PIN_NUM_CS >= 0
        int32_t pin_cs = LCD_PIN_NUM_CS;
        lcd_gpio_lo(pin_cs);
#endif
#endif
        lcd_rgb_write_command(0xFF, 1);
        lcd_rgb_write_data(0x77, 1);
        lcd_rgb_write_data(0x01, 1);
        lcd_rgb_write_data(0x00, 2);
        lcd_rgb_write_data(0x10, 1);

        // 0xC0 : LNSET : Display Line Setting
        lcd_rgb_write_command(0xC0, 1);
        uint32_t line1 = (LCD_VRES >> 3) + 1;
        uint32_t line2 = (LCD_VRES >> 1) & 3;
        lcd_rgb_write_data(line1 + (line2 ? 0x80 : 0x00), 1);
        lcd_rgb_write_data(line2, 1);

        // 0xC3 : RGBCTRL
        lcd_rgb_write_command(0xC3, 1);
        uint32_t rgbctrl = 0;
        if (LCD_DE_IDLE_HIGH) rgbctrl += 0x01;
        if (LCD_CLK_IDLE_HIGH) rgbctrl += 0x02;
        if (!LCD_HSYNC_POLARITY) rgbctrl += 0x04;
        if (!LCD_VSYNC_POLARITY) rgbctrl += 0x08;
        lcd_rgb_write_data(rgbctrl, 1);
        lcd_rgb_write_data(0x10, 1);
        lcd_rgb_write_data(0x08, 1);

        static uint8_t list0[] =
            {
                // Command2 BK0 SEL
                0xFF,
                5,
                0x77,
                0x01,
                0x00,
                0x00,
                0x10,

                0xC1,
                2,
                0x0D,
                0x02,
                0xC2,
                2,
                0x31,
                0x05,
                0xCD,
                1,
                0x08,

                // Positive Voltage Gamma Control
                0xB0,
                16,
                0x00,
                0x11,
                0x18,
                0x0E,
                0x11,
                0x06,
                0x07,
                0x08,
                0x07,
                0x22,
                0x04,
                0x12,
                0x0F,
                0xAA,
                0x31,
                0x18,

                // Negative Voltage Gamma Control
                0xB1,
                16,
                0x00,
                0x11,
                0x19,
                0x0E,
                0x12,
                0x07,
                0x08,
                0x08,
                0x08,
                0x22,
                0x04,
                0x11,
                0x11,
                0xA9,
                0x32,
                0x18,

                // Command2 BK1 SEL
                0xFF,
                5,
                0x77,
                0x01,
                0x00,
                0x00,
                0x11,

                0xB0,
                1,
                0x60,  // Vop=4.7375v
                0xB1,
                1,
                0x32,  // VCOM=32
                0xB2,
                1,
                0x07,  // VGH=15v
                0xB3,
                1,
                0x80,
                0xB5,
                1,
                0x49,  // VGL=-10.17v
                0xB7,
                1,
                0x85,
                0xB8,
                1,
                0x21,  // AVDD=6.6 & AVCL=-4.6
                0xC1,
                1,
                0x78,
                0xC2,
                1,
                0x78,

                0xE0,
                3,
                0x00,
                0x1B,
                0x02,

                0xE1,
                11,
                0x08,
                0xA0,
                0x00,
                0x00,
                0x07,
                0xA0,
                0x00,
                0x00,
                0x00,
                0x44,
                0x44,
                0xE2,
                12,
                0x11,
                0x11,
                0x44,
                0x44,
                0xED,
                0xA0,
                0x00,
                0x00,
                0xEC,
                0xA0,
                0x00,
                0x00,

                0xE3,
                4,
                0x00,
                0x00,
                0x11,
                0x11,
                0xE4,
                2,
                0x44,
                0x44,

                0xE5,
                16,
                0x0A,
                0xE9,
                0xD8,
                0xA0,
                0x0C,
                0xEB,
                0xD8,
                0xA0,
                0x0E,
                0xED,
                0xD8,
                0xA0,
                0x10,
                0xEF,
                0xD8,
                0xA0,

                0xE6,
                4,
                0x00,
                0x00,
                0x11,
                0x11,

                0xE7,
                2,
                0x44,
                0x44,

                0xE8,
                16,
                0x09,
                0xE8,
                0xD8,
                0xA0,
                0x0B,
                0xEA,
                0xD8,
                0xA0,
                0x0D,
                0xEC,
                0xD8,
                0xA0,
                0x0F,
                0xEE,
                0xD8,
                0xA0,

                0xEB,
                7,
                0x02,
                0x00,
                0xE4,
                0xE4,
                0x88,
                0x00,
                0x40,
                0xEC,
                2,
                0x3C,
                0x00,
                0xED,
                16,
                0xAB,
                0x89,
                0x76,
                0x54,
                0x02,
                0xFF,
                0xFF,
                0xFF,
                0xFF,
                0xFF,
                0xFF,
                0x20,
                0x45,
                0x67,
                0x98,
                0xBA,

                //-----------VAP & VAN---------------
                // Command2 BK3 SEL
                0xFF,
                5,
                0x77,
                0x01,
                0x00,
                0x00,
                0x13,

                0xE5,
                1,
                0xE4,

                // Command2 BK0 SEL
                0xFF,
                5,
                0x77,
                0x01,
                0x00,
                0x00,
                0x00,

                0x21,
                0,  // 0x20 normal, 0x21 IPS
                0x3A,
                1,
                0x60,  // 0x70 RGB888, 0x60 RGB666, 0x50 RGB565

                0x11,
                128,
                120,  // Sleep Out

                0x29,
                0,  // Display On

                0xFF,
                0xFF,
            };
        const uint8_t* addr = list0;
        for (;;) {  // For each command...
            uint8_t cmd = *addr++;
            uint8_t num = *addr++;  // Number of args to follow
            if (cmd == 0xFF && num == 0xFF) break;
            lcd_rgb_write_command(cmd, 1);  // Read, issue command
            uint_fast8_t ms = num & 128;    // If hibit set, delay follows args
            num &= ~128;                    // Mask out delay bit
            if (num) {
                do {                                 // For each argument...
                    lcd_rgb_write_data(*addr++, 1);  // Read, issue argument
                } while (--num);
            }
            if (ms) {
                ms = *addr++;  // Read post-command delay time (ms)
                vTaskDelay(pdMS_TO_TICKS(ms == 255 ? 500 : ms));
            }
        }
        for (int i = 0; i < sizeof(backup_pins) / sizeof(backup_pins[0]);++i) {
            lcd_pin_backup_restore(&backup_pins[i]);
        }
    }
#ifdef LCD_PIN_NUM_CS
#if LCD_PIN_NUM_CS >= 0
        int32_t pin_cs = LCD_PIN_NUM_CS;
        lcd_gpio_hi(pin_cs);
#endif
#endif
    return ESP_OK;
}
#endif // LCD_PIN_NUM_SDA
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)

/* Portions derived from LovyanGFX
LovyanGFX ORIGINAL LIBRARY LICENSE:
Software License Agreement (FreeBSD License)

Copyright (c) 2020 lovyan03 (https://github.com/lovyan03)

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/
#ifdef LCD_PIN_NUM_HSYNC
static __attribute__((always_inline)) inline volatile uint32_t* lcd_reg(uint32_t addr) { return (volatile uint32_t*)ETS_UNCACHED_ADDR(addr); }

static uint8_t** lcd_lines_buffer = NULL;

static dma_descriptor_t lcd_dmadesc_restart;
static dma_descriptor_t* lcd_dmadesc = NULL;
static esp_lcd_i80_bus_handle_t lcd_i80_bus = NULL;
static int32_t lcd_dma_ch;

static uint8_t* lcd_frame_buffer = NULL;
static intr_handle_t lcd_intr_handle;
static void lcd_default_isr_handler(void* args) {
    auto dev = &LCD_CAM;

    uint32_t intr_status = dev->lc_dma_int_st.val & 0x03;
    dev->lc_dma_int_clr.val = intr_status;
    if (intr_status & LCD_LL_EVENT_VSYNC_END) {
        GDMA.channel[lcd_dma_ch].out.conf0.out_rst = 1;
        GDMA.channel[lcd_dma_ch].out.conf0.out_rst = 0;
        GDMA.channel[lcd_dma_ch].out.link.addr = (uintptr_t) & (lcd_dmadesc_restart);
        GDMA.channel[lcd_dma_ch].out.link.start = 1;

        // bool need_yield = false;
        // call user registered callback
        // if (rgb_panel->on_vsync) {
        //     if (rgb_panel->on_vsync(&rgb_panel->base, NULL, rgb_panel->user_ctx)) {
        //         need_yield = true;
        //     }
        // }

        // check whether to update the PCLK frequency, it should be safe to update the PCLK frequency in the VSYNC interrupt
        // lcd_rgb_panel_try_update_pclk(rgb_panel);

        // if (need_yield) {
        //     portYIELD_FROM_ISR();
        // }
    }
}
static void lcd_gpio_pin_sig(uint32_t pin, uint32_t sig) {
    gpio_hal_iomux_func_sel(GPIO_PIN_MUX_REG[pin], PIN_FUNC_GPIO);
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    esp_rom_gpio_connect_out_signal(pin, sig, false, false);
}
static inline void* lcd_heap_alloc(size_t length) { return heap_caps_malloc(length, MALLOC_CAP_8BIT); }
static inline void* lcd_heap_alloc_dma(size_t length) { return heap_caps_malloc((length + 3) & ~3, MALLOC_CAP_DMA); }
static inline void* lcd_heap_alloc_psram(size_t length) { return heap_caps_malloc((length + 3) & ~3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); }
static inline void lcd_heap_free(void* buf) { heap_caps_free(buf); }
static int min_uint32(uint32_t lhs,uint32_t rhs) {
    return lhs<=rhs?lhs:rhs;
}
static int max_uint32(uint32_t lhs,uint32_t rhs) {
    return lhs>rhs?lhs:rhs;
}
static void lcd_calc_clock_div(uint32_t* div_a, uint32_t* div_b, uint32_t* div_n, uint32_t* clkcnt, uint32_t baseClock, uint32_t targetFreq) {
    uint32_t diff = INT32_MAX;
    *div_n = 256;
    *div_a = 63;
    *div_b = 62;
    *clkcnt = 64;
    uint32_t start_cnt = min_uint32(64u, (baseClock / (targetFreq * 2) + 1));
    uint32_t end_cnt = max_uint32(2u, baseClock / 256u / targetFreq);
    if (start_cnt <= 2) {
        end_cnt = 1;
    }
    for (uint32_t cnt = start_cnt; diff && cnt >= end_cnt; --cnt) {
        float fdiv = (float)baseClock / cnt / targetFreq;
        uint32_t n = max_uint32(2u, (uint32_t)fdiv);
        fdiv -= n;

        for (uint32_t a = 63; diff && a > 0; --a) {
            uint32_t b = roundf(fdiv * a);
            if (a == b && n == 256) {
                break;
            }
            uint32_t freq = baseClock / ((n * cnt) + (float)(b * cnt) / (float)a);
            uint32_t d = abs((int)targetFreq - (int)freq);
            if (diff <= d) {
                continue;
            }
            diff = d;
            *clkcnt = cnt;
            *div_n = n;
            *div_b = b;
            *div_a = a;
            if (b == 0 || a == b) {
                break;
            }
        }
    }
    if (*div_a == *div_b) {
        *div_b = 0;
        *div_n += 1;
    }
}

int32_t lcd_search_dma_out_ch(int peripheral_select) {
#if defined(SOC_GDMA_SUPPORTED)  // for ESP32S3 / ESP32C3
    // ESP32C3: SPI2==0
    // ESP32S3: SPI2==0 / SPI3==1
    // SOC_GDMA_TRIG_PERIPH_SPI3
    // SOC_GDMA_TRIG_PERIPH_LCD0
    // GDMAペリフェラルレジスタの配列を順に調べてペリフェラル番号が一致するDMAチャンネルを特定する;
    for (int i = 0; i < SOC_GDMA_PAIRS_PER_GROUP; ++i) {
        // ESP_LOGD("DBG","GDMA.channel:%d peri_sel:%d", i, GDMA.channel[i].out.peri_sel.sel);
        if ((*lcd_reg(DMA_OUT_PERI_SEL_CH0_REG + i * 0xC0) & 0x3F) == peripheral_select) {
            // ESP_LOGD("DBG","GDMA.channel:%d hit", i);
            return i;
        }
    }
#endif
    return -1;
}

esp_err_t lcd_panel_draw_bitmap(int x1, int y1, int x2, int y2, void* color_data) {
    if (lcd_lines_buffer == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (color_data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    int tmp;
    if (x1 > x2) {
        tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y1 > y2) {
        tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    int cx1 = x1;
    if (cx1 < 0) {
        cx1 = 0;
    } else if (cx1 >= LCD_HRES) {
        cx1 = LCD_HRES - 1;
    }
    int cx2 = x2;
    if (cx2 < 0) {
        cx2 = 0;
    } else if (cx2 >= LCD_HRES) {
        cx2 = LCD_HRES - 1;
    }
    int cy1 = y1;
    if (cy1 < 0) {
        cy1 = 0;
    } else if (cy1 >= LCD_VRES) {
        cy1 = LCD_VRES - 1;
    }
    int cy2 = y2;
    if (cy2 < 0) {
        cy2 = 0;
    } else if (cy2 >= LCD_VRES) {
        cy2 = LCD_VRES - 1;
    }
    int bmp_offs_x = cx1 - x1, bmp_offs_y = cy1 - y1;
    int bmp_w = cx2 - cx1 + 1, bmp_h = cy2 - cy1 + 1;
    size_t bmp_len = bmp_w * (LCD_BIT_DEPTH >> 3);
    size_t bmp_offs = (bmp_offs_x * (LCD_BIT_DEPTH >> 3));
    size_t scr_offs = (cx1 * (LCD_BIT_DEPTH >> 3));
    size_t bmp_stride = (x2 - x1 + 1) * (LCD_BIT_DEPTH >> 3);
    uint8_t* src = (uint8_t*)color_data + bmp_offs_y;
    for (int y = 0; y < bmp_h; ++y) {
        uint8_t* dst = (uint8_t*)lcd_lines_buffer[y + cy1] + scr_offs;
        memcpy(dst, src + bmp_offs, bmp_len);
#if __has_include(<esp32s3/rom/cache.h>)
        Cache_WriteBack_Addr((uint32_t)dst, bmp_len);
#endif
        src += bmp_stride;
    }
    return ESP_OK;
}
bool lcd_panel_init() {
#ifdef LCD_PIN_NUM_BCKL
#if LCD_PIN_NUM_BCKL >= 0
    gpio_set_direction((gpio_num_t)LCD_PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
#endif
#endif  // PIN_NUM_BCKL
#ifdef LCD_PIN_NUM_CS
#if LCD_PIN_NUM_CS >= 0
    lcd_gpio_hi(LCD_PIN_NUM_CS);
    gpio_set_direction((gpio_num_t)LCD_PIN_NUM_CS, GPIO_MODE_OUTPUT);
#endif
#endif
    // dummy settings.
    esp_lcd_i80_bus_config_t bus_config;
    memset(&bus_config, 0, sizeof(esp_lcd_i80_bus_config_t));
    // bus_config.dc_gpio_num = GPIO_NUM_NC;
    bus_config.dc_gpio_num = LCD_PIN_NUM_VSYNC;
    bus_config.wr_gpio_num = LCD_PIN_NUM_CLK;
    bus_config.clk_src = lcd_clock_source_t::LCD_CLK_SRC_PLL160M;
    bus_config.data_gpio_nums[0] = LCD_PIN_NUM_D00;
    bus_config.data_gpio_nums[1] = LCD_PIN_NUM_D01;
    bus_config.data_gpio_nums[2] = LCD_PIN_NUM_D02;
    bus_config.data_gpio_nums[3] = LCD_PIN_NUM_D03;
    bus_config.data_gpio_nums[4] = LCD_PIN_NUM_D04;
    bus_config.data_gpio_nums[5] = LCD_PIN_NUM_D05;
    bus_config.data_gpio_nums[6] = LCD_PIN_NUM_D06;
    bus_config.data_gpio_nums[7] = LCD_PIN_NUM_D07;
#ifdef LCD_PIN_NUM_D15
    bus_config.data_gpio_nums[8] = LCD_PIN_NUM_D08;
    bus_config.data_gpio_nums[9] = LCD_PIN_NUM_D09;
    bus_config.data_gpio_nums[10] = LCD_PIN_NUM_D10;
    bus_config.data_gpio_nums[11] = LCD_PIN_NUM_D11;
    bus_config.data_gpio_nums[12] = LCD_PIN_NUM_D12;
    bus_config.data_gpio_nums[13] = LCD_PIN_NUM_D13;
    bus_config.data_gpio_nums[14] = LCD_PIN_NUM_D14;
    bus_config.data_gpio_nums[15] = LCD_PIN_NUM_D15;
    bus_config.bus_width = 16;
#else
    bus_config.bus_width = 8;
#endif  // LCD_PIN_NUM_D15

    bus_config.max_transfer_bytes = 4092;

    if (ESP_OK != esp_lcd_new_i80_bus(&bus_config, &lcd_i80_bus)) {
        return false;
    }
    uint8_t pixel_bytes = (LCD_BIT_DEPTH & 0xFF) >> 3;
    auto dev = &LCD_CAM;  // getDev(_cfg.port);

    {
        static constexpr const uint8_t rgb332sig_tbl[] = {1, 0, 1, 0, 1, 2, 3, 4, 2, 3, 4, 5, 6, 5, 6, 7};
        static constexpr const uint8_t rgb565sig_tbl[] = {8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7};
        auto tbl = (pixel_bytes == 2) ? rgb565sig_tbl : rgb332sig_tbl;
        auto sigs = lcd_periph_signals.panels[0];
        for (size_t i = 0; i < 16; i++) {
            lcd_gpio_pin_sig(bus_config.data_gpio_nums[i], sigs.data_sigs[tbl[i]]);
        }
        lcd_gpio_pin_sig(LCD_PIN_NUM_DE, sigs.de_sig);
        lcd_gpio_pin_sig(LCD_PIN_NUM_HSYNC, sigs.hsync_sig);
        lcd_gpio_pin_sig(LCD_PIN_NUM_VSYNC, sigs.vsync_sig);
        lcd_gpio_pin_sig(LCD_PIN_NUM_CLK, sigs.pclk_sig);
    }

    // periph_module_enable(lcd_periph_signals.panels[_cfg.port].module);
    lcd_dma_ch = lcd_search_dma_out_ch(SOC_GDMA_TRIG_PERIPH_LCD0);
    if (lcd_dma_ch < 0) {
        esp_lcd_del_i80_bus(lcd_i80_bus);
        ESP_LOGE("lcd_panel_init", "DMA channel not found.");
        return false;
    }

    GDMA.channel[lcd_dma_ch].out.peri_sel.sel = SOC_GDMA_TRIG_PERIPH_LCD0;

    typeof(GDMA.channel[0].out.conf0) conf0;
    conf0.val = 0;
    conf0.out_eof_mode = 1;
    conf0.outdscr_burst_en = 1;
    conf0.out_data_burst_en = 1;
    GDMA.channel[lcd_dma_ch].out.conf0.val = conf0.val;

    typeof(GDMA.channel[0].out.conf1) conf1;
    conf1.val = 0;
    conf1.out_ext_mem_bk_size = GDMA_LL_EXT_MEM_BK_SIZE_64B;
    GDMA.channel[lcd_dma_ch].out.conf1.val = conf1.val;

    size_t fb_len = (LCD_HRES * pixel_bytes) * LCD_VRES;
    auto data = (uint8_t*)lcd_heap_alloc_psram(fb_len);
    lcd_frame_buffer = data;
    if(lcd_frame_buffer==nullptr) {
        ESP_LOGE("lcd_panel_init","out of memory (PSRAM)\n");
        return false;
    }
    static constexpr size_t MAX_DMA_LEN = (4096 - 64);
    size_t dmadesc_size = (fb_len - 1) / MAX_DMA_LEN + 1;
    auto dmadesc = (dma_descriptor_t*)heap_caps_malloc(sizeof(dma_descriptor_t) * dmadesc_size, MALLOC_CAP_DMA);
    lcd_dmadesc = dmadesc;

    size_t len = fb_len;
    while (len > MAX_DMA_LEN) {
        len -= MAX_DMA_LEN;
        dmadesc->buffer = (uint8_t*)data;
        data += MAX_DMA_LEN;
        *(uint32_t*)dmadesc = MAX_DMA_LEN | MAX_DMA_LEN << 12 | 0x80000000;
        dmadesc->next = dmadesc + 1;
        ++dmadesc;
    }
    *(uint32_t*)dmadesc = ((len + 3) & (~3)) | len << 12 | 0xC0000000;
    dmadesc->buffer = (uint8_t*)data;
    dmadesc->next = lcd_dmadesc;
    GDMA.channel[lcd_dma_ch].out.link.addr = (uintptr_t) & (lcd_dmadesc);
    GDMA.channel[lcd_dma_ch].out.link.start = 1;
    //////////////////////////////////////////////

    memcpy(&lcd_dmadesc_restart, lcd_dmadesc, sizeof(lcd_dmadesc_restart));
    int skip_bytes = (GDMA_LL_L2FIFO_BASE_SIZE + 1) * pixel_bytes;
    auto p = (uint8_t*)(lcd_dmadesc_restart.buffer);
    lcd_dmadesc_restart.buffer = &p[skip_bytes];
    lcd_dmadesc_restart.dw0.length -= skip_bytes;
    lcd_dmadesc_restart.dw0.size -= skip_bytes;

    uint32_t hsw = LCD_HSYNC_PULSE_WIDTH;
    uint32_t hbp = LCD_HSYNC_BACK_PORCH;
    uint32_t active_width = LCD_HRES;
    uint32_t hfp = LCD_HSYNC_FRONT_PORCH;

    uint32_t vsw = LCD_VSYNC_PULSE_WIDTH;
    uint32_t vbp = LCD_VSYNC_BACK_PORCH;
    uint32_t vfp = LCD_VSYNC_FRONT_PORCH;
    uint32_t active_height = LCD_VRES;

    uint32_t div_a, div_b, div_n, clkcnt;
    lcd_calc_clock_div(&div_a, &div_b, &div_n, &clkcnt, 240 * 1000 * 1000, LCD_PIXEL_CLOCK_HZ < 40000000u ? LCD_PIXEL_CLOCK_HZ : 40000000u);
    typeof(dev->lcd_clock) lcd_clock;
    lcd_clock.lcd_clkcnt_n = max_uint32(1u, clkcnt - 1);
    lcd_clock.lcd_clk_equ_sysclk = (clkcnt == 1);
    lcd_clock.lcd_ck_idle_edge = false;
    lcd_clock.lcd_ck_out_edge = LCD_CLK_IDLE_HIGH;
    lcd_clock.lcd_clkm_div_num = div_n;
    lcd_clock.lcd_clkm_div_b = div_b;
    lcd_clock.lcd_clkm_div_a = div_a;
    lcd_clock.lcd_clk_sel = 2;  // clock_select: 1=XTAL CLOCK / 2=240MHz / 3=160MHz
    lcd_clock.clk_en = true;
    dev->lcd_clock.val = lcd_clock.val;

    typeof(dev->lcd_user) lcd_user;
    lcd_user.val = 0;
    // lcd_user.lcd_dout_cyclelen = 0;
    lcd_user.lcd_always_out_en = true;
    // lcd_user.lcd_8bits_order = false;
    // lcd_user.lcd_update = false;
    // lcd_user.lcd_bit_order = false;
    // lcd_user.lcd_byte_order = false;
    lcd_user.lcd_2byte_en = pixel_bytes > 1;  // RGB565 or RGB332
    lcd_user.lcd_dout = 1;
    // lcd_user.lcd_dummy = 0;
    // lcd_user.lcd_cmd = 0;
    lcd_user.lcd_update = 1;
    lcd_user.lcd_reset = 1;  // self clear
    // lcd_user.lcd_reset = 0;
    lcd_user.lcd_dummy_cyclelen = 3;
    // lcd_user.lcd_cmd_2_cycle_en = 0;
    dev->lcd_user.val = lcd_user.val;

    typeof(dev->lcd_misc) lcd_misc;
    lcd_misc.val = 0;
    lcd_misc.lcd_afifo_reset = true;
    lcd_misc.lcd_next_frame_en = true;
    lcd_misc.lcd_bk_en = true;
    // lcd_misc.lcd_vfk_cyclelen = 0;
    // lcd_misc.lcd_vbk_cyclelen = 0;
    dev->lcd_misc.val = lcd_misc.val;

    typeof(dev->lcd_ctrl) lcd_ctrl;
    lcd_ctrl.lcd_hb_front = hbp + hsw - 1;
    lcd_ctrl.lcd_va_height = active_height - 1;
    lcd_ctrl.lcd_vt_height = vsw + vbp + active_height + vfp - 1;
    lcd_ctrl.lcd_rgb_mode_en = true;
    dev->lcd_ctrl.val = lcd_ctrl.val;

    typeof(dev->lcd_ctrl1) lcd_ctrl1;
    lcd_ctrl1.lcd_vb_front = vbp + vsw - 1;
    lcd_ctrl1.lcd_ha_width = active_width - 1;
    lcd_ctrl1.lcd_ht_width = hsw + hbp + active_width + hfp - 1;
    dev->lcd_ctrl1.val = lcd_ctrl1.val;

    typeof(dev->lcd_ctrl2) lcd_ctrl2;
    lcd_ctrl2.val = 0;
    lcd_ctrl2.lcd_vsync_width = vsw - 1;
    lcd_ctrl2.lcd_vsync_idle_pol = LCD_VSYNC_POLARITY;
    lcd_ctrl2.lcd_hs_blank_en = true;
    lcd_ctrl2.lcd_hsync_width = hsw - 1;
    lcd_ctrl2.lcd_hsync_idle_pol = LCD_HSYNC_POLARITY;
    // lcd_ctrl2.lcd_hsync_position = 0;
    lcd_ctrl2.lcd_de_idle_pol = LCD_DE_IDLE_HIGH;
    dev->lcd_ctrl2.val = lcd_ctrl2.val;

    dev->lc_dma_int_ena.val = 1;

    int isr_flags = ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_SHARED;
    esp_intr_alloc_intrstatus(lcd_periph_signals.panels[0].irq_id, isr_flags,
                              (uint32_t)&dev->lc_dma_int_st,
                              LCD_LL_EVENT_VSYNC_END, lcd_default_isr_handler, NULL, &lcd_intr_handle);
    esp_intr_enable(lcd_intr_handle);

    dev->lcd_user.lcd_update = 1;
    dev->lcd_user.lcd_start = 1;

#ifdef LCD_PIN_NUM_RST
#if LCD_PIN_NUM_RST >= 0
    lcd_gpio_hi(LCD_PIN_NUM_RST);
    gpio_set_direction((gpio_num_t)LCD_PIN_NUM_RST, GPIO_MODE_OUTPUT);
    lcd_gpio_hi(LCD_PIN_NUM_RST);
    vTaskDelay(pdMS_TO_TICKS(64));
    lcd_gpio_lo(LCD_PIN_NUM_RST);
    vTaskDelay(pdMS_TO_TICKS(4));
    lcd_gpio_hi(LCD_PIN_NUM_RST);
    vTaskDelay(pdMS_TO_TICKS(64));
#endif
#endif
    auto h = LCD_VRES;

    size_t lineArray_size = h * sizeof(void*);
    uint8_t** lineArray = (uint8_t**)lcd_heap_alloc_dma(lineArray_size);

    if (lineArray) {
        lcd_lines_buffer = lineArray;
        memset(lineArray, 0, lineArray_size);

        uint8_t bits = LCD_BIT_DEPTH;
        int w = (LCD_HRES + 3) & ~3;
        if (lcd_frame_buffer) {
            auto fb = lcd_frame_buffer;
            for (int i = 0; i < h; ++i) {
                lineArray[i] = fb;
                fb += w * bits >> 3;
            }
#ifdef LCD_PIN_NUM_CS
#if LCD_PIN_NUM_CS >= 0
            lcd_gpio_hi(LCD_PIN_NUM_CS);
            gpio_set_direction((gpio_num_t)LCD_PIN_NUM_CS, GPIO_MODE_OUTPUT);
#endif
#endif

#ifdef LCD_PANEL
            return ESP_OK == LCD_PANEL();
#else
            return ESP_OK;
#endif
        }
        printf("out of memory\n");
        lcd_heap_free(lineArray);
    }

    return false;
}
#endif  // LCD_PIN_NUM_HSYNC
#else
#include "esp_private/gdma.h"
#include "esp_pm.h"
#include "hal/dma_types.h"

#include "hal/lcd_hal.h"
#include "hal/lcd_ll.h"
// extract from esp-idf esp_lcd_rgb_panel.c
struct esp_rgb_panel_t
{
  esp_lcd_panel_t base;                                        // Base class of generic lcd panel
  int panel_id;                                                // LCD panel ID
  lcd_hal_context_t hal;                                       // Hal layer object
  size_t data_width;                                           // Number of data lines (e.g. for RGB565, the data width is 16)
  size_t sram_trans_align;                                     // Alignment for framebuffer that allocated in SRAM
  size_t psram_trans_align;                                    // Alignment for framebuffer that allocated in PSRAM
  int disp_gpio_num;                                           // Display control GPIO, which is used to perform action like "disp_off"
  intr_handle_t intr;                                          // LCD peripheral interrupt handle
  esp_pm_lock_handle_t pm_lock;                                // Power management lock
  size_t num_dma_nodes;                                        // Number of DMA descriptors that used to carry the frame buffer
  uint8_t *fb;                                                 // Frame buffer
  size_t fb_size;                                              // Size of frame buffer
  int data_gpio_nums[SOC_LCD_RGB_DATA_WIDTH];                  // GPIOs used for data lines, we keep these GPIOs for action like "invert_color"
  size_t resolution_hz;                                        // Peripheral clock resolution
  esp_lcd_rgb_timing_t timings;                                // RGB timing parameters (e.g. pclk, sync pulse, porch width)
  gdma_channel_handle_t dma_chan;                              // DMA channel handle
  esp_lcd_rgb_panel_frame_trans_done_cb_t on_frame_trans_done; // Callback, invoked after frame trans done
  void *user_ctx;                                              // Reserved user's data of callback functions
  int x_gap;                                                   // Extra gap in x coordinate, it's used when calculate the flush window
  int y_gap;                                                   // Extra gap in y coordinate, it's used when calculate the flush window
  struct
  {
    unsigned int disp_en_level : 1; // The level which can turn on the screen by `disp_gpio_num`
    unsigned int stream_mode : 1;   // If set, the LCD transfers data continuously, otherwise, it stops refreshing the LCD when transaction done
    unsigned int fb_in_psram : 1;   // Whether the frame buffer is in PSRAM
  } flags;
  dma_descriptor_t dma_nodes[]; // DMA descriptor pool of size `num_dma_nodes`
};
uint8_t* lcd_frame_buffer;
size_t lcd_frame_buffer_size;
esp_err_t lcd_panel_draw_bitmap(int x1, int y1, int x2, int y2, void* bitmap) {
    esp_err_t result=esp_lcd_panel_draw_bitmap(lcd_handle, x1, y1, x2 + 1, y2 + 1, bitmap);
    if(ESP_OK==result) {
        Cache_WriteBack_Addr((uint32_t)lcd_frame_buffer,lcd_frame_buffer_size);
    }
    return result;
}

bool lcd_panel_init() {
#ifdef LCD_PIN_NUM_BCKL
#if LCD_PIN_NUM_BCKL >= 0
    gpio_config_t bk_gpio_config;
    bk_gpio_config.mode = GPIO_MODE_OUTPUT;
    bk_gpio_config.pin_bit_mask = 1ULL << LCD_PIN_NUM_BCKL;
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
#endif  // LCD_PIN_NUM_BCKL >= 0
#endif  // LCD_PIN_NUM_BCKL
#ifdef LCD_PIN_NUM_CS
#if LCD_PIN_NUM_CS >= 0
    gpio_set_direction((gpio_num_t)LCD_PIN_NUM_CS, GPIO_MODE_OUTPUT);
#endif
#endif
#ifdef LCD_PIN_NUM_SCK
#if LCD_PIN_NUM_SCK >= 0
    gpio_set_direction((gpio_num_t)LCD_PIN_NUM_SCK, GPIO_MODE_OUTPUT);

#endif
#endif
#ifdef LCD_PIN_NUM_SDA
#if LCD_PIN_NUM_SDA >= 0
    gpio_set_direction((gpio_num_t)LCD_PIN_NUM_SDA, GPIO_MODE_OUTPUT);
#endif
#endif
#ifdef LCD_PANEL
    LCD_PANEL();
#endif
    esp_lcd_rgb_panel_config_t panel_config;
    memset(&panel_config, 0, sizeof(panel_config));

    panel_config.data_width = 16;
    panel_config.psram_trans_align = 64;
    panel_config.num_fbs = 1;
    panel_config.clk_src = LCD_CLK_SRC_DEFAULT;
    panel_config.disp_gpio_num = -1;
    panel_config.pclk_gpio_num = LCD_PIN_NUM_CLK;
    panel_config.vsync_gpio_num = LCD_PIN_NUM_VSYNC;
    panel_config.hsync_gpio_num = LCD_PIN_NUM_HSYNC;
    panel_config.de_gpio_num = LCD_PIN_NUM_DE;
    panel_config.data_gpio_nums[0] = LCD_PIN_NUM_D00;
    panel_config.data_gpio_nums[1] = LCD_PIN_NUM_D01;
    panel_config.data_gpio_nums[2] = LCD_PIN_NUM_D02;
    panel_config.data_gpio_nums[3] = LCD_PIN_NUM_D03;
    panel_config.data_gpio_nums[4] = LCD_PIN_NUM_D04;
    panel_config.data_gpio_nums[5] = LCD_PIN_NUM_D05;
    panel_config.data_gpio_nums[6] = LCD_PIN_NUM_D06;
    panel_config.data_gpio_nums[7] = LCD_PIN_NUM_D07;
    panel_config.data_gpio_nums[8] = LCD_PIN_NUM_D08;
    panel_config.data_gpio_nums[9] = LCD_PIN_NUM_D09;
    panel_config.data_gpio_nums[10] = LCD_PIN_NUM_D10;
    panel_config.data_gpio_nums[11] = LCD_PIN_NUM_D11;
    panel_config.data_gpio_nums[12] = LCD_PIN_NUM_D12;
    panel_config.data_gpio_nums[13] = LCD_PIN_NUM_D13;
    panel_config.data_gpio_nums[14] = LCD_PIN_NUM_D14;
    panel_config.data_gpio_nums[15] = LCD_PIN_NUM_D15;

    panel_config.timings.pclk_hz = LCD_PIXEL_CLOCK_HZ;
    panel_config.timings.h_res = LCD_HRES;
    panel_config.timings.v_res = LCD_VRES;
    // The following parameters should refer to LCD spec
    panel_config.timings.hsync_back_porch = LCD_HSYNC_BACK_PORCH;
    panel_config.timings.hsync_front_porch = LCD_HSYNC_FRONT_PORCH;
    panel_config.timings.hsync_pulse_width = LCD_HSYNC_PULSE_WIDTH;
    panel_config.timings.vsync_back_porch = LCD_VSYNC_BACK_PORCH;
    panel_config.timings.vsync_front_porch = LCD_VSYNC_FRONT_PORCH;
    panel_config.timings.vsync_pulse_width = LCD_VSYNC_PULSE_WIDTH;
    // TODO: test the following
    panel_config.timings.flags.pclk_active_neg = true;
    panel_config.timings.flags.de_idle_high = LCD_DE_IDLE_HIGH;
    panel_config.timings.flags.hsync_idle_low = !LCD_HSYNC_POLARITY;
    panel_config.timings.flags.vsync_idle_low = !LCD_VSYNC_POLARITY;
    panel_config.timings.flags.pclk_idle_high = LCD_CLK_IDLE_HIGH;
    panel_config.flags.fb_in_psram = true;  // allocate frame buffer in PSRAM
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &lcd_handle));
    
    /*
    esp_lcd_rgb_panel_event_callbacks_t cbs = {
        .on_vsync = LCD_on_vsync_event,
    };
    ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, &disp_drv));
    */
    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_handle));
    esp_rgb_panel_t * rgb_panel = __containerof(lcd_handle, esp_rgb_panel_t, base);

    lcd_frame_buffer = (uint8_t*)rgb_panel->fb;
    lcd_frame_buffer_size = rgb_panel->fb_size;
    #ifdef LCD_PANEL
    ESP_ERROR_CHECK(LCD_PANEL());
    #endif
    
    esp_lcd_panel_disp_on_off(lcd_handle, true);

#if LCD_PIN_NUM_BK_LIGHT >= 0
    gpio_set_level((gpio_num_t)LCD_PIN_NUM_BCKL, LCD_BCKL_ON_LEVEL);
#endif
    return true;
}
#endif // ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#endif  // ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#endif  // LCD_IMPLEMENTATION
#ifdef LCD_IMPLEMENTATION
#ifndef LCD_PIN_NUM_HSYNC
// initialize the screen using the esp lcd panel API
bool lcd_panel_init(size_t max_transfer_size, esp_lcd_panel_io_color_trans_done_cb_t done_callback) {
#ifdef LCD_PIN_NUM_BCKL
    gpio_set_direction((gpio_num_t)LCD_PIN_NUM_BCKL, GPIO_MODE_OUTPUT);
#endif               // LCD_PIN_NUM_BCKL
#ifdef LCD_I2C_HOST // I2C
    i2c_config_t i2c_conf;
    memset(&i2c_conf,0,sizeof(i2c_config_t));
    
    i2c_conf.mode = I2C_MODE_MASTER,
    i2c_conf.sda_io_num = LCD_PIN_NUM_SDA;
    i2c_conf.scl_io_num = LCD_PIN_NUM_SCL;
    i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_conf.master.clk_speed = LCD_PIXEL_CLOCK_HZ;
    
    ESP_ERROR_CHECK(i2c_param_config(LCD_I2C_HOST, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(LCD_I2C_HOST, I2C_MODE_MASTER, 0, 0, 0));

   ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config;
    memset(&io_config,0,sizeof(esp_lcd_panel_io_i2c_config_t));
    io_config.dev_addr = LCD_I2C_ADDR;
#ifdef LCD_CONTROL_PHASE_BYTES
    io_config.control_phase_bytes = LCD_CONTROL_PHASE_BYTES;
#else
    io_config.control_phase_bytes = 0;
#endif
    io_config.lcd_cmd_bits = 8;   
    io_config.lcd_param_bits = 8; 
    io_config.on_color_trans_done = done_callback;
    io_config.dc_bit_offset = LCD_DC_BIT_OFFSET;  
#if defined(LCD_ENABLE_CONTROL_PHASE) && LCD_ENABLE_CONTROL_PHASE != 0
    io_config.flags.disable_control_phase = 1;
#endif
    esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)LCD_I2C_HOST, &io_config, &io_handle);
#elif defined(LCD_SPI_HOST)  // 1-bit SPI
    spi_bus_config_t bus_config;
    memset(&bus_config, 0, sizeof(bus_config));
    bus_config.sclk_io_num = LCD_PIN_NUM_CLK;
    bus_config.mosi_io_num = LCD_PIN_NUM_MOSI;
#ifdef LCD_PIN_NUM_MISO
    bus_config.miso_io_num = LCD_PIN_NUM_MISO;
#else
    bus_config.miso_io_num = -1;
#endif  // LCD_PIN_NUM_MISO
#ifdef LCD_PIN_NUM_QUADWP
    bus_config.quadwp_io_num = LCD_PIN_NUM_QUADWP;
#else
    bus_config.quadwp_io_num = -1;
#endif
#ifdef LCD_PIN_NUM_QUADHD
    bus_config.quadhd_io_num = LCD_PIN_NUM_QUADHD;
#else
    bus_config.quadhd_io_num = -1;
#endif
    bus_config.max_transfer_sz = max_transfer_size + 8;

    // Initialize the SPI bus on LCD_SPI_HOST
    spi_bus_initialize(LCD_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config;
    memset(&io_config, 0, sizeof(io_config));
    io_config.dc_gpio_num = LCD_PIN_NUM_DC,
    io_config.cs_gpio_num = LCD_PIN_NUM_CS,
    io_config.pclk_hz = LCD_PIXEL_CLOCK_HZ,
    io_config.lcd_cmd_bits = 8,
    io_config.lcd_param_bits = 8,
    io_config.spi_mode = 0,
    io_config.trans_queue_depth = 10,
    io_config.on_color_trans_done = done_callback;
    // Attach the LCD to the SPI bus
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST,
                             &io_config, &io_handle);
#elif defined(LCD_PIN_NUM_D07)  // 8 or 16-bit i8080
    gpio_set_direction((gpio_num_t)LCD_PIN_NUM_RD, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)LCD_PIN_NUM_RD, 1);
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config;
    memset(&bus_config, 0, sizeof(bus_config));
    bus_config.clk_src = LCD_CLK_SRC_PLL160M;
    bus_config.dc_gpio_num = LCD_PIN_NUM_RS;
    bus_config.wr_gpio_num = LCD_PIN_NUM_WR;
    bus_config.data_gpio_nums[0] = LCD_PIN_NUM_D00;
    bus_config.data_gpio_nums[1] = LCD_PIN_NUM_D01;
    bus_config.data_gpio_nums[2] = LCD_PIN_NUM_D02;
    bus_config.data_gpio_nums[3] = LCD_PIN_NUM_D03;
    bus_config.data_gpio_nums[4] = LCD_PIN_NUM_D04;
    bus_config.data_gpio_nums[5] = LCD_PIN_NUM_D05;
    bus_config.data_gpio_nums[6] = LCD_PIN_NUM_D06;
    bus_config.data_gpio_nums[7] = LCD_PIN_NUM_D07;
#ifdef LCD_PIN_NUM_D15
    bus_config.data_gpio_nums[8] = LCD_PIN_NUM_D08;
    bus_config.data_gpio_nums[9] = LCD_PIN_NUM_D09;
    bus_config.data_gpio_nums[10] = LCD_PIN_NUM_D10;
    bus_config.data_gpio_nums[11] = LCD_PIN_NUM_D11;
    bus_config.data_gpio_nums[12] = LCD_PIN_NUM_D12;
    bus_config.data_gpio_nums[13] = LCD_PIN_NUM_D13;
    bus_config.data_gpio_nums[14] = LCD_PIN_NUM_D14;
    bus_config.data_gpio_nums[15] = LCD_PIN_NUM_D15;
    bus_config.bus_width = 16;
#else
    bus_config.bus_width = 8;
#endif  // LCD_PIN_NUM_D15
    bus_config.max_transfer_bytes = max_transfer_size;

    esp_lcd_new_i80_bus(&bus_config, &i80_bus);

    esp_lcd_panel_io_handle_t io_handle = NULL;

    esp_lcd_panel_io_i80_config_t io_config;
    memset(&io_config, 0, sizeof(io_config));
    io_config.cs_gpio_num = LCD_PIN_NUM_CS;
    io_config.pclk_hz = LCD_PIXEL_CLOCK_HZ;
    io_config.trans_queue_depth = 20;
    io_config.dc_levels.dc_idle_level = 0;
    io_config.dc_levels.dc_idle_level = 0;
    io_config.dc_levels.dc_cmd_level = 0;
    io_config.dc_levels.dc_dummy_level = 0;
    io_config.dc_levels.dc_data_level = 1;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    io_config.on_color_trans_done = done_callback;
    io_config.user_ctx = NULL;
#ifdef LCD_SWAP_COLOR_BYTES
    io_config.flags.swap_color_bytes = LCD_SWAP_COLOR_BYTES;
#else
    io_config.flags.swap_color_bytes = false;
#endif  // LCD_SWAP_COLOR_BYTES
    io_config.flags.cs_active_high = false;
    io_config.flags.reverse_color_bits = false;
    esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle);
#endif  // LCD_PIN_NUM_D15
    lcd_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config;
    memset(&panel_config, 0, sizeof(panel_config));
#ifdef LCD_PIN_NUM_RST
    panel_config.reset_gpio_num = LCD_PIN_NUM_RST;
#else
    panel_config.reset_gpio_num = -1;
#endif
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
if(((int)LCD_COLOR_SPACE) == 0) {
    panel_config.rgb_endian = LCD_RGB_ENDIAN_RGB;
} else {
    panel_config.rgb_endian = LCD_RGB_ENDIAN_BGR;
}
#else
    panel_config.color_space = LCD_COLOR_SPACE;
#endif
#ifndef LCD_BIT_DEPTH
    panel_config.bits_per_pixel = 16;
#else
    panel_config.bits_per_pixel = LCD_BIT_DEPTH;
#endif
    // Initialize the LCD configuration
    LCD_PANEL(io_handle, &panel_config, &lcd_handle);

#ifdef LCD_PIN_NUM_BCKL
    // Turn off backlight to avoid unpredictable display on
    // the LCD screen while initializing
    // the LCD panel driver. (Different LCD screens may need different levels)
    gpio_set_level((gpio_num_t)LCD_PIN_NUM_BCKL, LCD_BCKL_OFF_LEVEL);
#endif  // LCD_PIN_NUM_BCKL
    // Reset the display
    esp_lcd_panel_reset(lcd_handle);

    // Initialize LCD panel
    esp_lcd_panel_init(lcd_handle);

    esp_lcd_panel_swap_xy(lcd_handle, LCD_SWAP_XY);
    esp_lcd_panel_set_gap(lcd_handle, LCD_GAP_X, LCD_GAP_Y);
    esp_lcd_panel_mirror(lcd_handle, LCD_MIRROR_X, LCD_MIRROR_Y);
    esp_lcd_panel_invert_color(lcd_handle, LCD_INVERT_COLOR);
    // Turn on the screen
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    esp_lcd_panel_disp_on_off(lcd_handle, true);
#else
    esp_lcd_panel_disp_off(lcd_handle, false);
#endif

#ifdef LCD_PIN_NUM_BCKL
    // Turn on backlight (Different LCD screens may need different levels)
    gpio_set_level((gpio_num_t)LCD_PIN_NUM_BCKL, LCD_BCKL_ON_LEVEL);
#endif  // LCD_PIN_NUM_BCKL

    return true;
}
esp_err_t lcd_panel_draw_bitmap(int x1, int y1, int x2, int y2, void* bitmap) {
    return esp_lcd_panel_draw_bitmap(lcd_handle, x1, y1, x2 + 1, y2 + 1, bitmap);
}
#endif
#endif
#endif  // LCD_INIT_H