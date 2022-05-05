#include "gui_task.h"
#include "gui_desktop.h"
#include "../svga/vga.h"
#include "gui_timer.h"
#include "../png/upng.h"
#include "gui_imgs.h"

uint8_t* animatation = (uint8_t*)ainimation_start;
int32_t animat_pt[32];

void init_gui_task(){
    current_buffer = 0;
}

void gui_do_task(){
    /* change buffer */
    current_buffer = 1 - current_buffer;

    /* first darw background */
    gui_draw_background();

    draw_timer();

    update_desktop();

    /* draw windows*/
    show_windows();

    cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);

}

void draw_line(int startx, int starty, int length, int height, int orgx){
    int counter;
    int x;
    int offset;
    for(x = startx + 1; x < startx + length; x ++){
        counter = 0;
        offset = 0;
        current_buffer = 1 - current_buffer;
        if(current_buffer){
            offset = SCREEN_HEIGHT;
        }
        __svgalib_cirrusaccel_mmio_FillBox(orgx, starty + offset, x, height, 0xFFFFFFFF);
        while(counter != line_interval){
            counter ++;
        }
        cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);
    }
    offset = 0;
    current_buffer = 1 - current_buffer;
    if(current_buffer){
            offset = SCREEN_HEIGHT;
    }
    __svgalib_cirrusaccel_mmio_FillBox(orgx, starty + offset, x, height, 0xFFFFFFFF);
}

void draw_branch(int startx, int starty, int x_len, int y_len, int width){
    int counter = 0;
    int x, y;
    int offset;
    for(y = starty + 1; y < starty + y_len; y ++){
        counter = 0;
        offset = 0;
        current_buffer = 1 - current_buffer;
        if(current_buffer){
            offset = SCREEN_HEIGHT;
        }
        __svgalib_cirrusaccel_mmio_FillBox(startx, starty + offset, width, y - starty, 0xFFFFFFFF);
        while(counter != line_interval){
            counter ++;
        }
    }
    offset = 0;
    current_buffer = 1 - current_buffer;
    if(current_buffer){
            offset = SCREEN_HEIGHT;
    }
    // __svgalib_cirrusaccel_mmio_FillBox(startx, starty + offset, width, y - starty, 0xFFFFFFFF);
}

void boot_amination(){
    int i;
    animat_pt[0] = (int32_t)animatation;
    for(i = 1; i < 32; i ++){
        animat_pt[i] = (int32_t)(animat_pt[i - 1] + ainimation_size);
    }

    unsigned char code[] = "0123456789";

    unsigned char filename[] = "N000.png";

    int line_x = 0;
    int line_y = 500;
    int line_height = 10;
    for(i = 0; i < 32; i ++){
        filename[2] = code[i/10];
        filename[3] = code[i%10];
        load_png2buffer(filename, (unsigned short*)animat_pt[i]);
        draw_line(line_x, line_y, 1024 / 32, line_height, 0);
        line_x += 1024 / 32;
        if(i % 8 == 0){
            draw_branch(line_x, line_y, 50, 50, 10);
        }
    }

    int offset = 0;
    int counter = 0;
    for(i = 0; i < 32; i ++){
        offset = 0;
        current_buffer = 1 - current_buffer;
        if(current_buffer){
            offset = SCREEN_HEIGHT / 32;
        }
        int page;
        for (page = 0; page < 24; page++) {
            cirrus_setpage_2M(page + offset);
            memcpy((void *) VIDEO, &((const short*)animat_pt[i])[32768 * page], 0x10000);
        }
        cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);
        counter = 0;
        while(counter != animation_interval){
            counter ++;
        }
    }

    memcpy((void *)background_img, ((const short*)animat_pt[31]), ainimation_size);

}

void init_gui(){
    init_background();
	init_gui_window_items();
	init_desktop();
	init_gui_task();
	init_gui_font();

    // boot_amination();
	// gui_draw_background();
}

