#include "gui_desktop.h"
#include "../file_system.h"
#include "../svga/vga.h"
#include "gui_imgs.h"
#include "../lib.h"

void init_desktop()
{
    // uint8_t* file_name_pt = get_all_file_name();
    int file_num = get_file_num();
    uint32_t i;
    int x_offset = 0;
    int y_offset = 0;
    curr_file = 0;
    for (i = 0; i < file_num; i++)
    {
        x_offset = i / 9;
        y_offset = i % 9;
        desktop_file[i].file = get_dentry(i);
        if (desktop_file[i].file->file_type == 1)
        {
            desktop_file[i].x = DESKTOP_GRID_X_START + x_offset * DESKTOP_GRID_X;
            desktop_file[i].y = DESKTOP_GRID_Y_START + y_offset * DESKTOP_GRID_Y;
            curr_file = 1;
        }
        else
        {
            desktop_file[i].x = -1;
            desktop_file[i].y = -1;
        }
    }

    // int x, y;
    // rgb color;
    // for(y = WINDOW_DOWN_HEIGHT + WINDOW_TITLE_HEIGHT + STORE_Y; y <WINDOW_DOWN_HEIGHT + WINDOW_TITLE_HEIGHT + STORE_Y + TERMINAL_ICON_HEIGHT; y ++){
    //     for(x = WINDOW_SIDE_WIDTH; x < WINDOW_SIDE_WIDTH + TERMINAL_ICON_WIDTH; x ++){
    //         color = terminal_icon_img[x - WINDOW_SIDE_WIDTH + (y - WINDOW_DOWN_HEIGHT - WINDOW_TITLE_HEIGHT - STORE_Y) * TERMINAL_ICON_WIDTH];
    //         vga_setcolor(color);
    //         vga_drawpixel(x, y);
    //     }
    // }
}

void draw_file_icon(gui_file *file)
{
    int offset = 0;
    int x = file->x;
    int y = file->y;

    if (current_buffer)
    {
        offset = SCREEN_HEIGHT;
    }

    int i, j;
    int color;
    if (file->file->file_type == 2)
    {
        if (1==is_exe_file(file->file))
        {
        for (j = 0; j < DESKTOP_ICON_IMG_HEIGHT; j++)
        {
            if (j + y >= SCREEN_HEIGHT)
            {
                continue;
            }
            for (i = 0; i < DESKTOP_ICON_IMG_WIDTH; i++)
            {
                if (i + x >= SCREEN_WIDTH || i + x <= 0)
                {
                    continue;
                }
                if (terminal_icon_mask[i + j * DESKTOP_ICON_IMG_WIDTH] == 0)
                {
                    color = terminal_icon_img[i + j * DESKTOP_ICON_IMG_WIDTH] | 0xFF000000;
                    vga_setcolor(color);
                    vga_drawpixel(i + x, j + y + offset);
                }
            }
        }
        }
        else{
        for (j = 0; j < DESKTOP_ICON_IMG_HEIGHT; j++)
        {
            if (j + y >= SCREEN_HEIGHT)
            {
                continue;
            }
            for (i = 0; i < DESKTOP_ICON_IMG_WIDTH; i++)
            {
                if (i + x >= SCREEN_WIDTH || i + x <= 0)
                {
                    continue;
                }
                if (file_icon_mask[i + j * DESKTOP_ICON_IMG_WIDTH] == 0)
                {
                    color = file_icon[i + j * DESKTOP_ICON_IMG_WIDTH] | 0xFF000000;
                    vga_setcolor(color);
                    vga_drawpixel(i + x, j + y + offset);
                }
            }
        }
        }

    }
    else
    {
        for (j = 0; j < DESKTOP_ICON_IMG_HEIGHT; j++)
        {
            if (j + y >= SCREEN_HEIGHT)
            {
                continue;
            }
            for (i = 0; i < DESKTOP_ICON_IMG_WIDTH; i++)
            {
                if (folder_icon_mask[i + j * DESKTOP_ICON_IMG_WIDTH] == 0)
                {
                    if (i + x >= SCREEN_WIDTH || i + x <= 0)
                    {
                        continue;
                    }
                    color = folder_icon[i + j * DESKTOP_ICON_IMG_WIDTH] | 0xFF000000;
                    vga_setcolor(color);
                    vga_drawpixel(i + x, j + y + offset);
                }
            }
        }
    }

    int name_len = strlen((const int8_t *)file->file->file_name);

    int char_start_y = y + DESKTOP_ICON_IMG_HEIGHT + 2;
    int char_start_x;
    if (name_len <= 6)
    {
        char_start_x = x + (DESKTOP_ICON_IMG_WIDTH - name_len * FONT_WIDTH) / 2;
    }
    else
    {
        char_start_x = x + 1;
    }
    for (i = 0; i < 6; i++)
    {
        gui_putchar_transparent(*(file->file->file_name + i), char_start_x, char_start_y);
        char_start_x += FONT_WIDTH;
    }

    if (name_len > 6)
    {
        if (name_len - 6 > 6)
        {
            char_start_x = x + 1;
        }
        else
        {
            char_start_x = x + (DESKTOP_ICON_IMG_WIDTH - (name_len - 6) * FONT_WIDTH) / 2;
        }

        for (i = 0; i < 6; i++)
        {
            gui_putchar_transparent(*(file->file->file_name + i + 6), char_start_x, char_start_y + FONT_HEIGHT);
            char_start_x += FONT_WIDTH;
        }
    }
}

