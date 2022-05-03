#ifndef GUI_DESKTOP_H
#define GUI_DESKTOP_H

#include "gui.h"

/* magic num */
#define DESKTOP_ICON_IMG_WIDTH      50
#define DESKTOP_ICON_IMG_HEIGHT     50
#define DESKTOP_GRID_X  60
#define DESKTOP_GRID_Y  90
#define DESKTOP_GRID_X_START    10
#define DESKTOP_GRID_Y_START    10
#define DESKTOP_GRID_X_END      1020
#define DESKTOP_GRID_Y_END      660

int  curr_file;
void update_desktop();
void init_desktop();
void draw_directory(gui_window* window);

#endif
