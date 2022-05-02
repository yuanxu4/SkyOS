#include "gui_window.h"
#include "../svga/vga.h"
#include "gui_imgs.h"
#include "gui_desktop.h"
#include "gui_terminal.h"
#include "../keyboard.h"

static int win_init_x = 50;
static int win_init_y = 50;
static int curr_win_num = 0;
static int mouse_target_id = -1;
static int mouse_target_file = -1;
static int on_title = 0;

// static int mouse_window_offset_x = 999;
// static int mouse_windos_offset_y = 999;

/*
 * init_gui_window_items
 * description: set windows item to video mem
 * input: none
 * output: none
 * return: 0 for success, -1 for fail
 */
void init_gui_window_items(){
    unsigned int color;
    /* init window side */
    int i;
    int j;

    for(i = 0; i < WINDOW_SIDE_WIDTH; i ++){
        for(j = STORE_Y; j < STORE_Y + WINDOW_SIDE_HEIGHT; j ++){
            color = win_side_img[i + (j - STORE_Y) * WINDOW_SIDE_WIDTH];
            vga_setcolor(color);
            vga_drawpixel(i, j);
        }
    }

    /* init window title */
    i = 0;
    j = 0;

    for(i = WINDOW_SIDE_WIDTH; i < WINDOW_TITLE_WIDTH + WINDOW_SIDE_WIDTH; i ++){
        for(j = STORE_Y; j < STORE_Y + WINDOW_TITLE_HEIGHT; j ++){
            color = win_up_img[(i - WINDOW_SIDE_WIDTH) + (j - STORE_Y) * WINDOW_TITLE_WIDTH];
            vga_setcolor(color);
            vga_drawpixel(i, j);
        }
    }

    /* init window down */
    for(i = WINDOW_SIDE_WIDTH; i < WINDOW_DOWN_WIDTH + WINDOW_SIDE_WIDTH; i ++){
        for(j = STORE_Y + WINDOW_TITLE_HEIGHT; j < STORE_Y + WINDOW_TITLE_HEIGHT + WINDOW_DOWN_HEIGHT; j ++){
            color = win_down_img[(i - WINDOW_SIDE_WIDTH) + (j - STORE_Y - WINDOW_TITLE_HEIGHT) * WINDOW_DOWN_WIDTH];
            vga_setcolor(color);
            vga_drawpixel(i, j);
        }
    }

    /* init window list */
    for(i = 0; i < WINDOW_MAX_NUM; i ++){
        //window[i].content_pt = NULL;
        window[i].prior = WINDOW_MAX_NUM;
        window[i].status = 0;
        window[i].x = 0;
        window[i].y = 0;
        window[i].enable_cursor = 0;
    }

    curr_win_num = 0;

}

