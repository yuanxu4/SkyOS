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
 * Filename:      pit.h
 * History:
 */

#ifndef PIT_H
#define PIT_H

#ifndef ASM

#include "types.h"
/* set pit frequency to 100 hz */
#define PIT_FREQUENCY 100
#define PIT_IRQNUM  0
/* just for rtc init */
void pit_init();

/* the handler for rtc interrupt */
void pit_interrupt_handler();


#endif

#endif


