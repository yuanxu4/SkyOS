#include "mouse.h"
#include "lib.h"
#include "asmlink.h"
#include "GUI/gui.h"
#include "x86_desc.h"
#include "i8259.h"

static uint8_t mouse_data[3];
static int16_t mouse_X;
static int16_t mouse_Y;

int left_press_flag = 0;
int press_on_title = 0;
int hit_boarder_flag_x = 0;
int hit_boarder_flag_y = 0;
static int16_t last_press_X = -1;
static int16_t last_press_Y = -1;

static mouse_location_type mouse_location = BACKGROUND;
static mouse_location_type last_mouse_press_location = BACKGROUND;

// Waiting to Send Bytes to Port 0x60 and 0x64
// All output to port 0x60 or 0x64 must be preceded by waiting 
// for bit 1 (value=2) of port 0x64 to become clear. Similarly, 
// bytes cannot be read from port 0x60 until bit 0 (value=1) of 
// port 0x64 is set. See PS2 Keyboard for further details.
void send_IO_PORT(uint8_t port, uint8_t data){
    //All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
    int wait_flag = 1;
    while(wait_flag != 0){
        wait_flag = inb(0x64);
        wait_flag &= 0x02;
    }
    outb(data, port);
}

uint8_t read_from_PORT(uint8_t port){
    //All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
    int wait_flag = 0;
    while(wait_flag != 1){
        wait_flag = inb(0x64);
        wait_flag &= 0x01;
    }
    return inb(port);
}

// 0xD4 Byte, Command Byte, Data Byte
// Sending a command or data byte to the mouse (to port 0x60) 
// must be preceded by sending a 0xD4 byte to port 0x64 (with 
// appropriate waits on port 0x64, bit 1, before sending each 
// output byte). Note: this 0xD4 byte does not generate any ACK
// , from either the keyboard or mouse.
void send_cmd(uint8_t cmd){
    int ack_flag = 1;
    send_IO_PORT(0x64, 0xD4);
    send_IO_PORT(0x60, cmd);
    while(ack_flag != ACK){
        ack_flag = read_from_PORT(0x60);
    }
}

// mouse_location_type check_press_area(){
//     if(mouse_X >= TERMINAL_ICON_X && mouse_X <= TERMINAL_ICON_X + TERMINAL_ICON_WIDTH
//     && mouse_Y >= TERMINAL_ICON_Y && mouse_Y <= TERMINAL_ICON_Y + TERMINAL_ICON_HEIGHT
//     && last_press_X >= TERMINAL_ICON_X && last_press_X <= TERMINAL_ICON_X + TERMINAL_ICON_WIDTH
//     && last_press_Y >= TERMINAL_ICON_Y && last_press_Y <= TERMINAL_ICON_Y + TERMINAL_ICON_HEIGHT){
//         return TERMINAL_ICON;
//     }

//     if(check_inside_window(mouse_X, mouse_Y)){
//         return WINDOW;
//     }

//     return BACKGROUND;
// }

mouse_location_type check_area(){
    return check_mouse_gui(mouse_X, mouse_Y);
}

void mouse_left_release_handler(){
    switch(mouse_location){
        case BACKGROUND:
            break;

        case TERMINAL_ICON:
            if(last_mouse_press_location != TERMINAL_ICON){
                break;
            }
            try_to_get_new_window(TERMINAL_ICON);
            break;

        case WINDOW_CANCEL:
            if(last_mouse_press_location != WINDOW_CANCEL){
                break;
            }
            mouse_windows_release_interact_handler(mouse_X, mouse_Y, WINDOW_CANCEL);

        case FILE_ICON:
            if(last_mouse_press_location != FILE_ICON){
                break;
            }
            try_to_get_new_window(FILE_ICON);

        case WINDOW_TITLE:
            mouse_windows_release_interact_handler(mouse_X, mouse_Y, WINDOW_TITLE);
            break;

        default:
            break;
    }
    press_on_title = 0;
 }

void mouse_left_press_handler(){
    switch(mouse_location){
        case WINDOW_TITLE:
            press_on_title = 1;
            mouse_windows_press_interact_handler();
            break;

        case FILE_ICON:
            last_mouse_press_location = FILE_ICON;
            break;

        case WINDOW_CANCEL:
            last_mouse_press_location = WINDOW_CANCEL;
            break;

        case TERMINAL_ICON:
            last_mouse_press_location = TERMINAL_ICON;
            break;

        default:
            last_mouse_press_location = BACKGROUND;
            break;
    }
}