int draw_gui_window_frame(int x, int y){
    int frame_x;
    int win_x;
    int frame_y;
    int win_y;
    int frame_w;
    int frame_h;
    int offset = 0;
    /* switch buffer */
    if(current_buffer){
        offset += SCREEN_HEIGHT;
    }

    /* check if out of screen */
    if(x < 0){
        frame_x = WINDOW_SIDE_WIDTH - x;
        frame_w = WINDOW_TITLE_WIDTH + x;
        win_x = 0;
    }
    else if(x + WINDOW_TITLE_WIDTH > SCREEN_WIDTH){
        frame_x = WINDOW_SIDE_WIDTH;
        frame_w = WINDOW_TITLE_WIDTH - (x + WINDOW_TITLE_WIDTH - SCREEN_WIDTH);
        win_x = x;
    }
    else{
        frame_x = WINDOW_SIDE_WIDTH;
        frame_w = WINDOW_TITLE_WIDTH;
        win_x = x;
    }

    if(y < 0){
        frame_y = STORE_Y - y;
        frame_h = WINDOW_TITLE_HEIGHT + y;
        win_y = 0;
    }
    else if(y + WINDOW_TITLE_HEIGHT > SCREEN_HEIGHT){
        frame_y = STORE_Y;
        frame_h = WINDOW_TITLE_HEIGHT - (y + WINDOW_TITLE_HEIGHT - SCREEN_HEIGHT);
        win_y = y;
    }
    else{
        frame_y = STORE_Y;
        frame_h = WINDOW_TITLE_HEIGHT;
        win_y = y;
    }

    __svgalib_cirrusaccel_mmio_ScreenCopy(frame_x, frame_y, win_x, win_y + offset, frame_w, frame_h); //title

        /* check if left side out of screen */
    int draw_flag = 1;
    if(x < 0 && x + WINDOW_SIDE_WIDTH > 0){ //part out
        frame_x = - x;
        frame_w = WINDOW_SIDE_WIDTH + x;
        win_x = 0;
    }
    else if(x + WINDOW_SIDE_WIDTH < 0){ //all out
        draw_flag = 0;
    }
    else if(x + WINDOW_SIDE_WIDTH > SCREEN_WIDTH){
        frame_x = 0;
        frame_w = WINDOW_SIDE_WIDTH - (x + WINDOW_SIDE_WIDTH - SCREEN_WIDTH);
        win_x = x;
    }
    else{
        frame_x = 0;
        frame_w = WINDOW_SIDE_WIDTH;
        win_x = x;
    }

    if(y + WINDOW_SIDE_HEIGHT + WINDOW_TITLE_HEIGHT > SCREEN_HEIGHT && y + WINDOW_TITLE_HEIGHT < SCREEN_HEIGHT){
        //part out
        frame_y = STORE_Y;
        frame_h = WINDOW_SIDE_HEIGHT - (y + WINDOW_SIDE_HEIGHT + WINDOW_TITLE_HEIGHT - SCREEN_HEIGHT);
        win_y = y + WINDOW_TITLE_HEIGHT;
    }
    else if(y + WINDOW_TITLE_HEIGHT > SCREEN_HEIGHT){
        draw_flag = 0;
    }
    else{
        frame_y = STORE_Y;
        frame_h = WINDOW_SIDE_HEIGHT;
        win_y = y + WINDOW_TITLE_HEIGHT;
    }

    if(draw_flag == 1){
         __svgalib_cirrusaccel_mmio_ScreenCopy(frame_x, frame_y, win_x, win_y + offset, frame_w, frame_h); //left side
    }

    /* check if right side out of screen */
    draw_flag = 1;
    if(x + WINDOW_TITLE_WIDTH - WINDOW_SIDE_WIDTH < 0 && x + WINDOW_TITLE_WIDTH > 0){ //part out
        frame_x = - (x + WINDOW_TITLE_WIDTH - WINDOW_SIDE_WIDTH);
        frame_w = WINDOW_SIDE_WIDTH + x + WINDOW_TITLE_WIDTH - WINDOW_SIDE_WIDTH;
        win_x = 0;
    }
    else if(x + WINDOW_TITLE_WIDTH - WINDOW_SIDE_WIDTH > SCREEN_WIDTH){ //all out
        draw_flag = 0;
    }
    else if(x + WINDOW_TITLE_WIDTH - WINDOW_SIDE_WIDTH < SCREEN_WIDTH && x + WINDOW_TITLE_WIDTH > SCREEN_WIDTH){
        frame_x = 0;
        frame_w = WINDOW_SIDE_WIDTH - (x + WINDOW_TITLE_WIDTH - SCREEN_WIDTH);
        win_x = x;
    }
    else{
        frame_x = 0;
        frame_w = WINDOW_SIDE_WIDTH;
        win_x = x + WINDOW_TITLE_WIDTH - WINDOW_SIDE_WIDTH;
    }

    if(y + WINDOW_SIDE_HEIGHT + WINDOW_TITLE_HEIGHT > SCREEN_HEIGHT && y + WINDOW_TITLE_HEIGHT < SCREEN_HEIGHT){
        //part out
        frame_y = STORE_Y;
        frame_h = WINDOW_SIDE_HEIGHT - (y + WINDOW_SIDE_HEIGHT + WINDOW_TITLE_HEIGHT - SCREEN_HEIGHT);
        win_y = y + WINDOW_TITLE_HEIGHT;
    }
    else if(y + WINDOW_TITLE_HEIGHT > SCREEN_HEIGHT){
        draw_flag = 0;
    }
    else{
        frame_y = STORE_Y;
        frame_h = WINDOW_SIDE_HEIGHT;
        win_y = y + WINDOW_TITLE_HEIGHT;
    }

    if(draw_flag == 1){
         __svgalib_cirrusaccel_mmio_ScreenCopy(frame_x, frame_y, win_x, win_y + offset, frame_w, frame_h); //left side
    }

    /* check if down out of screen */
    draw_flag = 1;
    if(x < 0){
        frame_x = WINDOW_SIDE_WIDTH - x;
        frame_w = WINDOW_DOWN_WIDTH + x;
        win_x = 0;
    }
    else if(x + WINDOW_DOWN_WIDTH > SCREEN_WIDTH){
        frame_x = WINDOW_SIDE_WIDTH;
        frame_w = WINDOW_DOWN_WIDTH - (x + WINDOW_DOWN_WIDTH - SCREEN_WIDTH);
        win_x = x;
    }
    else{
        frame_x = WINDOW_SIDE_WIDTH;
        frame_w = WINDOW_DOWN_WIDTH;
        win_x = x;
    }

    if(y + WINDOW_TITLE_HEIGHT + WINDOW_SIDE_HEIGHT < SCREEN_HEIGHT
        && y + WINDOW_TITLE_HEIGHT + WINDOW_SIDE_HEIGHT + WINDOW_DOWN_HEIGHT > SCREEN_HEIGHT){
        //part out
        frame_y = STORE_Y + WINDOW_TITLE_HEIGHT;
        frame_h = WINDOW_DOWN_HEIGHT - (y + WINDOW_TITLE_HEIGHT + WINDOW_SIDE_HEIGHT + WINDOW_DOWN_HEIGHT - SCREEN_HEIGHT);
        win_y = y + WINDOW_TITLE_HEIGHT + WINDOW_SIDE_HEIGHT;
    }
    else if(y + WINDOW_TITLE_HEIGHT + WINDOW_SIDE_HEIGHT > SCREEN_HEIGHT){
        draw_flag = 0;
    }
    else{
        frame_y = STORE_Y + WINDOW_TITLE_HEIGHT;
        frame_h = WINDOW_DOWN_HEIGHT;
        win_y = y + WINDOW_TITLE_HEIGHT + WINDOW_SIDE_HEIGHT;
    }

    if(draw_flag == 1){
         __svgalib_cirrusaccel_mmio_ScreenCopy(frame_x, frame_y, win_x, win_y + offset, frame_w, frame_h); //left side
    }

    /* draw window */

    return 0;
}

