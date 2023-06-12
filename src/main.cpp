#include <Arduino.h>
#define LCD_IMPLEMENTATION
#include <lcd_init.h>
#include <uix.hpp>
using namespace gfx;
using namespace uix;
#include <ui.hpp>
#include <interface.hpp>

static char cpu_sz[32];
static char gpu_sz[32];

static uint32_t timeout_ts = 0;

#ifndef LCD_PIN_NUM_VSYNC
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t* edata, void* user_ctx) {
    main_screen.set_flush_complete();
    return true;
}
#endif

static void uix_flush(point16 location, typename screen_t::bitmap_type& bmp, void* state) {
    int x1 = location.x, y1 = location.y, x2 = location.x + bmp.dimensions().width - 1, y2 = location.y + bmp.dimensions().height - 1;
    lcd_panel_draw_bitmap(x1, y1, x2, y2, bmp.begin());
#ifdef LCD_PIN_NUM_VSYNC
    main_screen.set_flush_complete();
#endif
}


void setup() {
    Serial.begin(115200);
#ifdef LCD_PIN_NUM_VSYNC
    lcd_panel_init();
#else
    lcd_panel_init(lcd_buffer_size,lcd_flush_ready);
#endif
    main_screen_init(uix_flush);
#ifdef T_DISPLAY_S3
    pinMode(15, OUTPUT); 
    digitalWrite(15, HIGH);
#elif defined(S3_T_QT_PRO)
    pinMode(4, OUTPUT); 
    digitalWrite(4, HIGH);
#endif
}
void loop() {
    if(timeout_ts!=0 && millis()>timeout_ts+1000) {
        timeout_ts = 0;
        disconnected_label.visible(true);
        disconnected_svg.visible(true);
    }
    main_screen.update();

    int i = Serial.read();
    float tmp;
    if(i>-1) {
        timeout_ts = millis();
        disconnected_label.visible(false);
        disconnected_svg.visible(false);
        switch(i) {
            case read_status::command: {
                read_status data;
                if(sizeof(data)==Serial.readBytes((uint8_t*)&data,sizeof(data))) {
                    // update the display
                    if (cpu_buffers[0].full()) {
                        cpu_buffers[0].get(&tmp);
                    }
                    cpu_buffers[0].put(data.cpu_usage/100.0f);
                    cpu_values[0]=data.cpu_usage/100.0f;
                    if (cpu_buffers[1].full()) {
                        cpu_buffers[1].get(&tmp);
                    }
                    cpu_buffers[1].put(data.cpu_temp/(float)data.cpu_temp_max);
                    if(data.cpu_temp>cpu_max_temp) {
                        cpu_max_temp = data.cpu_temp;
                    }
                    cpu_values[1]=data.cpu_temp/(float)data.cpu_temp_max;
                    cpu_graph.invalidate();
                    cpu_bar.invalidate();
                    sprintf(cpu_sz,"%dC",data.cpu_temp);
                    cpu_temp_label.text(cpu_sz);

                    if (gpu_buffers[0].full()) {
                        gpu_buffers[0].get(&tmp);
                    }
                    gpu_buffers[0].put(data.gpu_usage/100.0f);
                    gpu_values[0] = data.gpu_usage/100.0f;
                    if (gpu_buffers[1].full()) {
                        gpu_buffers[1].get(&tmp);
                    }
                    gpu_buffers[1].put(data.gpu_temp/(float)data.gpu_temp_max);
                    if(data.gpu_temp>gpu_max_temp) {
                        gpu_max_temp = data.gpu_temp;
                    }
                    gpu_values[1] = data.gpu_temp/(float)data.gpu_temp_max;
                    gpu_graph.invalidate();
                    gpu_bar.invalidate();
                    sprintf(gpu_sz,"%dC",data.gpu_temp);
                    gpu_temp_label.text(gpu_sz);
                    
                } else {
                    while(-1!=Serial.read());
                }
            }
            break;
            default:
                while(-1!=Serial.read());
                break;
        };
    } else {
        Serial.write(0x2c);
    }

}