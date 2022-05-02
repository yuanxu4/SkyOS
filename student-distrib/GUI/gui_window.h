#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

/* libs */
#include "gui.h"
#include "../mouse.h"

/* magic numbers */
#define WINDOW_SIDE_WIDTH   3
#define WINDOW_SIDE_HEIGHT  460

#define WINDOW_TITLE_WIDTH  480
#define WINDOW_TITLE_HEIGHT 20

#define WINDOW_CANCEL_ICON_WIDTH 10
#define WINDOW_CANCEL_ICON_HEIGH 20
#define WINDOW_CANCEL_ICON_X 465
#define WINDOW_CANCEL_ICON_Y 0

#define WINDOW_WIDTH    480
#define WINDOW_HEIGHT   480

#define WINDOW_DOWN_WIDTH 480
#define WINDOW_DOWN_HEIGHT 3

/* functions and defines */
void init_gui_window_items();
int draw_gui_window_frame(int x, int y);
void try_to_get_new_window(mouse_location_type type);
void show_windows();
int check_mouse_gui(int x, int y);
void mouse_windows_release_interact_handler(int x, int y);
void mouse_windows_press_interact_handler();
void window_move(int x, int y);

#endif
