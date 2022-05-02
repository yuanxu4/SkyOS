/*
 * vga.c
 *
 * Description:
 *  the implementation for vga
 *  this file is a adapted version from the svgalib https://www.svgalib.org
 *
 * ECE 391 SP 2022
 * History:
 * create file              - Apr 19, Yuan
 *    
 */
/* VGAlib version 1.2 - (c) 1993 Tommy Frandsen                    */
/*                                                                 */
/* This library is free software; you can redistribute it and/or   */
/* modify it without any restrictions. This library is distributed */
/* in the hope that it will be useful, but without any warranty.   */

/* Cirrus support Copyright (C) 1993 Harm Hanemaayer */

/*
 * Dec 1994:
 * 
 * Mode setting rewritten with more SVGA generalization and increased
 * flexibility. Supports 542x/543x.
 * Uses interfaces in timing.h and vgaregs.h. Only saves/restores
 * extended registers that are actually changed for SVGA modes.
 */

#include "vga.h"


#define B16MASK 0xFFFF
#define SHIFT_16 16
rgb color;


/*************************help funtion adapted from svgalib->vga.c*******************************/
//just a small delay
void __svgalib_delay(void)
{
    int i;
    for (i = 0; i < 10; i++);
}

//turn off the screen
void vga_screenoff() {
    /* turn screen back on */
    outb(0x01, SEQ_I);
	outb(inb(SEQ_D) | 0x20, SEQ_D);

    /* enable video output */
    inb(IS1_RC);
	__svgalib_delay();
	outb(0x00, ATT_IW);
}

//turn on the screen
void vga_screenon(void) {
    /* turn screen back on */
	outb(0x01, SEQ_I);
	outb(inb(SEQ_D) & 0xDF, SEQ_D);

    /* enable video output */
	inb(IS1_RC);
	__svgalib_delay();
	outb(0x20, ATT_IW);
}

//enable the color emulation
void setcoloremulation() {
	outb(inb(MIS_R) | 0x01, MIS_W);
}




/************************device operation function adapted from svgalib->vga.c*********************/
/*
 * Initial the vga device which is cirrus 5446 (QEMU) 
 */
// fill in the card spec info
void vga_init() {
    cirrus_init();
}


/*
 * set the mode in the vga device which is cirrus 5446 (QEMU) 
 * and we have only one mode so we don't need pass the argument in the function
 */
void vga_set_mode() {
    //disable the screen
    vga_screenoff();
    setcoloremulation();
    //fill in the info
    info = (info_t) {vga_x, vga_y, VGA_COLOR, vga_pix_byte*vga_x, vga_pix_byte};
    //set our display mode which is 1024*768 60hz
    cirrus_setmode(&info);
    //set the display on 0 in frame buffer
    cirrus_setdisplaystart(0);
    //set the scan length on bytes
    cirrus_setlogicalwidth(vga_line_byte);
    //set the linear
    cirrus_setlinear(0);
    //set the window page map into the frame buffer
    cirrus_setpage_2M(0);
    //clear the display
    vga_clearall();
    //enable the screen
    vga_screenon();
}

//this funtion clear all the page in the buffer memory
void vga_clearall() {
    int i;
    int pages = (vga_y * vga_line_byte + 65535) >>  16;
    vga_screenoff();
    for (i = 0; i < pages; i++) {
        cirrus_setpage_2M(i);
        memset(GM, 0, 65536);
    }
    vga_screenon();
}

int __svgalib_setregs(const unsigned char *regs)
{
    int i;

    /* update misc output register */
    __svgalib_outmisc(regs[MIS]);

    /* synchronous reset on */
    __svgalib_outseq(0x00,0x01);

    /* write sequencer registers */
    __svgalib_outseq(0x01,regs[SEQ + 1] | 0x20);
    outb(1, SEQ_I);
    outb(regs[SEQ + 1] | 0x20, SEQ_D);
    for (i = 2; i < SEQ_C; i++) {
       __svgalib_outseq(i,regs[SEQ + i]);
    }

    /* synchronous reset off */
    __svgalib_outseq(0x00,0x03);

	/* deprotect CRT registers 0-7 */
    __svgalib_outcrtc(0x11,__svgalib_incrtc(0x11)&0x7f);
    
    /* write CRT registers */
    for (i = 0; i < CRT_C; i++) {
        __svgalib_outcrtc(i,regs[CRT + i]);
    }

    /* write graphics controller registers */
    for (i = 0; i < GRA_C; i++) {
	outb(i, GRA_I);
	outb(regs[GRA + i], GRA_D);
    }

    /* write attribute controller registers */
    for (i = 0; i < ATT_C; i++) {
	inb(IS1_RC);		/* reset flip-flop */
	__svgalib_delay();
	outb(i, ATT_IW);
	__svgalib_delay();
	outb(regs[ATT + i], ATT_IW);
	__svgalib_delay();
    }

    return 0;
}

/*
 * void vga_setcolor(rgb c)
 * Inputs:  c -- the color to be set
 * Outputs: None
 * Side Effects:change the color set
 */
void vga_setcolor(rgb c) {
    color = c;
}


/*
 * void vga_drawpixel(int x, int y)
 * Inputs: x,y -- the coordinate of the pixel we need to write
 * Outputs: None
 * Side Effects: write one pixel one the screen 
 */
void vga_drawpixel(int x, int y) {
    unsigned long index = y * info.xbytes + info.bytesperpixel * x;
    cirrus_setpage_2M(index >> SHIFT_16);
    index  = index & B16MASK;
    gr_writew(color_convert(color), index);
}

