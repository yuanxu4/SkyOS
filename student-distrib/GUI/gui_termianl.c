#include "gui_terminal.h"
#include "../keyboard.h"
#include "../pit.h"
#include "../svga/vga.h"

void draw_terminal(gui_window* window){
    int offset = 0;
    if(current_buffer){
        offset = SCREEN_HEIGHT;
    }

    /* check if valid */
    if(window->content_pt == NULL){
        return;
    }

    /* init x, y */
    int x = window->x + WINDOW_SIDE_WIDTH;
    int y = window->y + WINDOW_TITLE_HEIGHT;
    int i, j;
    int ch_x = 0;
    int ch_y = 0;
    terminal_t* terminal_pt = (terminal_t*)window->content_pt;
    for(j = 0; j < GUI_TERMINAL_HEIGHT; j ++){
        ch_x = 0;
        for(i = 0; i < GUI_TERMINAL_WIDTH; i ++){
            // if(*(char *)(terminal_pt->page_addr + i + j * GUI_TERMINAL_WIDTH * 2) == 0x7){
            //     continue;
            // }
            gui_putchar(*(char *)(terminal_pt->page_addr + i * 2 + j * GUI_TERMINAL_WIDTH * 2), x + ch_x * FONT_WIDTH, y + ch_y * FONT_HEIGHT);//*(window->content + i + j * GUI_TERMINAL_WIDTH)
            ch_x ++;
        }
        ch_y ++;
    }

    if(window->enable_cursor){
        if(pit_timer % 200 == 0){
            __svgalib_cirrusaccel_mmio_Reverse((terminal_pt->cursor_x) * FONT_WIDTH + x, (terminal_pt->cursor_y) * FONT_HEIGHT + y + offset, FONT_WIDTH, FONT_HEIGHT);
        }
    }
}