void update_desktop()
{
    int y_off = 0;
    if (current_buffer)
    {
        y_off = SCREEN_HEIGHT;
    }

    int j, i;
    rgb color;
    for (j = 0; j < DESKTOP_ICON_IMG_HEIGHT; j++)
    {
        for (i = 0; i < DESKTOP_ICON_IMG_WIDTH; i++)
        {
            if (terminal_icon_mask[i + j * DESKTOP_ICON_IMG_WIDTH] == 0)
            {
                color = terminal_icon_img[i + j * DESKTOP_ICON_IMG_WIDTH] | 0xFF000000;
                vga_setcolor(color);
                vga_drawpixel(i + TERMINAL_ICON_X, j + TERMINAL_ICON_Y + y_off);
            }
        }
    }
    // uint8_t* file_name_pt = get_all_file_name();
    draw_file_icon(&desktop_file[0]);
    // draw_file_icon(DESKTOP_GRID_X_START + 5 + x_offset * DESKTOP_GRID_X, DESKTOP_GRID_Y_START + y_offset * DESKTOP_GRID_Y, (uint8_t *)(file_name_pt + i * MAX_LEN_FILE_NAME));
    //__svgalib_cirrusaccel_mmio_ScreenCopy(WINDOW_SIDE_WIDTH, WINDOW_DOWN_HEIGHT + WINDOW_TITLE_HEIGHT + STORE_Y, TERMINAL_ICON_X, TERMINAL_ICON_Y + y_off, TERMINAL_ICON_WIDTH, TERMINAL_ICON_HEIGHT);
}

void draw_directory(gui_window *window)
{
    init_desktop();
    int y_off = 0;
    if (current_buffer)
    {
        y_off = SCREEN_HEIGHT;
    }
    gui_file *file_pt = (gui_file *)window->content_pt;
    int i;
    int j;
    int y_offset, x_offset;
    int file_num = get_file_num();
    curr_file = file_num;

    int start_x = window->x + WINDOW_SIDE_WIDTH;
    int start_y = window->y + WINDOW_TITLE_HEIGHT;
    int dir_width = 80 * FONT_WIDTH;
    int dir_height = 25 * FONT_HEIGHT;
    if (window->x + 80 * FONT_WIDTH + WINDOW_SIDE_WIDTH <= 0)
    {
        return;
    }
    else if (window->x + WINDOW_SIDE_WIDTH >= SCREEN_WIDTH)
    {
        return;
    }
    else if (window->x + WINDOW_SIDE_WIDTH + 80 * FONT_WIDTH > 0 && window->x + WINDOW_SIDE_WIDTH < 0)
    {
        start_x = 0;
        dir_width = 80 * FONT_WIDTH + window->x + WINDOW_SIDE_WIDTH;
    }
    else if (window->x + 80 * FONT_WIDTH + WINDOW_SIDE_WIDTH > SCREEN_WIDTH && window->x + WINDOW_SIDE_WIDTH < SCREEN_WIDTH)
    {
        dir_width = 80 * FONT_WIDTH - (window->x + 80 * FONT_WIDTH + WINDOW_SIDE_WIDTH - SCREEN_WIDTH);
    }
    // y
    if (window->y + WINDOW_TITLE_HEIGHT >= SCREEN_HEIGHT)
    {
        return;
    }
    else if (window->y + 25 * FONT_HEIGHT + WINDOW_TITLE_HEIGHT > SCREEN_HEIGHT && window->y + WINDOW_TITLE_HEIGHT < SCREEN_HEIGHT)
    {
        dir_height = 25 * FONT_HEIGHT - (window->y + 25 * FONT_HEIGHT + WINDOW_TITLE_HEIGHT - SCREEN_HEIGHT);
    }
    __svgalib_cirrusaccel_mmio_FillBox(start_x, start_y + y_off, dir_width, dir_height, color_convert(0xFF282828));
    // __svgalib_cirrusaccel_mmio_ScreenCopy(WINDOW_SIDE_WIDTH, STORE_Y + WINDOW_DOWN_HEIGHT + WINDOW_TITLE_HEIGHT,
    //            window->x + WINDOW_SIDE_WIDTH, window->y + WINDOW_TITLE_HEIGHT + y_off, 80 * FONT_WIDTH, 25 * FONT_HEIGHT);
    j = 0;
    for (i = 0; i < file_num - 1; i++)
    {
        if (0 == if_belong_dir(desktop_file[i].file, file_pt->file))
        {
            y_offset = j / 10;
            x_offset = j % 10;
            desktop_file[i].x = window->x + x_offset * DESKTOP_GRID_X + WINDOW_SIDE_WIDTH;
            desktop_file[i].y = window->y + y_offset * DESKTOP_GRID_Y + WINDOW_TITLE_HEIGHT;
            draw_file_icon(&desktop_file[i]);
            j++;
        }
    }
}
