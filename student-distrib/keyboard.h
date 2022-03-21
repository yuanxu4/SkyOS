/* keyboard.h - Defines used in interactions with the keyboard
 * controller
 * created: 3/20/2022
 * by: Haina Lou
 */

#include "types.h"
#define KEYBOARD_PORT   0x60
#define KEYBARD_IRQ     0x01
#define SIMPLE_CASE     0x35
#define ENTER           0X1c

const char scancode_simple_lowcase[SIMPLE_CASE]{                
    0,0,'1','2','3','4','5','6','7','8',    // 0x00-0x09
    '9','0','-','=',0,0,'q','w','e','r','t',// 0x0a-0x14
    'y','u','i','o','p','[',']',0,0,'a','s',
    'd','f','g','h','j','k','l',';',0,0,0,0,
    'z','x','c','v','b','n','m',',','.','/'     // 0x2c-0x35   
}
