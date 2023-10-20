// required to implort the icon implementation
// must appear only once in the project
#define DISCONNECTED_ICON_IMPLEMENTATION
#include <disconnected_icon.hpp>
// our font for the UI. 
#define OPENSANS_REGULAR_IMPLEMENTATION
#include <fonts/OpenSans_Regular.hpp>
#include <ui.hpp>
// for easier modification
const gfx::open_font& text_font = OpenSans_Regular;

using namespace uix;
using namespace gfx;

// used for the draw routines
// the state data for the bars
typedef struct bar_info {
    size_t size;
    float* values;
    rgba_pixel<32>* colors;
} bar_info_t;
// the state data for the graphs
typedef struct graph_info {
    size_t size;
    buffer_t* buffers;
    rgba_pixel<32>* colors;
} graph_info_t;

// callback for a canvas control to draw the bars
static void draw_bar(canvas_t::control_surface_type& destination, 
                    const gfx::srect16& clip, 
                    void* state) {
    // reconstitute our state info
    const bar_info_t& inf = *(bar_info_t*)state;
    // get the height of each bar
    int h = destination.dimensions().height / inf.size;
    int y = 0;
    for (size_t i = 0; i < inf.size; ++i) {
        // the current value to graph
        float v = inf.values[i];
        rgba_pixel<32> col = inf.colors[i];
        rgba_pixel<32> bcol = col;
        // if the color is the default (black), then we create a gradient
        // using the HSV color model
        if (col == rgba_pixel<32>()) {
            // two reference points for the ends of the graph
            hsva_pixel<32> px = color<hsva_pixel<32>>::red;
            hsva_pixel<32> px2 = color<hsva_pixel<32>>::green;
            auto h1 = px.channel<channel_name::H>();
            auto h2 = px2.channel<channel_name::H>();
            // adjust so we don't overshoot
            h2 -= 32;
            // the actual range we're drawing
            auto range = abs(h2 - h1) + 1;
            // the width of each gradient segment
            int w = (int)ceilf(destination.dimensions().width / 
                                (float)range) + 1;
            // the step of each segment - default 1
            int s = 1;
            // if the gradient is larger than the control
            if (destination.dimensions().width < range) {
                // change the segment to width 1
                w = 1;
                // and make its step larger
                s = range / (float)destination.dimensions().width;
            }
            int x = 0;
            // c is the current color offset
            // it increases by s (step)
            int c = 0;
            // for each color in the range
            for (auto j = 0; j < range; ++j) {
                // adjust the H value (inverted and offset)
                px.channel<channel_name::H>(range - c - 1 + h1);
                // create the rect for our segment
                rect16 r(x, y, x + w, y + h);
                // if we're drawing the filled part
                // it's fully opaque
                // otherwise it's semi-transparent
                if (x >= (v * destination.dimensions().width)) {
                    px.channel<channel_name::A>(95);
                } else {
                    px.channel<channel_name::A>(255);
                }
                // black out the area underneath so alpha blending
                // works correctly
                draw::filled_rectangle(destination, 
                                    r, 
                                    main_screen.background_color(), 
                                    &clip);
                // draw the segment
                draw::filled_rectangle(destination, 
                                    r, 
                                    px, 
                                    &clip);
                // increment
                x += w;
                c += s;
            }
        } else {
            // draw the solid color bars
            // first draw the background
            bcol.channel<channel_name::A>(95);
            draw::filled_rectangle(destination, 
                                srect16((destination.dimensions().width * v), 
                                        y, 
                                        destination.dimensions().width - 1, 
                                        y + h), 
                                bcol, 
                                &clip);
            // now the filled part
            draw::filled_rectangle(destination, 
                                srect16(0, 
                                        y, 
                                        (destination.dimensions().width * v) - 1, 
                                        y + h),
                                col, 
                                &clip);
        }
        // increment to the next bar
        y += h;
    }
}
static void draw_graph(canvas_t::control_surface_type& destination, 
                    const gfx::srect16& clip, 
                    void* state) {
    // reconstitute the state
    const graph_info_t& inf = *(graph_info_t*)state;
    // store the dimensions
    const uint16_t width = destination.dimensions().width;
    const uint16_t height = destination.dimensions().height;
    spoint16 pt=spoint16::zero();
    // for each graph
    for (size_t i = 0; i < inf.size; ++i) {
        // easy access to the current buffer
        buffer_t& buf = inf.buffers[i];
        // the current color
        rgba_pixel<32> col = inf.colors[i];
        // is the graph a gradient?
        bool grad = col == rgba_pixel<32>();
        // the point value
        float v = NAN;
        // if we have data
        if (!buf.empty()) {
            // get and store the first value
            // (translating it to the graph)
            v = *buf.peek(0)/255.0f;
            pt.x = 0;
            pt.y = height - (v * height) - 1;
            if (pt.y < 0) pt.y = 0;
        }
        // for each subsequent value
        for (size_t i = 1; i < buf.size(); ++i) {
            // retrieve the value
            v = *buf.peek(i)/255.0f;
            // if it's a gradient
            if (grad) {
                // get our anchors for the ends
                hsva_pixel<32> px = color<hsva_pixel<32>>::red;
                hsva_pixel<32> px2 = color<hsva_pixel<32>>::green;
                // get our H values
                auto h1 = px.channel<channel_name::H>();
                auto h2 = px2.channel<channel_name::H>();
                // offset the second one to avoid overshoot
                h2 -= 32;
                // get the H range
                auto range = abs(h2 - h1) + 1;
                // set the H value based on v (inverted and offet)
                px.channel<channel_name::H>(h1 + (range - (v * range)));
                // convert to RGBA8888
                convert(px, &col);
            }
            // compute the current data point
            spoint16 pt2;
            pt2.x = (i / 100.0f) * width;
            pt2.y = height - (v * height) - 1;
            if (pt2.y < 0) pt2.y = 0;
            // draw an anti-aliased line
            // from the old point to the 
            // new point.
            draw::line_aa(destination, 
                            srect16(pt, pt2), 
                            col, 
                            col, 
                            true, 
                            &clip);
            // store the current point as 
            // the next old point
            pt = pt2;
        }
    }
}

