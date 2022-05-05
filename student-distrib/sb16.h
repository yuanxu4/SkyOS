/* 
 * sb16.h : the .h file for the sb16 driver, get the infomation from the manual/doc for sb0350.pdf 
 * created: 05/01/2022
 * by: Yuan Xu
 * reference: https://wiki.osdev.org/Sound_Blaster_16
 *            http://qzx.com/pc-gpe/sbdsp.txt
 *            https://github.com/margaretbloom/sb16-wav
 */
#ifndef _SB16_H
#define _SB16_H

#include "types.h"
#include "lib.h"
#include "i8259.h"

#define SB16_IRQ_NUM        0x05
#define SB16_READY          0x80 

#define SB16_ACK            0x22F

#define SB16_READ_STATU     0x22E
#define SB16_READ_PORT      0x22A
#define SB16_WRITE_STATU    0x22C
#define SB16_WRITE_PORT     0x22C

#define DSP_RESET           0x226
#define DSP_RESET_YES       0xAA
#define MAGIC_WAV           0x45564157
#define MAGIC_INDEX         8

#define BUFF_SIZE           65536/2
#define BUFF_DIM            2
#define BLOC_SIZE          BUFF_SIZE/BUFF_DIM
#define WAVINFO_SIZE        44

#define WAV_16_2_MODE       0x30
#define WAV_8_2_MODE        0x00

#define DMA_ABLE_PORT_16    0xD4           
#define DMA_ABLE_PORT_8     0x0A           
#define DMA_FLIP_PORT_16    0xD8           
#define DMA_FLIP_PORT_8     0x0C           
#define DMA_MODE_PORT_16    0xD6           
#define DMA_MODE_PORT_8     0x0B           
#define DMA_PAGE_PORT_16    0x8B           
#define DMA_PAGE_PORT_8     0x83           
#define DMA_POS_PORT_16     0xC4           
#define DMA_POS_PORT_8      0x02           
#define DMA_LEN_PORT_16     0xC6           
#define DMA_LEN_PORT_8      0x03           

#define DSP_O               0x41


void sb16_interrupt_handler();

int play_music(uint8_t* fname);

void sb16_stop();
void sb16_wait();



#endif
