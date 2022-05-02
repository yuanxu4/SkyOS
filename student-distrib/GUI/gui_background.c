#include "gui_background.h"
#include "../svga/vga.h"
#include "gui_imgs.h"
#include "../lib.h"
void init_background(){
    // int x, y;

    // for(y = 0; y < SCREEN_HEIGHT; y ++){
    //     for(x = 0; x < SCREEN_WIDTH; x ++){
    //         background_img[x + y * SCREEN_WIDTH] |= 0xFF000000;
    //     }
    // }

    // for(y = 0; y < SCREEN_HEIGHT; y ++){
    //     for(x = 0; x < SCREEN_WIDTH; x ++){
    //         background_img[x + y * SCREEN_WIDTH] = color_convert(background_img[x + y * SCREEN_WIDTH]);
    //     }
    // }
}

void gui_draw_background(){
    int page;
    int offset = 0;
    if(current_buffer == 1){
        offset = SCREEN_HEIGHT / 32;
    }

    for (page = 0; page < 24; page++) {
        cirrus_setpage_2M(page + offset);
        memcpy((void *) VIDEO, &(background_img[32768 * page]), 0x10000);
    }

    // int x, y;
    // for(y = 0; y < SCREEN_HEIGHT; y ++){
    //     for(x = 0; x < SCREEN_WIDTH; x ++){
    //         vga_setcolor(background_img[x + y * SCREEN_WIDTH]);
    //         vga_drawpixel(x, y);
    //     }
    // }
}

