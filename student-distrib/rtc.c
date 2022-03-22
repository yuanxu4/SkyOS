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
#define RTC_RATE_1024   6  
#define RTC_RATE_256    8  
#define RTC_RATE_128    9 
#define RTC_RATE_2      15  

void rtc_read_R3();

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
    outb((prev & 0xF0) | RTC_RATE_256, RTC_DATA_PORT); // set the rate to the 256HZ
    restore_flags(flags);
    
}

/* 
 * rtc_interrupt_handler
 *
 * the hander for the rth, we just need to call a test function in the lib for checkpoint one
 * 
 */
 void rtc_interrupt_handler() {
    
    test_interrupts();
    rtc_read_R3();  
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
void rtc_read_R3() {

    uint32_t flags;
    cli_and_save(flags);
    outb(RTC_REGISTER_C, RTC_INDEX_PORT);  // set the index of register C
    inb(RTC_DATA_PORT);                 // read the register C
    restore_flags(flags);

}
