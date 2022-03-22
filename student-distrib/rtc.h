/*
 * tab:4
 *
 * rtc.h - function file for the real-time-clock(rtc)
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
 * Filename:      rtc.h
 * History:
 */

#ifndef RTC_H
#define RTC_H

#define RTC_IRQ 8

#include "types.h"
/* just for rtc init */
void rtc_init();
/* the handler for rtc interrupt */
void rtc_interrupt_handler();
/* the refresh for rtc*/
void rtc_read_R3();

#endif




