/* keyboard.h - Defines used in interactions with the keyboard
 * controller
 * created: 3/20/2022
 * by: Haina Lou
 */

#include "keyboard.h"
#include "lib.h"
#include  "i8259.h"
/* lowcase character and numbers */
const char scancode_simple_lowcase[SIMPLE_CASE]={                
    0,0,'1','2','3','4','5','6','7','8',    // 0x00-0x09
    '9','0','-','=',0,0,'q','w','e','r','t',// 0x0a-0x14
    'y','u','i','o','p','[',']',0,0,'a','s',
    'd','f','g','h','j','k','l',';',0,0,0,0,
    'z','x','c','v','b','n','m',',','.','/'     // 0x2c-0x35   
};

void scancode_output(uint8_t scancode);

/* keyboard init
 * 
 * Inputs: void
 * Outputs: void
 * Side Effects: handle keyboard output to terminal when
 * interrupt occurs
 */
void keyboard_init(void){
    enable_irq(KEYBARD_IRQ);
}


/* Keyboard_handler() 
 * called when IDT want to handle the interrupt
 * 
 * Inputs: void
 * Outputs: void
 * Side Effects: handle keyboard output to terminal when
 * interrupt occurs
 */
void keyboard_handler(void){
    uint8_t scancode;
    /* read scancode input from keyboard */
    scancode = inb(KEYBOARD_PORT);
    /* handle scancode and print to terminal */
    scancode_output(scancode);

    send_eoi(KEYBARD_IRQ);
}

/* scancode_output()
 * 
 * Inputs: void
 * Outputs: void
 * Side Effects: output corresponding keycode to console
 * interrupt occurs
 */
void scancode_output(uint8_t scancode){
    uint8_t output_char;
    // if (scancode > SIMPLE_CASE){
        
    // }

    /* press Enter */
    if (scancode == ENTER){
        putc('\n');
    }
    /* if scancode is in right range*/
    if(scancode <= SIMPLE_CASE){
        /* find corresponding keycode */
        output_char = scancode_simple_lowcase[scancode];
        putc(output_char);
    }
}
