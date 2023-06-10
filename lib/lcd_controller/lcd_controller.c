#include <stdio.h>
#include "lcd_pin_config.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ili9488.h"
#include "lcd_controller.h"

static esp_lcd_panel_handle_t panel_handle = NULL;
static void *user_ctx = NULL;
static esp_lcd_panel_io_color_trans_done_cb_t on_color_trans_done_cb = NULL;

void lcd_color_trans_done_register_cb(esp_lcd_panel_io_color_trans_done_cb_t color_trans_done_cb, void *data)
{
    on_color_trans_done_cb = color_trans_done_cb;
    user_ctx = data;
}

esp_err_t lcd_flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2, void *color_map)
{
    return esp_lcd_panel_draw_bitmap(panel_handle, x1, y1, x2+1 , y2+1 , color_map);
}

esp_err_t lcd_init(size_t max_transfer_bytes)
{
    gpio_config_t bk_cs_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = ((1ULL << CONFIG_LCD_BACK_LIGHT_GPIO) | (1ULL << CONFIG_LCD_CS_GPIO) | (1ULL << CONFIG_LCD_RD_GPIO))};

    gpio_config(&bk_cs_gpio_config);
    if(CONFIG_LCD_CS_GPIO!=-1) {
        gpio_set_level(CONFIG_LCD_CS_GPIO, 0);
    }
    gpio_set_level(CONFIG_LCD_RD_GPIO,1);
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_PLL160M,
        .dc_gpio_num = CONFIG_LCD_RS_GPIO,
        .wr_gpio_num = CONFIG_LCD_WR_GPIO,
        .data_gpio_nums = {
            CONFIG_LCD_D00_GPIO,
            CONFIG_LCD_D01_GPIO,
            CONFIG_LCD_D02_GPIO,
            CONFIG_LCD_D03_GPIO,
            CONFIG_LCD_D04_GPIO,
            CONFIG_LCD_D05_GPIO,
            CONFIG_LCD_D06_GPIO,
            CONFIG_LCD_D07_GPIO,
            CONFIG_LCD_D08_GPIO,
            CONFIG_LCD_D09_GPIO,
            CONFIG_LCD_D10_GPIO,
            CONFIG_LCD_D11_GPIO,
            CONFIG_LCD_D12_GPIO,
            CONFIG_LCD_D13_GPIO,
            CONFIG_LCD_D14_GPIO,
            CONFIG_LCD_D15_GPIO,
        },
        .bus_width = 16,
        .max_transfer_bytes = max_transfer_bytes,
    };
    
    esp_lcd_new_i80_bus(&bus_config, &i80_bus);

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = CONFIG_LCD_CS_GPIO,
        .pclk_hz = 4 * 1000 * 1000,
        .trans_queue_depth = 20,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        //.lcd_cmd_bits = 16,
        //.lcd_param_bits = 16,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .on_color_trans_done = on_color_trans_done_cb,
        .user_ctx = user_ctx,
        .flags = {
            .swap_color_bytes = true,
            .cs_active_high = false,
            .reverse_color_bits = false,
        },
    };
    esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle);

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_ili9488(io_handle, &panel_config, &panel_handle);

    static const uint8_t rotation = 3;

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, false);
    bool swap_xy, mirror_x,mirror_y;
    switch(rotation&3) {
        case 0:
            mirror_x = true;
            mirror_y = false;
            swap_xy  = false;
            break;
        case 1:
            mirror_x = false;
            mirror_y = false;
            swap_xy = true;
            break;
        case 2:
            mirror_x = false;
            mirror_y = true;
            swap_xy = false;
            break;
        case 3:
            mirror_x = true;
            mirror_y = true;
            swap_xy = true;
            break;
    }
    esp_lcd_panel_swap_xy(panel_handle,swap_xy);
    esp_lcd_panel_mirror(panel_handle,mirror_x,mirror_y);
    
    esp_lcd_panel_set_gap(panel_handle, 0, 0);
    esp_lcd_panel_disp_off(panel_handle, true);

    /* Turn on LCD backlight */
    gpio_set_level(CONFIG_LCD_BACK_LIGHT_GPIO, 1);

    
    
    return true;
}
