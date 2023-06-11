#include <ui.hpp>
#include <fonts/OpenSans_Regular.hpp>
const gfx::open_font& text_font = OpenSans_Regular;

using namespace uix;
using namespace gfx;
typedef struct bar_info {
    size_t size;
    float* values;
    rgba_pixel<32>* colors;
} bar_info_t;
typedef struct graph_info {
    size_t size;
    buffer_t* buffers;
    rgba_pixel<32>* colors;
} graph_info_t;

static void draw_bar(canvas_t::control_surface_type& destination, const gfx::srect16& clip, void* state) {
    const bar_info_t& inf = *(bar_info_t*)state;
    int h = destination.dimensions().height/inf.size;
    int y = 0;
    for(size_t i = 0;i<inf.size;++i) {
        float v = inf.values[i];
        rgba_pixel<32> col = inf.colors[i];
        rgba_pixel<32> bcol = col;
        if(col==rgba_pixel<32>()) {
            
            hsva_pixel<32> px = color<hsva_pixel<32>>::red;
            hsva_pixel<32> px2 = color<hsva_pixel<32>>::green;
            auto h1 = px.channel<channel_name::H>();
            auto h2 = px2.channel<channel_name::H>();
            h2-=32;
            auto range = abs(h2-h1)+1;
            int w = (int)ceilf(destination.dimensions().width/(float)range)+1;
            int s=1;
            if(destination.dimensions().width<range) {
                w=1;
                s=range/(float)destination.dimensions().width;
            }
            int x = 0;
            int c = 0;
            for(auto j = 0;j<range;++j) {
                px.channel<channel_name::H>(range-c-1+h1);
                rect16 r(x,y,x+w,y+h);
                if(x>=(v*destination.dimensions().width)) {
                    px.channel<channel_name::A>(95);
                } else {
                    px.channel<channel_name::A>(255);
                }
                convert(px,&col);
                draw::filled_rectangle(destination,r,main_screen.background_color(),&clip);
                draw::filled_rectangle(destination,r,col,&clip);
                x+=w;
                c+=s;
            }
            
        } else {
            bcol.channel<channel_name::A>(31);
            draw::filled_rectangle(destination,srect16((destination.dimensions().width*v),y,destination.dimensions().width-1,y+h),bcol,&clip);
            draw::filled_rectangle(destination,srect16(0,y,(destination.dimensions().width*v)-1,y+h),col,&clip);
        }
        y+=h;
    }
}
static void draw_graph(canvas_t::control_surface_type& destination, const gfx::srect16& clip, void* state) {
    const graph_info_t& inf = *(graph_info_t*)state;
    const uint16_t width = destination.dimensions().width;
    const uint16_t height = destination.dimensions().height;
    spoint16 pt;
    for(size_t i = 0; i < inf.size; ++i) {
        buffer_t& buf = inf.buffers[i];
        rgba_pixel<32> col = inf.colors[i];
        bool grad=col==rgba_pixel<32>();
        float v = NAN;
        if(!buf.empty()) {
            v = *buf.peek(0);
            pt.x = 0;
            pt.y = height-(v*height)-1;
            if(pt.y<0) pt.y=0;
        }
        for (size_t i = 1; i < buf.size(); ++i) {
            v = *buf.peek(i);
            if(grad) {
            
                hsva_pixel<32> px = color<hsva_pixel<32>>::red;
                hsva_pixel<32> px2 = color<hsva_pixel<32>>::green;
                auto h1 = px.channel<channel_name::H>();
                auto h2 = px2.channel<channel_name::H>();
                h2-=32;
                auto range = abs(h2-h1)+1;
                px.channel<channel_name::H>(h1+(range-(v*range)));
                convert(px,&col);
            }
            spoint16 pt2;
            pt2.x = (i/100.0f)*width;
            pt2.y = height-(v*height)-1;
            if(pt2.y<0) pt2.y=0;
            screen_t::pixel_type spx = convert<rgba_pixel<32>,rgb_pixel<16>>(col);
            draw::line_aa(destination,srect16(pt,pt2),spx,typename screen_t::pixel_type(),true,&clip);
            pt=pt2;
        }
    }
}

buffer_t cpu_buffers[2];
rgba_pixel<32> cpu_colors[] = { color32_t::blue, rgba_pixel<32>()};
float cpu_values[] = { 0.0f, 0.0f };
int cpu_max_temp = 1;
buffer_t gpu_buffers[2];
rgba_pixel<32> gpu_colors[] = { color32_t::blue, rgba_pixel<32>()};
float gpu_values[] = { 0.0f, 0.0f };
int gpu_max_temp = 1;
rgba_pixel<32> transparent(0,0,0,0);
static uint8_t lcd_buffer1[lcd_buffer_size];
static uint8_t lcd_buffer2[lcd_buffer_size];

