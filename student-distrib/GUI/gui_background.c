#include "gui_background.h"
#include "../svga/vga.h"
#include "gui_imgs.h"
#include "../lib.h"
#include "../png/upng.h"

static unsigned char png_data[SCREEN_HEIGHT * SCREEN_WIDTH * 4];
static unsigned int png_buffer[SCREEN_HEIGHT * SCREEN_WIDTH];


void load_png2buffer(const char* filename, unsigned short* tar_buffer){
    upng_t image;
    const unsigned char *buffer;
    image = upng_new_from_file(filename);
    image.buffer = (unsigned char*) &png_data;
    upng_decode(&image);

    unsigned int idx;
    unsigned px_size = upng_get_pixelsize(&image) / 8;
    int i, j;
    vga_color color;
    buffer = upng_get_buffer(&image);
    unsigned height = upng_get_height(&image);
    unsigned width = upng_get_width(&image);
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            idx = (j * width + i) * px_size;
            if (px_size == 3) {
                png_buffer[j * width + i] = (0x000000 | buffer[idx + 0] << 16 | buffer[idx + 1] << 8 | buffer[idx + 2]);
                color = color_convert(png_buffer[i + j * SCREEN_WIDTH]);
                tar_buffer[i + j * SCREEN_WIDTH] = color;
            } else if (px_size == 4) {
                png_buffer[j * width + i] =
                        (buffer[idx + 3] << 24 | buffer[idx + 0] << 16 | buffer[idx + 1] << 8 | buffer[idx + 2]);
                    color = color_convert(png_buffer[i + j * SCREEN_WIDTH]);
            tar_buffer[i + j * SCREEN_WIDTH] = color;
            } else {
               
            }
        }
    }

    // for(j = 0; j < SCREEN_HEIGHT; j ++){
    //     for(i = 0; i < SCREEN_WIDTH; i ++){
    //         color = color_convert(png_buffer[i + j * SCREEN_WIDTH]);
    //         background_img[i + j * SCREEN_WIDTH] = color;
    //     }
    // }
}

void init_background(){
    // upng_t image;
    // dentry_t png;
    // int32_t size;
    // const unsigned char *buffer;
    // image = upng_new_from_file("N000.png");
    // image.buffer = (unsigned char*) &png_data;
    // upng_decode(&image);

    // unsigned int idx;
    // unsigned px_size = upng_get_pixelsize(&image) / 8;
    // int i, j;
    // buffer = upng_get_buffer(&image);
    // unsigned height = upng_get_height(&image);
    // unsigned width = upng_get_width(&image);
    // for (j = 0; j < height; j++) {
    //     for (i = 0; i < width; i++) {
    //         idx = (j * width + i) * px_size;
    //         if (px_size == 3) {
    //             png_buffer[j * width + i] = (0x000000 | buffer[idx + 0] << 16 | buffer[idx + 1] << 8 | buffer[idx + 2]);
    //         } else if (px_size == 4) {
    //             png_buffer[j * width + i] =
    //                     (buffer[idx + 3] << 24 | buffer[idx + 0] << 16 | buffer[idx + 1] << 8 | buffer[idx + 2]);
    //         } else {
               
    //         }
    //     }
    // }

    // vga_color color;
    // for(j = 0; j < SCREEN_HEIGHT; j ++){
    //     for(i = 0; i < SCREEN_WIDTH; i ++){
    //         color = color_convert(png_buffer[i + j * SCREEN_WIDTH]);
    //         background_img[i + j * SCREEN_WIDTH] = color;
    //     }
    // }

    

    // rpng_load_image("resources/fudesumi_rpng_save.png", &width, &height, &channels, &bits);
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

