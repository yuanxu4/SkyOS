#ifndef GUI_IMG_H
#define GUI_IMG_H
#include "gui.h"
#include "gui_desktop.h"

#define GRAPH_START_ADDR 0x02400000
#define BACKGROUND_SIZE  SCREEN_HEIGHT * SCREEN_WIDTH
#define WIN_UP_SIZE      WINDOW_TITLE_WIDTH * WINDOW_TITLE_HEIGHT * 2
#define WIN_SIDE_SIZE    WINDOW_SIDE_HEIGHT * WINDOW_SIDE_WIDTH * 2
#define WIN_DOWN_SIZE    WINDOW_DOWN_WIDTH * WINDOW_DOWN_HEIGHT * 2
#define FILE_ICON_SIZE   DESKTOP_ICON_IMG_HEIGHT * DESKTOP_ICON_IMG_WIDTH * 2
#define FILE_ICON_MASK_SIZE DESKTOP_ICON_IMG_HEIGHT * DESKTOP_ICON_IMG_WIDTH * 2
#define FOLDER_ICON_SIZE   DESKTOP_ICON_IMG_HEIGHT * DESKTOP_ICON_IMG_WIDTH * 2
#define FOLDER_ICON_MASK_SIZE DESKTOP_ICON_IMG_HEIGHT * DESKTOP_ICON_IMG_WIDTH * 2

// unsigned short* background_img;

// unsigned int* win_up_img;

// unsigned int* win_side_img;

// unsigned int* win_down_img;

// unsigned int* file_icon;

// unsigned int* file_icon_mask;

// unsigned int* folder_icon;

// unsigned int* folder_icon_mask;
extern unsigned short background_img[SCREEN_HEIGHT * SCREEN_WIDTH];

extern unsigned int win_up_img[WINDOW_TITLE_WIDTH * WINDOW_TITLE_HEIGHT];

extern unsigned int win_side_img[WINDOW_SIDE_HEIGHT * WINDOW_SIDE_WIDTH];

extern unsigned int win_down_img[WINDOW_DOWN_WIDTH * WINDOW_DOWN_HEIGHT];

extern unsigned int file_icon[DESKTOP_ICON_IMG_HEIGHT * DESKTOP_ICON_IMG_WIDTH];

extern unsigned int file_icon_mask[DESKTOP_ICON_IMG_HEIGHT * DESKTOP_ICON_IMG_WIDTH];

extern unsigned int folder_icon[DESKTOP_ICON_IMG_HEIGHT * DESKTOP_ICON_IMG_WIDTH];

extern unsigned int folder_icon_mask[DESKTOP_ICON_IMG_HEIGHT * DESKTOP_ICON_IMG_WIDTH];

extern unsigned int terminal_icon_img[TERMINAL_ICON_WIDTH * TERMINAL_ICON_HEIGHT];

#endif
