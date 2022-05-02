#include "gui_timer.h"
#include "../pit.h"
#include "../svga/vga.h"

void draw_timer(){
    char num[10] = "0123456789";
    char time[8];
    int second = (pit_timer / 100) % 60;
    int minute = ((pit_timer / 100) / 60) % 60;
    int hour = (((pit_timer / 100) / 60) / 60);
    //disp hour
    gui_putchar(num[hour / 10], TIMER_X, TIMER_Y);
    gui_putchar(num[hour % 10], TIMER_X + FONT_WIDTH, TIMER_Y);

    gui_putchar(":", TIMER_X + FONT_WIDTH * 2, TIMER_Y);

    gui_putchar(num[minute / 10], TIMER_X + FONT_WIDTH * 3, TIMER_Y);
    gui_putchar(num[minute % 10], TIMER_X + FONT_WIDTH * 4, TIMER_Y);

    gui_putchar(":", TIMER_X + FONT_WIDTH * 5, TIMER_Y);

    gui_putchar(num[second / 10], TIMER_X + FONT_WIDTH * 6, TIMER_Y);
    gui_putchar(num[second % 10], TIMER_X + FONT_WIDTH * 7, TIMER_Y);
}