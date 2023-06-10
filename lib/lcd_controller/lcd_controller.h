#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize LCD
     *
     * @param[in] max_transfer_bytes Maximum transfer size by i80 controller
     * @return esp_err_t Initialize state
     */
    esp_err_t lcd_init(size_t max_transfer_bytes);

    /**
     * @brief Flush specified area of screen.
     *
     * @param x_start Horizontal coordinate of start point [0..Width - 1]
     * @param y_start Vertical coordinate of start point [0..Height - 1]
     * @param x_end Horizonal coordinate of end point [0..Width - 1]
     * @param y_end Vertical coordinate of end point [0..Height - 1]
     * @param color_map Pixel data pointer
     * @return esp_err_t
     *      ESP_OK : Flush successfully
     */
    esp_err_t lcd_flush(int16_t x1, int16_t y1, int16_t x2, int16_t y2, void *color_map);

    /**
     * @brief Register callback which is invoked when color data transfer has finished.
     *
     * @param[in] esp_lcd_panel_io_color_trans_done_cb_t Callback function.
     * @param[in] data user custom data
     * @return None
     */
    void lcd_color_trans_done_register_cb(esp_lcd_panel_io_color_trans_done_cb_t color_trans_done_cb, void *data);

#ifdef __cplusplus
}
#endif