void increase_active_window_prior(){
    int i;
    int target_prior = 0;
    /* check if is wake a window*/
    if(mouse_target_id >= 0){
        target_prior = window[mouse_target_id].prior;
    }

    for(i = 0; i < WINDOW_MAX_NUM; i ++){
        if(window[i].status == 1 && window[i].prior <= target_prior && mouse_target_id != i){
            window[i].prior ++;
            window[i].enable_cursor = 0;
        }
    }
}

void decrease_active_window_prior(){
    int i;
    int target_prior = 0;
    /* check if is wake a window*/
    if(mouse_target_id >= 0){
        target_prior = window[mouse_target_id].prior;
    }
    for(i = 0; i < WINDOW_MAX_NUM; i ++){
        if(window[i].status == 1 && window[i].prior > target_prior){
            window[i].prior --;
        }
    }
}

void try_to_get_new_window(mouse_location_type type){
    /* check if reach max */
    if(curr_win_num == WINDOW_MAX_NUM){
        win_init_x = 50;
        win_init_y = 50;
        return;
    }

    /* get a inactive window and set the prior to 0
        re-order the current window*/
    int id;
    for(id = 0; id < WINDOW_MAX_NUM; id ++){
        if(window[id].status == 0){
            break;
        }
    }
    window[id].prior = 0;
    increase_active_window_prior();

    /* set XY and active */
    window[id].x = win_init_x;
    window[id].y = win_init_y;
    window[id].status = 1;

    win_init_x += 50;
    win_init_y += 50;

    curr_win_num ++;

    terminal_t* terminal_pt;

    switch(type){
        case TERMINAL_ICON:
            terminal_pt = get_available_terminal();
            if(terminal_pt == NULL){
                break;
            }
            window[id].type = TERMINAL_ICON;
            window[id].content_pt = (uint32_t)(terminal_pt);
            window[id].enable_cursor = 1;
            terminal_pt->status = 1;
            terminal_switch(terminal_pt);
            break;

        case FILE_ICON:
            window[id].content_pt = (uint32_t)(&desktop_file[1]);
            window[id].type = FILE_ICON;
            break;

        default:
            break;
    }
    
}

