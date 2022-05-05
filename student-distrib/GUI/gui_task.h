#ifndef GUI_TASK_H
#define GUI_TASK_H

/* libs */
#include "gui.h"

#define ainimation_start 0x2400000
#define animation_interval 10000000

#define ainimation_size  SCREEN_WIDTH * SCREEN_HEIGHT * 2
#define animation_num 32
#define line_interval 5000000

void gui_do_task();
void init_gui_task();
void init_gui();
void desktop_duck(int x, int y);

#endif

