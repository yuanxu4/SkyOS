#include "gui_task.h"
#include "gui_desktop.h"
#include "../svga/vga.h"
#include "gui_timer.h"
void init_gui_task(){
    current_buffer = 0;
}

void gui_do_task(){
    /* change buffer */
    current_buffer = 1 - current_buffer;

    /* first darw background */
    gui_draw_background();

    draw_timer();

    update_desktop();

    /* draw windows*/
    show_windows();

    cirrus_setdisplaystart(current_buffer * 1024 * 2 * 768);

}

void init_gui(){
    init_background();
	init_gui_window_items();
	init_desktop();
	init_gui_task();
	init_gui_font();
	gui_draw_background();
}