asmlinkage void mouse_IRQ_handler(){
    send_eoi(Mouse_IRQ);
    // printf("status reg: %x\n", inb(0x64));
    if (0 == (inb(0x64) & 0x1)) {
        return;
    }
    if (0 == (inb(0x64) & 0x20)) {
        return;
    }

    /* get data from mouse */
    mouse_data[0] = read_from_PORT(0x60);
    mouse_data[1] = read_from_PORT(0x60);
    mouse_data[2] = read_from_PORT(0x60);
    
    /* update current position */
    mouse_X += (int8_t)mouse_data[1];
    mouse_Y -= (int8_t)mouse_data[2];
    //handler out of screen
    if(mouse_X >= SCREEN_WIDTH){
        mouse_X = SCREEN_WIDTH;
        hit_boarder_flag_x = 1;
    }
    else if(mouse_X <= 0){
        mouse_X = 0;
        hit_boarder_flag_x = 1;
    }

    if(mouse_Y >= SCREEN_HEIGHT){
        mouse_Y = SCREEN_HEIGHT;
        hit_boarder_flag_y = 1;
    }
    else if(mouse_Y <= 0){
        mouse_Y = 0;
        hit_boarder_flag_y = 1;
    }

    mouse_location = check_area();

    /* hanlder left event */
    if(mouse_data[0] & 0x1){
        mouse_left_press_handler();
        left_press_flag = 1;
        last_press_X = mouse_X;
        last_press_Y = mouse_Y;
    }
    else{
        if(left_press_flag == 1){
            mouse_left_release_handler();
        }
        left_press_flag = 0;
        last_press_X = -1;
        last_press_Y = -1;
    }

    if(hit_boarder_flag_x == 0 && hit_boarder_flag_y == 0 && press_on_title == 1){
        window_move((int8_t)mouse_data[1], -(int8_t)mouse_data[2]);
    }
    else if(hit_boarder_flag_x == 1 && press_on_title == 1){
        window_move(0, -(int8_t)mouse_data[2]);
    }
    else if(hit_boarder_flag_y == 1 && press_on_title == 1){
        window_move((int8_t)mouse_data[1], 0);
    }

    // printf("X: %d, Y: %d, info: %x\n", mouse_X, mouse_Y, mouse_data[0]);
    Cursor_Set_XY(mouse_X, mouse_Y);
    hit_boarder_flag_x = 0;
    hit_boarder_flag_y = 0;
}

// Set Compaq Status/Enable IRQ12
// On some systems, the PS2 aux port is disabled at boot. Data 
// coming from the aux port will not generate any interrupts. 
// To know that data has arrived, you need to enable the aux port 
// to generate IRQ12. There is only one way to do that, which involves 
// getting/modifying the "compaq status" byte. You need to send the command 
// byte 0x20 ("Get Compaq Status Byte") to the PS2 controller on port 0x64. 
// If you look at RBIL, it says that this command is Compaq specific, but this 
// is no longer true. This command does not generate a 0xFA ACK byte. The very 
// next byte returned should be the Status byte. (Note: on some versions of Bochs
// , you will get a second byte, with a value of 0xD8, after sending this command
// , for some reason.) After you get the Status byte, you need to set bit number 1 
// (value=2, Enable IRQ12), and clear bit number 5 (value=0x20, Disable Mouse Clock). 
// Then send command byte 0x60 ("Set Compaq Status") to port 0x64, followed by the 
// modified Status byte to port 0x60. This might generate a 0xFA ACK byte from the keyboard.
void init_mouse(){
    uint8_t compaq;
    /* reset mouse */
    send_cmd(Reset);

    /* Get Compaq Status Byte and set*/
    send_IO_PORT(0x64, 0x20);
    compaq = read_from_PORT(0x60);
    compaq = 0x61 | 0x2;
    compaq = compaq & ~(0x20);
    send_IO_PORT(0x64, 0x60);
    send_IO_PORT(0x60, compaq);

    /* Set default */
    send_cmd(Set_Defaults);

    /* Enable */
    send_cmd(Enable_Packet_Streaming);

    /* Install IRQ handler */
    
}
