#ifndef GUI_TIMER_H
#define GUI_TIMER_H

#include "gui.h"

#define TIMER_WIDTH     FONT_WIDTH * 8
#define TIMER_HEIGHT    FONT_HEIGHT
#define TIMER_X         SCREEN_WIDTH - TIMER_WIDTH - 5
#define TIMER_Y         SCREEN_HEIGHT - TIMER_HEIGHT - 1

void draw_timer();

#endif
