/*
 * vga.h
 *
 * Description:
 * head file of vga
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

#ifndef VGA_H
#define VGA_H

#include "../lib.h"
#include "cirrus.h"
#include "timing.h"
#include "vgaregs.h"

//def the type of the color 
typedef unsigned int rgb;
typedef unsigned short vga_color; 

//color converter!!!
//rgb 00000000rrrrrrrrggggggggbbbbbbbb
//vga_color 0rrrrrgggggbbbbb rrrrrggggggbbbbb
#define color_convert(rgb)  ((vga_color)(((rgb >> 3) & 0x1F) | ((rgb >> 6) & 0x3E0) | ((rgb >> 9) & 0x7C00)))

#define vga_x 1024
#define vga_y 768
#define vga_pix_byte 2
#define VGA_COLOR (1 << 15)
#define vga_line_byte (2*1024)

/* Mode for SetTransparency. */
#define DISABLE_TRANSPARENCY_COLOR	0
#define ENABLE_TRANSPARENCY_COLOR	1
#define DISABLE_BITMAP_TRANSPARENCY	2
#define ENABLE_BITMAP_TRANSPARENCY	3

/* Flags for SetMode (accelerator interface). */
#define BLITS_SYNC			0
#define BLITS_IN_BACKGROUND		0x1


/* --------------------- Macro definitions shared by library modules */

/* VGA index register ports */
#define CRT_IC  0x3D4		/* CRT Controller Index - color emulation */
#define CRT_IM  0x3B4		/* CRT Controller Index - mono emulation */
#define ATT_IW  0x3C0		/* Attribute Controller Index & Data Write Register */
#define GRA_I   0x3CE		/* Graphics Controller Index */
#define SEQ_I   0x3C4		/* Sequencer Index */
#define PEL_IW  0x3C8		/* PEL Write Index */
#define PEL_IR  0x3C7		/* PEL Read Index */

/* VGA data register ports */
#define CRT_DC  0x3D5		/* CRT Controller Data Register - color emulation */
#define CRT_DM  0x3B5		/* CRT Controller Data Register - mono emulation */
#define ATT_R   0x3C1		/* Attribute Controller Data Read Register */
#define GRA_D   0x3CF		/* Graphics Controller Data Register */
#define SEQ_D   0x3C5		/* Sequencer Data Register */
#define MIS_R   0x3CC		/* Misc Output Read Register */
#define MIS_W   0x3C2		/* Misc Output Write Register */
#define IS1_RC  0x3DA		/* Input Status Register 1 - color emulation */
#define IS1_RM  0x3BA		/* Input Status Register 1 - mono emulation */
#define PEL_D   0x3C9		/* PEL Data Register */
#define PEL_MSK 0x3C6		/* PEL mask register */

/* 8514/MACH regs we need outside of the mach32 driver.. */
#define PEL8514_D	0x2ED
#define PEL8514_IW	0x2EC
#define PEL8514_IR	0x2EB
#define PEL8514_MSK	0x2EA

/* EGA-specific registers */

#define GRA_E0	0x3CC		/* Graphics enable processor 0 */
#define GRA_E1	0x3CA		/* Graphics enable processor 1 */

/* standard VGA indexes max counts */
#define CRT_C   24		/* 24 CRT Controller Registers */
#define ATT_C   21		/* 21 Attribute Controller Registers */
#define GRA_C   9		/* 9  Graphics Controller Registers */
#define SEQ_C   5		/* 5  Sequencer Registers */
#define MIS_C   1		/* 1  Misc Output Register */

/* VGA registers saving indexes */
#define CRT     0		/* CRT Controller Registers start */
#define ATT     (CRT+CRT_C)	/* Attribute Controller Registers start */
#define GRA     (ATT+ATT_C)	/* Graphics Controller Registers start */
#define SEQ     (GRA+GRA_C)	/* Sequencer Registers */
#define MIS     (SEQ+SEQ_C)	/* General Registers */
#define EXT     (MIS+MIS_C)	/* SVGA Extended Registers */

#define GM     ((char*) VIDEO)
#define gr_readb(off)		(((volatile unsigned char *)GM)[(off)])
#define gr_readw(off)		(*(volatile unsigned short*)((GM)+(off)))
#define gr_readl(off)		(*(volatile unsigned long*)((GM)+(off)))
#define gr_writeb(v,off)	(GM[(off)] = (v))
#define gr_writew(v,off)	(*(unsigned short*)((GM)+(off)) = (v))
#define gr_writel(v,off)	(*(unsigned long*)((GM)+(off)) = (v))





#define min(x, y) ((x) < (y) ? (x) : (y))
#define LIMIT(var, lim) if (var > lim) var = lim;

info_t info;
CardSpecs cardspecs;

extern info_t info;
extern CardSpecs cardspecs;
//inital the vga device
void vga_init();
//vga set the mode
void vga_set_mode();
void vga_clearall();
//set the device register
int __svgalib_setregs(const unsigned char *regs);

//simple function for drawing picture

//set the current color -- original version is from svgalib/vgacol.c
void vga_setcolor(rgb c);
//draw one pix on the screen the -- original version is from svgalib/vgapix.c
void vga_drawpixel(int x, int y);


#endif

