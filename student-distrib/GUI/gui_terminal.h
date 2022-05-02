#ifndef GUI_TERMINAL_H
#define GUI_TERMINAL_H

/* libs */
#include "gui.h"

/* magic numbers */
#define GUI_TERMINAL_WIDTH      80
#define GUI_TERMINAL_HEIGHT     25
#define GUI_TERMINAL_WIDTH_PIXEL     GUI_TERMINAL_WIDTH * FONT_WIDTH
#define GUI_TERMINAL_HEIGHT_PIXEL    GUI_TERMINAL_HEIGHT * FONT_HEIGHT

/* function define */
void draw_terminal(gui_window* window);

#endif
