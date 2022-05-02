#ifndef MOUSE_H
#define MOUSE_H

/* libs */

/* magic number */
#define Reset   0xFF
#define Resend  0xFE
#define Set_Defaults    0xF6
#define Disable_Packet_Streaming    0xF5
#define Enable_Packet_Streaming     0xF4
#define Set_Sample_Rate 0xF3
#define Get_mouseID     0xF2
#define Request_Single_Packet       0xEB
#define Status_Request  0xE9
#define Set_Resolution  0xE8
#define ACK     0xFA

#define Mouse_IRQ 12

typedef enum{
    BACKGROUND = 0,
    TERMINAL_ICON = 1,
    WINDOW_TITLE = 2,
    WINDOW_CANCEL = 3,
    FILE_ICON = 4,
} mouse_location_type;

/* function defines */
void init_mouse();

#endif
