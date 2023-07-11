#ifdef ESP_PLATFORM
#define LCD_IMPLEMENTATION
#include <lcd_init.h>
#endif
#include <display.hpp>
#ifndef ESP_PLATFORM
static lcd_t lcd;
#endif
// define our transfer buffer(s) and initialize
// the main screen with it/them.
// for RGB interface screens we only use one
// because there is no DMA
#ifdef LCD_DMA
uint8_t lcd_buffer1[lcd_buffer_size];
uint8_t lcd_buffer2[lcd_buffer_size];
#else
uint8_t lcd_buffer1[lcd_buffer_size];
#endif

screen_t* active_screen = nullptr;


#ifdef ESP_PLATFORM
// only needed if DMA enabled
#ifdef LCD_DMA
static bool lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, 
                            esp_lcd_panel_io_event_data_t* edata, 
                            void* user_ctx) {
    if(active_screen!=nullptr) {
        active_screen->set_flush_complete();
    }
    return true;
}
#endif
static void uix_wait(void* state) {
}
static void uix_flush(const gfx::rect16& bounds, 
                    const void* bmp, 
                    void* state) {
    lcd_panel_draw_bitmap(bounds.x1,bounds.y1,bounds.x2,bounds.y2,(void*)bmp);
    // no DMA, so we are done once the above completes
#ifndef LCD_DMA
    if(active_screen!=nullptr) {
        active_screen->set_flush_complete();
    }
#endif
}
void display_init() {
#ifndef LCD_PIN_NUM_VSYNC
    lcd_panel_init(lcd_buffer_size,lcd_flush_ready);
#else
    lcd_panel_init();
#endif
}
#else
void display_init() {
    lcd.initialize();
}
void uix_wait(void* state) {
    gfx::draw::wait_all_async(lcd);
}
void uix_flush(const gfx::rect16& bounds, 
                    const void* bmp, 
                    void* state) {
    gfx::const_bitmap<screen_t::pixel_type,screen_t::palette_type> cbmp(bounds.dimensions(),bmp);
    gfx::draw::bitmap_async(lcd,bounds,cbmp,cbmp.bounds());
}
#endif

void display_update() {
    if(active_screen!=nullptr) {
        active_screen->update();
    }
}

void display_screen(screen_t& new_screen) {
    
    if(active_screen!=nullptr) {
        if(active_screen->flushing()) {
            active_screen->update();
        }
    }
    active_screen = &new_screen;
    active_screen->on_flush_callback(uix_flush);
#ifndef ESP_PLATFORM
    active_screen->wait_flush_callback(uix_wait);
#endif
    active_screen->invalidate();

    
}