void draw_window(gui_window* window){
    switch(window->type){
        case TERMINAL_ICON:
            draw_terminal(window);
            break;

        case FILE_ICON:
            draw_directory(window);
            break;
    } 
    draw_gui_window_frame(window->x, window->y);   
}

void show_windows(){
    int id;
    int prior;
    for(prior = curr_win_num; prior >= 0; prior --){
        for(id = 0; id < WINDOW_MAX_NUM; id ++){
            if(window[id].prior == prior && window[id].status == 1){
                draw_window(&(window[id]));
            }
        }
    }
}

int check_mouse_gui(int x, int y){
    if(x >= TERMINAL_ICON_X && x <= TERMINAL_ICON_X + TERMINAL_ICON_WIDTH
    && y >= TERMINAL_ICON_Y && y <= TERMINAL_ICON_Y + TERMINAL_ICON_HEIGHT){
        return TERMINAL_ICON;
    }

    if(x > window[mouse_target_id].x + WINDOW_CANCEL_ICON_X 
    && x < window[mouse_target_id].x + WINDOW_CANCEL_ICON_X + WINDOW_CANCEL_ICON_WIDTH
    && y > window[mouse_target_id].y + WINDOW_CANCEL_ICON_Y
    && y < window[mouse_target_id].y + WINDOW_CANCEL_ICON_Y + WINDOW_CANCEL_ICON_HEIGH){
        return WINDOW_CANCEL;
    }

    int id;
    for(id = 0; id < WINDOW_MAX_NUM; id ++){
        if(window[id].status == 1){
            if(x > window[id].x && x < window[id].x + WINDOW_TITLE_WIDTH 
            && y > window[id].y && y < window[id].y + WINDOW_TITLE_HEIGHT){
                mouse_target_id = id;
                return WINDOW_TITLE;
            }
        }
    }

    for(id = 0; id < curr_file; id ++){
        if(x > desktop_file[id].x && x < desktop_file[id].x + DESKTOP_GRID_X
        && y > desktop_file[id].y && y < desktop_file[id].y + DESKTOP_GRID_Y){
            mouse_target_file = id;
            return FILE_ICON;
        }
    }

    return BACKGROUND;
}

void window_cancel(int win_id){
    window[win_id].status = 0;
    window[win_id].prior = WINDOW_MAX_NUM;
    terminal_t* terminal_pt;
    switch(window->type){
        case TERMINAL_ICON:
            terminal_pt = (terminal_t* )window[win_id].content_pt;
            terminal_pt->status = 0;
            window[win_id].enable_cursor = 0;
            break;
        
        case FILE_ICON:
            curr_file = 1;
            break;

        default:
            break;
    }
    //window[win_id].content_pt = NULL;
    decrease_active_window_prior();
    curr_win_num --;
}

void wake_window(){
    window[mouse_target_id].prior = 0;
    increase_active_window_prior();
}

void mouse_windows_release_interact_handler(int x, int y){
    on_title = 0;
    if(mouse_target_id == -1){
        return;
    }
    /* check if is cancel */
    window_cancel(mouse_target_id);
    mouse_target_id = -1;
    mouse_target_file = -1;
}

void window_move(int x, int y){
    if(mouse_target_id == -1){
        return;
    }
    window[mouse_target_id].x += x;
    window[mouse_target_id].y += y;
}

void mouse_windows_press_interact_handler(){
    if(mouse_target_id == -1){
        return;
    }
    /* check if is in title */
    wake_window();
    if(window[mouse_target_id].type == TERMINAL_ICON){
        window[mouse_target_id].enable_cursor = 1;
        terminal_t* terminal_pt = (terminal_t*)(window[mouse_target_id].content_pt);
        terminal_switch(terminal_pt);
    }
}

