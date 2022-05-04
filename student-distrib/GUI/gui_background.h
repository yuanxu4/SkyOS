#ifndef GUI_BACKGROUND_H
#define GUI_BACKGROUND_H

/* libs */
#include "gui.h"

/* magic num */


/*function and define */
void gui_draw_background();
void init_background();
void load_png2buffer(const char* filename, unsigned short* tar_buffer);

#endif