screen_t main_screen({LCD_WIDTH,LCD_HEIGHT},lcd_buffer_size,lcd_buffer1,lcd_buffer2);
screen_t disconnected_screen({LCD_WIDTH,LCD_HEIGHT},lcd_buffer_size,lcd_buffer1,lcd_buffer2);

// define the declarations from the header
buffer_t cpu_buffers[2];
rgba_pixel<32> cpu_colors[] = {color32_t::light_blue, rgba_pixel<32>()};
float cpu_values[] = {0.0f, 0.0f};
int cpu_max_temp = 1;
buffer_t gpu_buffers[2];
rgba_pixel<32> gpu_colors[] = {color32_t::light_blue, rgba_pixel<32>()};
float gpu_values[] = {0.0f, 0.0f};
int gpu_max_temp = 1;

// define our CPU controls and state
label_t cpu_label(main_screen);
label_t cpu_temp_label(main_screen);
canvas_t cpu_bar(main_screen);
canvas_t cpu_graph(main_screen);
static bar_info_t cpu_bar_state;
static graph_info_t cpu_graph_state;

// define our GPU controls and state
label_t gpu_label(main_screen);
canvas_t gpu_bar(main_screen);
label_t gpu_temp_label(main_screen);
canvas_t gpu_graph(main_screen);
static bar_info_t gpu_bar_state;
static graph_info_t gpu_graph_state;

// define our disconnected controls
svg_box_t disconnected_svg(disconnected_screen);
label_t disconnected_label(disconnected_screen);

