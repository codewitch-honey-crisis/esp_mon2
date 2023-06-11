#pragma once
#include <lcd_config.h>
#include <uix.hpp>
#include <circular_buffer.hpp>
using screen_t = uix::screen<LCD_WIDTH,LCD_HEIGHT,gfx::rgb_pixel<16>>;
using label_t = uix::label<typename screen_t::pixel_type,typename screen_t::palette_type>;
using svg_box_t = uix::svg_box<typename screen_t::pixel_type,typename screen_t::palette_type>;
using canvas_t = uix::canvas<typename screen_t::pixel_type,typename screen_t::palette_type>;
using color16_t = gfx::color<gfx::rgb_pixel<16>>;
using color32_t = gfx::color<gfx::rgba_pixel<32>>;
using buffer_t = circular_buffer<float,100>;

extern buffer_t cpu_buffers[];
extern float cpu_values[];
extern int cpu_max_temp;
extern gfx::rgba_pixel<32> cpu_colors[];
extern buffer_t gpu_buffers[];
extern float gpu_values[];
extern int gpu_max_temp;
extern gfx::rgba_pixel<32> gpu_colors[];

constexpr static const int lcd_buffer_size = 32 * 1024;

extern screen_t main_screen;

extern label_t cpu_label;
extern label_t cpu_temp_label;
extern canvas_t cpu_bar;
extern canvas_t cpu_graph;

extern label_t gpu_label;
extern label_t gpu_temp_label;
extern canvas_t gpu_bar;
extern canvas_t gpu_graph;

extern label_t disconnected_label;
extern svg_box_t disconnected_svg;

extern void main_screen_init(screen_t::on_flush_callback_type flush_callback, void* flush_callback_state = nullptr);