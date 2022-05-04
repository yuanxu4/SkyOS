#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

/* libs */
#include "gui.h"
#include "../mouse.h"

/* magic numbers */
#define WINDOW_SIDE_WIDTH   0
#define WINDOW_SIDE_HEIGHT  0

#define WINDOW_TITLE_WIDTH  640
#define WINDOW_TITLE_HEIGHT 32

#define WINDOW_CANCEL_ICON_WIDTH 26
#define WINDOW_CANCEL_ICON_HEIGH 26
#define WINDOW_CANCEL_ICON_X 426
#define WINDOW_CANCEL_ICON_Y 4

#define WINDOW_WIDTH    480
#define WINDOW_HEIGHT   480

#define WINDOW_DOWN_WIDTH 0
#define WINDOW_DOWN_HEIGHT 0

/* functions and defines */
void init_gui_window_items();
int draw_gui_window_frame(int x, int y);
void try_to_get_new_window(mouse_location_type type);
void show_windows();
int check_mouse_gui(int x, int y);
void mouse_windows_release_interact_handler(int x, int y, mouse_location_type type);
void mouse_windows_press_interact_handler();
void window_move(int x, int y);

#endif
