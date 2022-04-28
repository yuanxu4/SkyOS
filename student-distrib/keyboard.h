/* keyboard.h - Defines used in interactions with the keyboard
 * controller
 * created: 3/20/2022
 * by: Haina Lou
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifndef ASM

#include "types.h"
#include "task.h"
#define KEYBOARD_PORT 0x60
#define KEYBARD_IRQ 0x01
#define keynum 0x3E
#define kb_bufsize 0x80

#define ENTER               0X1c
#define ENTER_release       0x9C
#define l_shift             0x2A
#define l_shift_release     0xAA
#define r_shift             0x36
#define r_shift_release     0xB6
#define caps                0x3A
#define caps_release        0xBA
#define control             0x1D
#define control_release     0x9D
#define L                   0x26
#define ALT                 0x38  
#define ALT_RELEASE         0xB8 
#define F1                  0X3B
#define F2                  0x3C
#define F3                  0x3D    
#define DELETE_NUM          14


#define MAX_TERMINAL_NUM    3
#define VIDEO_MEM_ADDR      0xB8000
#define VIDEO_MEM_SIZE      0x1000 
// #define SIZE_4KB            4096
// #define VIDEO_MEM_INDEX     VIDEO_MEM_ADDR/SIZE_4KB
#define TERM1_ADDR  (0xB8000 + 1*VIDEO_MEM_SIZE)
#define TERM2_ADDR  (0xB8000 + 2*VIDEO_MEM_SIZE)
#define TERM3_ADDR  (0xB8000 + 3*VIDEO_MEM_SIZE) 
#define TERMINAL_DIRECT_ADDR 0xb7000
#define ONTO_DISPLAY_WRAP(code) {               \
    video_mem = (char*) TERMINAL_DIRECT_ADDR;   \
    code;                                       \
    video_mem = (char*) VIDEO;                  \
}        

typedef struct terminal_t terminal_t;
struct terminal_t{

    uint32_t terminal_id;
    uint8_t keyboard_buf[kb_bufsize];
    uint8_t character_num;
    uint32_t page_addr;
    uint32_t cursor_x;
    uint32_t cursor_y;
    // PCB_t *current_process;
    uint32_t num_task;
    volatile uint32_t enter_flag;

};

void keyboard_init(void);
int32_t terminal_init();
int32_t terminal_open(const uint8_t *filename);
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t terminal_close(int32_t fd);


int32_t terminal_switch(terminal_t *terminal_next);



#endif

#endif
