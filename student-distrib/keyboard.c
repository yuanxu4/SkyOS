/* keyboard.h - Defines used in interactions with the keyboard
 * controller
 * created: 3/20/2022
 * by: Haina Lou
 */

#include "keyboard.h"
#include "lib.h"
#include  "i8259.h"

/* Keyboard_handler called when IDT want to handle the interrupt
 * 
 * Inputs: void
 * Outputs: void
 * Side Effects: handle keyboard output to terminal when
 * interrupt occurs
 */
asmlinkage void keyboard_handler(void){
    uint8_t scancode;
    /* scancode input from keyboard */
    scancode = inb(KEYBOARD_PORT);
    /* handle scancode and print to terminal */
    scancode_handle(scancode);

    send_eoi(KEYBARD_IRQ);
}

void scancode_handle(uint8_t scancode){
    uint8_t output_char;
    // if (scancode > SIMPLE_CASE){
        
    // }

    /* press Enter */
    if (scancode == ENTER){
        putc('\n');
    }
    /* if scancode is in right range*/
    if(scancode>=0 && scancode <= SIMPLE_CASE){
        output_char = scancode_simple_lowcase[scancode];
        putc(output_char);
    }
}