/*
 * tab:4
 *
 * rtc.c - function file for the real-time-clock(rtc)
 *
 * "Copyright (c) 2020 by Yuan Xu."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * Author:        Yuan Xu
 * Version:       1
 * Filename:      rth.c
 * History:
 */

#include "i8259.h"
#include "rtc.h"
#include "lib.h"

/*
 * Port 0x70 is used to specify an index or "register number", and to disable NMI. 
 * Port 0x71 is used to read or write from/to that byte of CMOS configuration space. 
 */
#define RTC_INDEX_PORT  0x70
#define RTC_DATA_PORT   0x71

/* with the NMI disable (by setting the 0x80 bit)*/
#define RTC_REGISTER_A  0x8A
#define RTC_REGISTER_B  0x8B
#define RTC_REGISTER_C  0x8C

/* frequency =  32768 >> (rate-1) */
#define RTC_RATE_2      15  

/* RTC frequency limi t*/
#define RTC_MAX_FRE     1024
#define RTC_MIN_FRE     2

static volatile int32_t rtc_happen_flag;


void rtc_reset_R3();

/* 
 * rtc_init 
 *
 * Initialize the rtc 
 * enable the rtc interrupt in the IRG num 8 
 * initial the rate of the rtc into 1024hz
 * 
 * reference: https://wiki.osdev.org/RTC 
 * this is a very useful website 
 */
void rtc_init() {
    uint32_t flags;

    //mask the interrupt 
    cli_and_save(flags);
    outb(RTC_REGISTER_B, RTC_INDEX_PORT);   // set the index of register B 
    uint8_t prev = inb(RTC_DATA_PORT);      // get the value of register B
    outb(RTC_REGISTER_B, RTC_INDEX_PORT);   // reset the index
    outb (prev | 0x40, RTC_DATA_PORT);      // make the register B six bit to 1
    restore_flags(flags);
        
    //set the frequence 
    cli_and_save(flags);
    outb(RTC_REGISTER_A, RTC_INDEX_PORT);   // set the index of register A
    prev = inb(RTC_DATA_PORT);              // get the value of register A
    outb(RTC_REGISTER_A, RTC_INDEX_PORT);   // reset the index
    outb((prev & 0xF0) | RTC_RATE_2, RTC_DATA_PORT); // set the rate to the 2HZ
    restore_flags(flags);
    
}

/* 
 * rtc_interrupt_handler
 *
 * the hander for the rth, we just need to call a test function in the lib for checkpoint one
 * 
 */
 void rtc_interrupt_handler() {
    /* clear the flag */
    rtc_happen_flag = 0;
    rtc_reset_R3();  
    send_eoi(RTC_IRQ);

}

/* 
 *rtc_read_R3
 *
 * What is important is that if register C is not read after an IRQ 8, 
 * then the interrupt will not happen again. So we need to read the register C
 * 
 * reference: https://wiki.osdev.org/RTC 
 * this is a very useful website 
 */
void rtc_reset_R3() {

    uint32_t flags;
    cli_and_save(flags);              
    outb(RTC_REGISTER_C, RTC_INDEX_PORT);  // set the index of register C
    inb(RTC_DATA_PORT);                 // read the register C
    restore_flags(flags);

}

/* 
 *rtc_open
 *
 * this is the open function for rtc, which set the rtc into a 2hz frequency 
 * 
 * input: filename -- is just the name of the rtc we are suppose to open, but useless for this function
 * output: none
 * return: -1 fail, 0 success
 * reference: https://wiki.osdev.org/RTC 
 * this is a very useful website 
 */
int32_t rtc_open(const uint8_t* filename) {
    /* start the rtc and set the frequency to 2hz*/
    rtc_init();
    return 0;
}

/* 
 * rtc_read
 *
 * thisis the read function for the rtc, and for checkpoint 2 it just return 0 after a interrupt
 * input: fd -- this is the file descriptor which is not use for this read
 *        buf -- not use for checkpoint 2
 *        nbytes -- not use for checkpoint 2
 * output: return after the rtc_interrupt_handler 
 * return: -1 for failure, 0 for success
 * 
 * 
 * reference: https://wiki.osdev.org/RTC 
 * this is a very useful website 
 */
int32_t rtc_read( int32_t fd, void* buf, int32_t nbytes ) {
    rtc_happen_flag = 1;
    while(rtc_happen_flag){

    }
    return 0;
    

}

/* 
 *rtc_write
 *
 * this is the write function for the rtc，which set the frequency for the rtc interrupt
 * 
 * input: fd -- this is the file descriptor which is not use for this read
 *        buf -- contain the rate for the rtc( it should be power of 2, and size is 4 bytes)
 *        nbytes -- must be 4
 * output: change the rtc frequency into the buf
 * return: -1 for failure and 0 for success
 *  
 * reference: https://wiki.osdev.org/RTC 
 * this is a very useful website 
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    /* check for normal argument */
    if( nbytes != 4 || buf == NULL)
        return -1;
    int32_t frequency = *(int32_t*) buf;
    int32_t rate = RTC_RATE_2 + 1;
    /* check for the valid rate*/
    if( (RTC_MAX_FRE < frequency) || (RTC_MIN_FRE > frequency) || (frequency & (frequency - 1) != 0))
        return -1;
    /* frequency =  32768 >> (rate-1) */
    while(frequency != 1) {
        frequency = frequency >> 1;
        rate--;
    }
    uint32_t flags;
    cli_and_save(flags);
    outb(RTC_REGISTER_A, RTC_INDEX_PORT);   // set the index of register A
    uint8_t prev = inb(RTC_DATA_PORT);              // get the value of register A
    outb(RTC_REGISTER_A, RTC_INDEX_PORT);   // reset the index
    outb((prev & 0xF0) | rate, RTC_DATA_PORT); // set the rate to the frequency
    restore_flags(flags);

}

/* 
 * rtc_close
 *
 * thisis the read function for the rtc, and for checkpoint 2 it just return 0 after a interrupt
 * input: fd -- this is the file descriptor which is not use for this read
 * output: return 0
 * return: -1 for failure, 0 for success
 * 
 * 
 * reference: https://wiki.osdev.org/RTC 
 * this is a very useful website 
 */
int32_t rtc_close( int32_t fd) {
    return 0;
}