// initialize the main screen
void main_screen_init() {
    uint16_t w = main_screen.dimensions().width,
        h = main_screen.dimensions().height;
    // declare a transparent pixel/color
    rgba_pixel<32> transparent(0, 0, 0, 0);
    // screen is black
    main_screen.background_color(color_t::black);

    // declare the first label. Everything else is based on this.
    // to do so we measure the size of the text (@ 1/7th of 
    // height of the screen) and bound the label based on that
    cpu_label.text("CPU");
    cpu_label.text_line_height(h / 7);
    cpu_label.bounds(text_font.measure_text(ssize16::max(),
                                            spoint16::zero(),
                                            cpu_label.text(),
                                            text_font.scale(h / 7))
                         .bounds()
                         .offset(5, 5)
                         .inflate(8, 4));
    // set the design properties
    cpu_label.text_color(color32_t::white);
    cpu_label.background_color(transparent);
    cpu_label.border_color(transparent);
    cpu_label.text_justify(uix_justify::bottom_right);
    cpu_label.text_open_font(&text_font);
    // register the control with the screen
    main_screen.register_control(cpu_label);

    // the temp label is right below the first label
    cpu_temp_label.bounds(cpu_label.bounds()
                              .offset(0, (h / 7) + 1));
    cpu_temp_label.text_color(color32_t::white);
    cpu_temp_label.background_color(transparent);
    cpu_temp_label.border_color(transparent);
    cpu_temp_label.text("0C");
    cpu_temp_label.text_justify(uix_justify::bottom_right);
    cpu_temp_label.text_open_font(&text_font);
    cpu_temp_label.text_line_height((h / 7));
    main_screen.register_control(cpu_temp_label);

    // the bars are to the right of the label
    cpu_bar.bounds({int16_t(cpu_label.bounds().x2 + 5), 
                    cpu_label.bounds().y1, 
                    int16_t(w - 5), 
                    cpu_label.bounds().y2});
    cpu_bar_state.size = 2;
    cpu_bar_state.colors = cpu_colors;
    cpu_bar_state.values = cpu_values;
    cpu_bar.on_paint_callback(draw_bar, &cpu_bar_state);
    main_screen.register_control(cpu_bar);

    // the graph is below the above items.
    cpu_graph.bounds({cpu_bar.bounds().x1, 
                        int16_t(cpu_label.bounds().y2 + 5), 
                        cpu_bar.bounds().x2, 
                        int16_t(h / 
                                    2 - 5)});
    cpu_graph_state.size = 2;
    cpu_graph_state.colors = cpu_colors;
    cpu_graph_state.buffers = cpu_buffers;
    cpu_graph.on_paint_callback(draw_graph, &cpu_graph_state);
    main_screen.register_control(cpu_graph);

    // the GPU label is offset from the CPU
    // label by half the height of the screen
    gpu_label.bounds(cpu_label.bounds().offset(0, h / 2));
    gpu_label.text_color(color32_t::white);
    gpu_label.border_color(transparent);
    gpu_label.background_color(transparent);
    gpu_label.text("GPU");
    gpu_label.text_justify(uix_justify::bottom_right);
    gpu_label.text_open_font(&text_font);
    gpu_label.text_line_height((h / 7));
    main_screen.register_control(gpu_label);

    // lay out the rest of the controls the 
    // same as was done with the CPU
    gpu_temp_label.bounds(gpu_label.bounds().offset(0, (h / 7) + 1));
    gpu_temp_label.text_color(color32_t::white);
    gpu_temp_label.background_color(transparent);
    gpu_temp_label.border_color(transparent);
    gpu_temp_label.text("0C");
    gpu_temp_label.text_justify(uix_justify::bottom_right);
    gpu_temp_label.text_open_font(&text_font);
    gpu_temp_label.text_line_height((h / 7));
    main_screen.register_control(gpu_temp_label);

    gpu_bar.bounds({int16_t(gpu_label.bounds().x2 + 5), 
                    gpu_label.bounds().y1, 
                    int16_t(w - 5), 
                    gpu_label.bounds().y2});
    gpu_bar_state.size = 2;
    gpu_bar_state.colors = gpu_colors;
    gpu_bar_state.values = gpu_values;
    gpu_bar.on_paint_callback(draw_bar, &gpu_bar_state);
    main_screen.register_control(gpu_bar);

    gpu_graph.bounds(cpu_graph.bounds()
                        .offset(0, h / 2));
    gpu_graph_state.size = 2;
    gpu_graph_state.colors = gpu_colors;
    gpu_graph_state.buffers = gpu_buffers;
    gpu_graph.on_paint_callback(draw_graph, &gpu_graph_state);
    main_screen.register_control(gpu_graph);
}

void disconnected_screen_init() {
    disconnected_label.bounds(disconnected_screen.bounds());
    disconnected_label.background_color(color32_t::white);
    disconnected_label.border_color(color32_t::white);
    disconnected_screen.register_control(disconnected_label);
    // here we center and scale the SVG control based on
    // the size of the screen, clamped to a max of 128x128
    float sscale;
    if(disconnected_screen.dimensions().width<128 || disconnected_screen.dimensions().height<128) {
        sscale = disconnected_icon.scale(disconnected_screen.dimensions());
    } else {
        sscale = disconnected_icon.scale(size16(128,128));
    }
    disconnected_svg.bounds(srect16(0,
                                    0,
                                    disconnected_icon.dimensions().width*sscale-1,
                                    disconnected_icon.dimensions().height*sscale-1)
                                        .center(disconnected_screen.bounds()));
    disconnected_svg.doc(&disconnected_icon);
    disconnected_screen.register_control(disconnected_svg);
}