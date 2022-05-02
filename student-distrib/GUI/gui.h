#ifndef GUI_H
#define GUI_H

/* libs */
#include "gui_window.h"
#include "gui_task.h"
#include "gui_cursor.h"
#include "text.h"
#include "gui_background.h"
#include "../lib.h"
#include "../file_system.h"

/* magic number */
#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT    768

#define FONT_Y          SCREEN_HEIGHT * 2
#define FONT_WIDTH      8
#define FONT_HEIGHT     16

#define TERMINAL_ICON_X     9
#define TERMINAL_ICON_Y     DESKTOP_GRID_Y + DESKTOP_GRID_Y_START
#define TERMINAL_ICON_WIDTH 50
#define TERMINAL_ICON_HEIGHT    50

#define BAR_WIDTH
#define BAR_HEIGHT
#define BAR_X
#define BAR_Y
#define STORE_Y (SCREEN_HEIGHT * 2 + FONT_HEIGHT * 2)

#define WINDOW_MAX_NUM 3

/* struct define */
typedef struct gui_item gui_item;
struct gui_item{
    unsigned char* buffer;
    unsigned int width;
    unsigned int height;
    int x;
    int y;
};

typedef struct gui_window gui_window;
struct gui_window{
    int x;
    int y;
    uint32_t content_pt;
    int prior;
    int status;
    int type;
    int enable_cursor;
};

typedef struct gui_status_bar gui_status_bar;
struct gui_status_bar{
    unsigned char* buffer;
    unsigned int width;
    unsigned int height;
    int x;
    int y;
};

typedef struct gui_file gui_file;
struct gui_file{
    dentry_t* file;
    int x;
    int y;
};

int current_buffer;

gui_item background;
gui_status_bar status_bar;
gui_window window[WINDOW_MAX_NUM];
gui_file    desktop_file[NUM_DIR_ENTRY];


#endif
