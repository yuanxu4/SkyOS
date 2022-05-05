/* 
 * sb16.h : the .c file for sb16 driver, get the infomation from the manual/doc for sb0350.pdf 
 * created: 05/01/2022
 * by: Yuan Xu
 * 
 * reference: https://wiki.osdev.org/Sound_Blaster_16
 *            http://qzx.com/pc-gpe/sbdsp.txt
 *            https://github.com/margaretbloom/sb16-wav
 */
#include "sb16.h"
#define LOBYTE(w)    ((uint8_t)(((uint16_t)(w)) & 0xff))
#define HIBYTE(w)    ((uint8_t)((((uint16_t)(w)) >> 8) & 0xff))
volatile int ack_flag = 1;
//64kb
int8_t DMA_buff[BUFF_DIM][BUFF_SIZE] __attribute__((aligned(32768))) = {};
int32_t fd;
uint8_t* temp[2];
volatile static int end_flag;
/**************************** help function **********************************/
uint8_t dsp_read() {
    // check the status bit bit
    while (!(inb(SB16_READ_STATU) & SB16_READY)){ };
    return (uint8_t) inb(SB16_READ_PORT);
}
void dsp_write(uint8_t data) {
    // check the status bit bit
    while (inb(SB16_WRITE_STATU) & SB16_READY){ };
    outb(data, SB16_WRITE_PORT);
    return;
}
void dsp_reset(){
    cli();
    uint8_t i = 0;
    outb(1, DSP_RESET);
    //wait for 3ms 
    while(i < 30){ ++i; }
    outb(0, DSP_RESET);
    //the HW need some time(10ms) to set 
    while(dsp_read() != DSP_RESET_YES) {};
    sti();
}

void sb16_set_mode(uint16_t buff, uint8_t page, uint16_t frequency, 
                   uint8_t bcommand, uint8_t bmode,uint8_t channel) {
    //the wav mode is 16bit stereo
    if(bmode == WAV_16_2_MODE) {
        //set DMA
        outb(0x04+channel, DMA_ABLE_PORT_16); //disable the channel
        outb(1,DMA_FLIP_PORT_16); //write any value into flip port
        outb(0x58+channel, DMA_MODE_PORT_16); //set the DMA mode
        outb(page, DMA_PAGE_PORT_16);//set the page
        outb(LOBYTE(buff), DMA_POS_PORT_16);//set the position
        outb(HIBYTE(buff), DMA_POS_PORT_16);
        outb(LOBYTE(BUFF_SIZE-1), DMA_LEN_PORT_16);
        outb(HIBYTE(BUFF_SIZE-1), DMA_LEN_PORT_16);
        outb(channel, DMA_ABLE_PORT_16); //enable the channel
        //set DSP
        dsp_write(DSP_O);
        dsp_write(HIBYTE(frequency));
        dsp_write(LOBYTE(frequency));
        dsp_write(bcommand);
        dsp_write(bmode);
        dsp_write(LOBYTE(BLOC_SIZE-1));
        dsp_write(HIBYTE(BLOC_SIZE-1));
    } else if(bmode == WAV_8_2_MODE) {
        printf("not support yet");
        return;
    }
}

uint8_t getpage() {
    return (uint8_t)((uint32_t)DMA_buff >> 16);
}
uint16_t getoff() {
    return (uint16_t)(((uint32_t)DMA_buff >> 1)%(BUFF_SIZE*2));
}
int32_t check_magicwav(uint8_t* file_info) {
    uint8_t magic_num[4];
    memcpy(magic_num,(file_info+8),4);
    if(*(uint32_t*)magic_num != MAGIC_WAV) {
        printf("magic number not same");
        return -1;
    }
    return 0;
}
uint16_t getrate(uint8_t* file_info) {
    return *(uint16_t*)(file_info + 24);
}

int32_t sb16_init(uint8_t* file_info) {
    ack_flag = 1;
    enable_irq(SB16_IRQ_NUM);
    if(-1 == check_magicwav(file_info)) {
        return -1;
    }
    uint16_t rate = getrate(file_info);
    uint8_t page_num = getpage();
    uint16_t off = getoff();
    sb16_set_mode(off, page_num, rate, 0xB6, WAV_16_2_MODE, 1);
    return 0;
}

void sb16_stop() {
    end_flag = 1;
    ack_flag = 1;
    dsp_reset();
    disable_irq(SB16_IRQ_NUM);
}

void sb16_interrupt_handler() {
    cli();
    if(ack_flag  == 1 ) {
        ack_flag = 0;
    }else {
        ack_flag = 1;
    }
    if (0 >= read(fd, temp[ack_flag], BUFF_SIZE)) {
                end_flag = 1;
                sb16_stop();
            }
    //acknowlege the transfer by read port
    inb(SB16_ACK);
    send_eoi(SB16_IRQ_NUM);
    sti();
}

void sb16_wait() {
    while(end_flag){};
}


/****************************** play music function *****************************/
int play_music(uint8_t* fname) {

    uint8_t f_info[44];
    end_flag = 0;
    fd = open (fname);
    read(fd, f_info, 44);
    sb16_init(f_info);
    temp[0] = (uint8_t*)DMA_buff;
    temp[1] = (uint8_t*)((int32_t)DMA_buff + BUFF_SIZE);
    read(fd, temp[0], BUFF_SIZE);
    read(fd, temp[1], BUFF_SIZE);
    return 0;
}
