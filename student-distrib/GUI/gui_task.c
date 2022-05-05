#include "gui_task.h"
#include "gui_desktop.h"
#include "../svga/vga.h"
#include "gui_timer.h"
#include "../png/upng.h"
#include "gui_imgs.h"
#include "../keyboard.h"
#include "../pit.h"
#include "../sb16.h"
uint8_t *animatation = (uint8_t *)ainimation_start;
int32_t animat_pt[animation_num];
int duck_frame = 0;

// void init_duck(){
//     // cirrus_setpage_2M(SCREEN_HEIGHT / 32 * 2 + 2);
//     // memcpy((void *) VIDEO, &background_img[0], 0x10000);
// }

void desktop_duck(int x, int y)
{
    int offset = 0;
    if (current_buffer)
    {
        offset = SCREEN_HEIGHT;
    }
    __svgalib_cirrusaccel_mmio_ScreenCopy(duck_frame * DUCK_WIDTH, STORE_Y + WINDOW_TITLE_HEIGHT, x, y + offset, DUCK_WIDTH, DUCK_HEIGHT);
}

void init_gui_task()
{
    current_buffer = 0;
}

void gui_do_task()
{
    /* change buffer */
    current_buffer = 1 - current_buffer;

    /* first darw background */
    gui_draw_background();

    draw_timer();

    update_desktop();

    /* draw windows*/
    show_windows();

    desktop_duck(900, 600);

    cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);
    duck_frame++;
    if (duck_frame > 23)
    {
        duck_frame = 0;
    }
}

// void draw_line(int startx, int starty, int length, int height, int orgx){
//     int counter;
//     int x;
//     int offset;
//     for(x = startx + 1; x < startx + length; x ++){
//         counter = 0;
//         offset = 0;
//         current_buffer = 1 - current_buffer;
//         if(current_buffer){
//             offset = SCREEN_HEIGHT;
//         }
//         __svgalib_cirrusaccel_mmio_FillBox(orgx, starty + offset, x, height, 0xFFFFFFFF);
//         while(counter != line_interval){
//             counter ++;
//         }
//         cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);
//     }
//     offset = 0;
//     current_buffer = 1 - current_buffer;
//     if(current_buffer){
//             offset = SCREEN_HEIGHT;
//     }
//     __svgalib_cirrusaccel_mmio_FillBox(orgx, starty + offset, x, height, 0xFFFFFFFF);
// }

void draw_branch(int startx, int starty, int x_len, int y_len, int width, char *name)
{
    int counter = 0;
    int x, y;
    int offset;
    for (y = starty + 1; y < starty + y_len; y++)
    {
        counter = 0;
        offset = 0;
        current_buffer = 1 - current_buffer;
        if (current_buffer)
        {
            offset = SCREEN_HEIGHT;
        }
        __svgalib_cirrusaccel_mmio_FillBox(startx, starty + offset, width, y - starty, 0xFFFFFFFF);
        cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);
        while (counter != line_interval)
        {
            counter++;
        }
    }
    offset = 0;
    current_buffer = 1 - current_buffer;
    if (current_buffer)
    {
        offset = SCREEN_HEIGHT;
    }
    __svgalib_cirrusaccel_mmio_FillBox(startx, starty + offset, width, y - starty - 1, 0xFFFFFFFF);

    current_buffer = 1 - current_buffer;

    int length = strlen((int8_t *)name);
    int i;
    for (i = 0; i < length; i++)
    {
        counter = 0;
        gui_putchar(name[i], startx + i * FONT_WIDTH, y);
        while (counter != line_interval * 10)
        {
            counter++;
        }
    }

    current_buffer = 1 - current_buffer;

    for (i = 0; i < length; i++)
    {
        gui_putchar(name[i], startx + i * FONT_WIDTH, y);
    }
}

void draw_duck(int x, int y)
{
    int i;
    int counter = 0;
    int offset = 0;
    for (i = 0; i < 24; i++)
    {
        offset = 0;
        current_buffer = 1 - current_buffer;
        if (current_buffer)
        {
            offset = SCREEN_HEIGHT;
        }
        counter = 0;
        __svgalib_cirrusaccel_mmio_ScreenCopy(i * DUCK_WIDTH, STORE_Y + WINDOW_TITLE_HEIGHT, x + i, y + offset, DUCK_WIDTH, DUCK_HEIGHT);
        __svgalib_cirrusaccel_mmio_FillBox(128, y + 33 + offset, x + i - 128 + DUCK_WIDTH, 10, 0xFFFFFFFF);
        cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);
        while (counter != line_interval)
        {
            counter++;
        }
    }
}

