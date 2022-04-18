/* keyboard.h - Defines used in interactions with the keyboard
 * controller
 * created: 3/20/2022
 * by: Haina Lou
 */
#ifndef KEYBOARD_H
#define KEYBOARD_H

#ifndef ASM

#include "types.h"
#define KEYBOARD_PORT 0x60
#define KEYBARD_IRQ 0x01
#define keynum 0x3B
#define kb_bufsize 0x80

#define ENTER 0X1c
#define ENTER_release 0x9C
#define l_shift 0x2A
#define l_shift_release 0xAA
#define r_shift 0x36
#define r_shift_release 0xB6
#define caps 0x3A
#define caps_release 0xBA
#define control 0x1D
#define control_release 0x9D
#define L               0x26
#define DELETE_NUM      14

void keyboard_init(void);
int32_t terminal_init();
int32_t terminal_open(const uint8_t *filename);
int32_t terminal_read(int32_t fd, void *buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t terminal_close(int32_t fd);

#endif

#endif
