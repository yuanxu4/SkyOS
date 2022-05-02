/*
 * cirrus.h
 *
 *  this file is a adapted version from the svgalib
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

#ifndef CIRRUS_H
#define CIRRUS_H

#include "vga.h"


/* Flags: */
/* Every programmable scanline width is supported by the accelerator. */
#define ACCELERATE_ANY_LINEWIDTH	0x1
/* Bitmap (1-bit-per-pixel) operations support transparency (bit = 0). */
#define BITMAP_TRANSPARENCY		0x2
/* For bitmaps (1 bpp) stored in video memory, the most-significant bit */
/* within a byte is the leftmost pixel. */
#define BITMAP_ORDER_MSB_FIRST		0x4

/* Operation flags: see vga.h. */



/* Macros. */

#define BLTBYTEADDRESS(x, y) \
	(y * __svgalib_accel_screenpitchinbytes + x * __svgalib_accel_bytesperpixel)

#define BLTPIXADDRESS(x, y) \
	(y * __svgalib_accel_screenpitch + x)

/* Variables defined in accel.c */
extern int __svgalib_accel_screenpitch;
extern int __svgalib_accel_bytesperpixel;
extern int __svgalib_accel_screenpitchinbytes;
extern int __svgalib_accel_mode;



/*
 * The following driver function fills in available accelerator
 * primitives for a graphics mode (operations etc.). It could be part
 * of the setmode driver function.
 *
 * void initOperations( AccelSpecs *accelspecs, int bpp, int width_in_pixels );
 */

//the start point for BitBLT register MMIO
extern unsigned char *MMIO_POINTER;

/* Raster ops. */
#define ROP_COPY			0	/* Straight copy. */
#define ROP_INVERT		4 

// initial the accel mode set the last memory into 0xffffffff
void __svgalib_cirrusaccel_init(int bpp, int width_in_pixels);
//wait until the BitBLT engine finished
void __svgalib_cirrusaccel_mmio_Sync();
//set the accel mode


#define MMIOBACKGROUNDCOLOR    0x00
#define MMIOFOREGROUNDCOLOR    0x04
#define MMIOWIDTH        0x08
#define MMIOHEIGHT        0x0A
#define MMIODESTPITCH        0x0C
#define MMIOSRCPITCH        0x0E
#define MMIODESTADDR        0x10
#define MMIOSRCADDR        0x14
#define MMIOBLTWRITEMASK    0x17
#define MMIOBLTMODE        0x18
#define MMIOROP            0x1A
#define MMIOTRANSPARENTCOLOR  0x1C
#define MMIOBLTSTATUS        0x40
/* Enable support for > 85 MHz dot clocks on the 5434. */
#define SUPPORT_5434_PALETTE_CLOCK_DOUBLING
/* Use the special clocking mode for all dot clocks at 256 colors, not */
/* just those > 85 MHz, for debugging. */
/* #define ALWAYS_USE_5434_PALETTE_CLOCK_DOUBLING */

#define CIRRUSREG_GR(i) (VGA_TOTAL_REGS + i - VGA_GRAPHICS_COUNT)
#define CIRRUSREG_SR(i) (VGA_TOTAL_REGS + 5 + i - VGA_SEQUENCER_COUNT)
#define CIRRUSREG_CR(i) (VGA_TOTAL_REGS + 5 + 27 + i - VGA_CRTC_COUNT)
#define CIRRUSREG_DAC (VGA_TOTAL_REGS + 5 + 27 + 15)
#define CIRRUS_TOTAL_REGS (VGA_TOTAL_REGS + 5 + 27 + 15 + 1)

/* Indices into mode register array. */
#define CIRRUS_GRAPHICSOFFSET1	CIRRUSREG_GR(0x09)
#define CIRRUS_GRAPHICSOFFSET2	CIRRUSREG_GR(0x0A)
#define CIRRUS_GRB		CIRRUSREG_GR(0x0B)
#define CIRRUS_SR7		CIRRUSREG_SR(0x07)
#define CIRRUS_VCLK3NUMERATOR	CIRRUSREG_SR(0x0E)
#define CIRRUS_DRAMCONTROL	CIRRUSREG_SR(0x0F)
#define CIRRUS_PERFTUNING	CIRRUSREG_SR(0x16)
#define CIRRUS_SR17		CIRRUSREG_SR(0x17)
#define CIRRUS_VCLK3DENOMINATOR CIRRUSREG_SR(0x1E)
#define CIRRUS_MCLKREGISTER	CIRRUSREG_SR(0x1F)
#define CIRRUS_CR19		CIRRUSREG_CR(0x19)
#define CIRRUS_CR1A		CIRRUSREG_CR(0x1A)
#define CIRRUS_CR1B		CIRRUSREG_CR(0x1B)
#define CIRRUS_CR1D 		CIRRUSREG_CR(0x1D)
#define CIRRUS_HIDDENDAC	CIRRUSREG_DAC

#define port_outb(port, value) outb((value), (port))
#define port_outw(port, value) outw((value), (port))
#define port_outl(port, value) outl((value), (port))

