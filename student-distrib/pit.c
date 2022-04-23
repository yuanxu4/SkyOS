/*
 * tab:4
 *
 * pit.c - function file for the PIT
 *
 * "Copyright (c) 2020 by Haina Lou."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * Author:        Haina Lou
 * Version:       1
 * Filename:      pit.c
 * History:
 */

#include "pit.h"
#include "lib.h"
#include "i8259.h"

/* 
 *  pit_init 
 *
 * Initialize the pit 
 * enable the pit interrupt in the IRG num 0
 * initial the rate of the rtc into 1024hz
 * 
 * reference: https://wiki.osdev.org/Programmable_Interval_Timer
 */
void pit_init()
{
    /* Calculate our divisor */
    uint16_t divisor = 1193180/PIT_FREQUENCY;       
    /* Set our command byte 0x36 */
    outb(0x36, 0x43);             
    /* Set low byte of divisor  0x40 is dataport */
    outb((uint8_t)(divisor & 0xFF), 0x40);   
    /* Set high byte of divisor */
    outb((uint8_t)(divisor >> 8), 0x40);     
    enable_irq(PIT_IRQNUM);
    return 0;
}

/* 
 * pit_interrupt_handler
 *
 * the hander for the pic
 * every interrupt will schedule
 * 
 */
 void pit_interrupt_handler() 
{
    return 0;
}

