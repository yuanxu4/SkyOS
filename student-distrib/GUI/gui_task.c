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

void boot_amination(){
    int i;
    animat_pt[0] = (int32_t)animatation;
    for(i = 1; i < 32; i ++){
        animat_pt[i] = (int32_t)(animat_pt[i - 1] + ainimation_size);
    }

    unsigned char code[] = "0123456789";

    unsigned char filename[] = "N000.png";
    for(i = 0; i < 32; i ++){
        filename[2] = code[i/10];
        filename[3] = code[i%10];
        load_png2buffer(filename, (unsigned short*)animat_pt[i]);
    }

    int page;
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

    boot_amination();

	gui_draw_background();
}