/* BitBLT modes. */

#define FORWARDS        0x00
#define BACKWARDS        0x01
#define SYSTEMDEST        0x02
#define SYSTEMSRC        0x04
#define TRANSPARENCYCOMPARE    0x08
#define PIXELWIDTH16        0x10
#define PIXELWIDTH32        0x30    /* 543x only. */
#define PATTERNCOPY        0x40
#define COLOREXPAND        0x80

/* Macros for normal I/O BitBLT register access. */


/* Pitch: the 5426 goes up to 4095, the 5434 can do 8191. */

#define SETDESTPITCH(pitch) \
    port_outw(GRA_I, (((pitch) & 0x000000FF) << 8) | 0x24); \
    port_outw(GRA_I, (((pitch) & 0x00001F00)) | 0x25);

#define SETSRCPITCH(pitch) \
    port_outw(GRA_I, (((pitch) & 0x000000FF) << 8) | 0x26); \
    port_outw(GRA_I, (((pitch) & 0x00001F00)) | 0x27);

/* Width: the 5426 goes up to 2048, the 5434 can do 8192. */

/* Height: the 5426 goes up to 1024, the 5434 can do 2048. */
/* It appears many 5434's only go up to 1024. */

#define SETTRANSPARENCYCOLOR(c) \
    port_outw(GRA_I, ((c) << 8) | 0x34);

#define SETTRANSPARENCYCOLOR16(c) \
    port_outw(GRA_I, ((c & 0xFF) << 8) | 0x34); \
    port_outw(GRA_I, (c & 0xFF00) | 0x35);

#define SETTRANSPARENCYCOLORMASK16(m) \
    port_outw(GRA_I, ((m) << 8) | 0x38); \
    port_outw(GRA_I, ((m) & 0xFF00) | 0x39);

#define SETROP(rop) \
    port_outw(GRA_I, ((rop) << 8) | 0x32);

#define BLTBUSY(s) { \
    port_outb(GRA_I, 0x31); \
    s = inb(GRA_D) & 1; \
    }

#define WAITUNTILFINISHED() \
    for (;;) { \
        int busy; \
        BLTBUSY(busy); \
        if (!busy) \
            break; \
    }

#define MMIOSETDESTADDR(addr) \
  *(unsigned int *)(MMIO_POINTER + MMIODESTADDR) = addr;

#define MMIOSETSRCADDR(addr) \
  *(unsigned int *)(MMIO_POINTER + MMIOSRCADDR) = addr;


/* Width: the 5426 goes up to 2048, the 5434 can do 8192. */

#define MMIOSETWIDTH(width) \
  *(unsigned short *)(MMIO_POINTER + MMIOWIDTH) = (width) - 1;

/* Height: the 5426 goes up to 1024, the 5434 can do 2048. */

#define MMIOSETHEIGHT(height) \
  *(unsigned short *)(MMIO_POINTER + MMIOHEIGHT) = (height) - 1;

#define MMIOSETBLTMODE(m) \
  *(unsigned char *)(MMIO_POINTER + MMIOBLTMODE) = m;


#define MMIOSETROP(rop) \
  *(unsigned char *)(MMIO_POINTER + MMIOROP) = rop;

#define MMIOSTARTBLT() \
  *(unsigned char *)(MMIO_POINTER + MMIOBLTSTATUS) |= 0x02;

#define MMIOBLTBUSY(s) \
  s = *(volatile unsigned char *)(MMIO_POINTER + MMIOBLTSTATUS) & 1;

#define MMIOWAITUNTILFINISHED() \
    for (;;) { \
        int busy; \
        MMIOBLTBUSY(busy); \
        if (!busy) \
            break; \
    }

#define FINISHBACKGROUNDBLITS() \
    if (__svgalib_accel_mode & BLITS_IN_BACKGROUND) \
        WAITUNTILFINISHED();

#define MMIOFINISHBACKGROUNDBLITS() \
    if (__svgalib_accel_mode & BLITS_IN_BACKGROUND) \
        MMIOWAITUNTILFINISHED();

/* graphics mode information */
typedef struct {
    int xdim;
    int ydim;
    int colors;
    int xbytes;
    int bytesperpixel;
}info_t;

extern int cirrus_pattern_address;	/* Pattern with 1's (8 bytes) */
extern int cirrus_bitblt_pixelwidth;

//initail the device fill in the card spec
int cirrus_init();
//set the mode with the info
int cirrus_setmode(info_t *vga_info);
//set the display address in the memory in device
void cirrus_setdisplaystart(int address);
//set the scanline width
void cirrus_setlogicalwidth(int width);
void cirrus_setlinear(int addr);
//set the page of the window into the frame buffer
void cirrus_setpage_2M(int page);
// video mem copy accel
void __svgalib_cirrusaccel_mmio_ScreenCopy(int x1, int y1, int x2, int y2, int width, int height);

void __svgalib_cirrusaccel_mmio_FillBox(int x, int y, int width, int height, int color);
void __svgalib_cirrusaccel_mmio_Reverse(int x, int y, int width, int height);



#endif