void boot_amination()
{
    cirrus_setpage_2M(SCREEN_HEIGHT / 32 * 2 + 2);
    memcpy((void *)VIDEO, &background_img[0], 0x10000);
    int i;
    animat_pt[0] = (int32_t)animatation;
    for (i = 1; i < animation_num; i++)
    {
        animat_pt[i] = (int32_t)(animat_pt[i - 1] + ainimation_size);
    }

    char code[] = "0123456789";

    char filename[] = "N000.png";

    char *names[5];
    char name1[] = "1970: UNIX";
    char name2[] = "1981: LINUX";
    char name3[] = "1992: WINDOWS";
    char name4[] = "1997: MacOS";
    char name5[] = "2022: SkyOS";

    names[0] = name1;
    names[1] = name2;
    names[2] = name3;
    names[3] = name4;
    names[4] = name5;

    int j;
    int duck_x = 128;
    int offset = 0;
    for (i = 0; i < animation_num; i++)
    {
        filename[2] = code[i / 10];
        filename[3] = code[i % 10];
        load_png2buffer(filename, (unsigned short *)animat_pt[i]);
        draw_duck(duck_x, 500 + offset);
        if (i % 6 == 2)
        {
            draw_branch(duck_x + DUCK_WIDTH, 500 + 33 + 10, 20, 20, 10, names[i / 6]);
        }
        duck_x += 24;
    }

    play_music((uint8_t *)"Apple.wav");
    int counter = 0;
    for (i = 0; i < animation_num; i++)
    {
        offset = 0;
        current_buffer = 1 - current_buffer;
        if (current_buffer)
        {
            offset = SCREEN_HEIGHT / 32;
        }
        int page;
        for (page = 0; page < 24; page++)
        {
            cirrus_setpage_2M(page + offset);
            memcpy((void *)VIDEO, &((const short *)animat_pt[i])[32768 * page], 0x10000);
        }
        cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);
        counter = 0;
        while (counter != animation_interval)
        {
            counter++;
        }
    }

    memcpy((void *)background_img, ((const short *)animat_pt[31]), ainimation_size);
    // sb16_stop();
}

void close_amination()
{
    int i;
    animat_pt[0] = (int32_t)animatation;
    for (i = 1; i < animation_num; i++)
    {
        animat_pt[i] = (int32_t)(animat_pt[i - 1] + ainimation_size);
    }
    int offset = 0;
    int counter = 0;
    for (i = 31; i >= 0; i--)
    {
        offset = 0;
        current_buffer = 1 - current_buffer;
        if (current_buffer)
        {
            offset = SCREEN_HEIGHT / 32;
        }
        int page;
        for (page = 0; page < 24; page++)
        {
            cirrus_setpage_2M(page + offset);
            memcpy((void *)VIDEO, &((const short *)animat_pt[i])[32768 * page], 0x10000);
        }
        cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);
        counter = 0;
        while (counter != animation_interval)
        {
            counter++;
        }
    }
    char ack_title[] = "ACKNOWLEDGEMENT!!!";
    char acknowledgement1[] = "Thanks professors and TAs";
    char acknowledgement2[] = "Zoom tutorial from Zikai Liu";
    char acknowledgement3[] = "GUI Art Designer: Yichi Jin(Au73)";
    char *acknowledgement[4];
    acknowledgement[0] = ack_title;
    acknowledgement[1] = acknowledgement1;
    acknowledgement[2] = acknowledgement2;
    acknowledgement[3] = acknowledgement3;

    char reference_title[] = "REFERENCE:";
    char reference1[] = "svgalib https://www.svgalib.org";
    char reference2[] = "SVGA Technical Reference Manual of CL-GD5446";
    char reference3[] = "Sound Blaster Series Hardware Programming Guide";
    char reference4[] = "https://wiki.osdev.org/Sound_Blaster_16";
    char reference5[] = "http://qzx.com/pc-gpe/sbdsp.txt;";
    char reference6[] = "github.com/margaretbloom/sb16-wav";
    char reference7[] = "https://github.com/cloudwu/buddy";
    char reference8[] = "https://wiki.osdev.org/Mouse";
    char *reference[9];
    reference[0] = reference_title;
    reference[1] = reference1;
    reference[2] = reference2;
    reference[3] = reference3;
    reference[4] = reference4;
    reference[5] = reference5;
    reference[6] = reference6;
    reference[7] = reference7;
    reference[8] = reference8;

    int length;
    int j;
    int startx = 400;
    int starty = 200;

    current_buffer = 1 - current_buffer;
    cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);

    for (i = 0; i < 4; i++)
    {
        length = strlen((int8_t *)acknowledgement[i]);
        for (j = 0; j < length; j++)
        {
            counter = 0;
            gui_putchar(acknowledgement[i][j], startx + j * FONT_WIDTH, starty);
            while (counter != line_interval * 5)
            {
                counter++;
            }
        }
        starty += FONT_HEIGHT;
    }

    starty += FONT_HEIGHT;
    for (i = 0; i < 9; i++)
    {
        length = strlen((int8_t *)reference[i]);
        for (j = 0; j < length; j++)
        {
            counter = 0;
            gui_putchar(reference[i][j], startx + j * FONT_WIDTH, starty);
            while (counter != line_interval * 5)
            {
                counter++;
            }
        }
        starty += FONT_HEIGHT;
    }

    __svgalib_cirrusaccel_mmio_FillBox(0, 0, 1024, 768 * 2, 0);
}

void init_gui()
{
    init_gui_window_items();
    init_desktop();
    init_gui_task();
    init_gui_font();
    // cirrus_setdisplaystart(2 * 1024 * 2 * 768);
    //  gui_draw_background();
}