screen_t main_screen(lcd_buffer_size,lcd_buffer1,lcd_buffer2);

label_t cpu_label(main_screen);
label_t cpu_temp_label(main_screen);
canvas_t cpu_bar(main_screen);
canvas_t cpu_graph(main_screen);
static bar_info_t cpu_bar_state;
static graph_info_t cpu_graph_state;

label_t gpu_label(main_screen);
canvas_t gpu_bar(main_screen);
label_t gpu_temp_label(main_screen);
canvas_t gpu_graph(main_screen);
static bar_info_t gpu_bar_state;
static graph_info_t gpu_graph_state;

void main_screen_init(screen_t::on_flush_callback_type flush_callback, void* flush_callback_state) {
    main_screen.background_color(color16_t::black);
    main_screen.on_flush_callback(flush_callback,flush_callback_state);
    
    cpu_label.text("CPU");
    cpu_label.text_line_height(main_screen.dimensions().height/10);
    cpu_label.bounds(text_font.measure_text(ssize16::max(),spoint16::zero(),cpu_label.text(),text_font.scale(cpu_label.text_line_height())).bounds().offset(5,5).inflate(8,4));
    cpu_label.text_color(color32_t::white);
    cpu_label.background_color(transparent);
    cpu_label.border_color(transparent);
    cpu_label.text_justify(uix_justify::bottom_right);
    cpu_label.text_open_font(&text_font);
    
    main_screen.register_control(cpu_label);

    cpu_temp_label.bounds(cpu_label.bounds().offset(0,cpu_label.text_line_height()+1));
    cpu_temp_label.text_color(color32_t::white);
    cpu_temp_label.background_color(transparent);
    cpu_temp_label.border_color(transparent);
    cpu_temp_label.text("0C");
    cpu_temp_label.text_justify(uix_justify::bottom_right);
    cpu_temp_label.text_open_font(&text_font);
    cpu_temp_label.text_line_height(cpu_label.text_line_height());
    main_screen.register_control(cpu_temp_label);

    cpu_bar.bounds({int16_t(cpu_label.bounds().x2+5),cpu_label.bounds().y1,int16_t(main_screen.dimensions().width-5),cpu_label.bounds().y2});
    cpu_bar_state.size = 2;
    cpu_bar_state.colors = cpu_colors;
    cpu_bar_state.values = cpu_values;    
    cpu_bar.on_paint(draw_bar,&cpu_bar_state);
    main_screen.register_control(cpu_bar);

    cpu_graph.bounds({cpu_bar.bounds().x1,int16_t(cpu_label.bounds().y2+5),cpu_bar.bounds().x2,int16_t(main_screen.dimensions().height/2-5)});
    cpu_graph_state.size = 2;
    cpu_graph_state.colors = cpu_colors;
    cpu_graph_state.buffers = cpu_buffers;
    cpu_graph.on_paint(draw_graph,&cpu_graph_state);
    main_screen.register_control(cpu_graph);
    
    gpu_label.bounds(cpu_label.bounds().offset(0,main_screen.dimensions().height/2));
    gpu_label.text_color(color32_t::white);
    gpu_label.border_color(transparent);
    gpu_label.background_color(transparent);
    gpu_label.text("GPU");
    gpu_label.text_justify(uix_justify::bottom_right);
    gpu_label.text_open_font(&text_font);
    gpu_label.text_line_height(cpu_label.text_line_height());
    main_screen.register_control(gpu_label);

    gpu_temp_label.bounds(gpu_label.bounds().offset(0,gpu_label.text_line_height()+1));
    gpu_temp_label.text_color(color32_t::white);
    gpu_temp_label.background_color(transparent);
    gpu_temp_label.border_color(transparent);
    gpu_temp_label.text("0C");
    gpu_temp_label.text_justify(uix_justify::bottom_right);
    gpu_temp_label.text_open_font(&text_font);
    gpu_temp_label.text_line_height(cpu_label.text_line_height());
    main_screen.register_control(gpu_temp_label);

    gpu_bar.bounds({int16_t(gpu_label.bounds().x2+5),gpu_label.bounds().y1,int16_t(main_screen.dimensions().width-5),gpu_label.bounds().y2});
    gpu_bar_state.size = 2;
    gpu_bar_state.colors = gpu_colors;
    gpu_bar_state.values = gpu_values;    
    gpu_bar.on_paint(draw_bar,&gpu_bar_state);
    main_screen.register_control(gpu_bar);

    gpu_graph.bounds(cpu_graph.bounds().offset(0,main_screen.dimensions().height/2));
    gpu_graph_state.size = 2;
    gpu_graph_state.colors = gpu_colors;
    gpu_graph_state.buffers = gpu_buffers;
    gpu_graph.on_paint(draw_graph,&gpu_graph_state);
    main_screen.register_control(gpu_graph);

}