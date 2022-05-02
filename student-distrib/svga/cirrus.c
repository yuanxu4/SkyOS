/*
 * cirrus.c
 *
 * Description:
 * the device driver for the cirrus 5446
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



#include "vga.h"


static int cirrus_memory;
static int cirrus_chiptype;
static int cirrus_chiprev;
static unsigned char actualMCLK, programmedMCLK;
static int DRAMbandwidth, DRAMbandwidthLimit;


#define NU_FIXED_CLOCKS 21
#define MMIOSETFOREGROUNDCOLOR16(c) \
  *(unsigned short *)(MMIO_POINTER + MMIOFOREGROUNDCOLOR) = c;

/* 12.588 clock (0x33, 0x3B) replaced with 12.599 (0x2C, 0x33). */

static int cirrus_fixed_clocks[NU_FIXED_CLOCKS] =
{
    12599, 18000, 19600,
    25227, 28325, 31500, 36025, 37747, 39992, 41164,
    45076, 49867, 64983, 72163, 75000, 80013, 85226, 89998,
    95019, 100226, 108035
};

static unsigned char fixed_clock_numerator[NU_FIXED_CLOCKS] =
{
    0x2C, 0x27, 0x28,
    0x4A, 0x5B, 0x42, 0x4E, 0x3A, 0x51, 0x45,
    0x55, 0x65, 0x76, 0x7E, 0x6E, 0x5F, 0x7D, 0x58,
    0x49, 0x46, 0x53
};

static unsigned char fixed_clock_denominator[NU_FIXED_CLOCKS] =
{
    0x33, 0x3e, 0x3a,
    0x2B, 0x2F, 0x1F, 0x3E, 0x17, 0x3A, 0x30,
    0x36, 0x3A, 0x34, 0x32, 0x2A, 0x22, 0x2A, 0x1C,
    0x16, 0x14, 0x16
};

/*
 * It looks like the 5434 palette clock doubling mode doensn't like
 * clocks like (0x7B, 0x20), whereas (0x53, 0x16) is OK.
 */

enum {
    CLGD5420 = 0, CLGD7548, CLGD5420B, CLGD5422, CLGD5422C, CLGD5424, CLGD5426,
    CLGD5428, CLGD5429, CLGD5430, CLGD5434, CLGD5436
};

enum {
    LINEAR_QUERY_BASE, LINEAR_QUERY_GRANULARITY, LINEAR_QUERY_RANGE,
    LINEAR_ENABLE, LINEAR_DISABLE
};
void cirrus_unlock();
/***************************** help function *********************************************/
/*
 * This is a temporary function that allocates and fills in a ModeInfo
 * structure based on a svgalib mode number.
 */
//do some adaption for that because we have only one mode
void __svgalib_createModeInfoStructureForSvgalibMode(ModeInfo *modeinfo, info_t *__svgalib_vga_info)
{
    /* Create the new ModeInfo structure. */
    modeinfo->width = __svgalib_vga_info->xdim;
    modeinfo->height = __svgalib_vga_info->ydim;
    modeinfo->bytesPerPixel = __svgalib_vga_info->bytesperpixel;
	modeinfo->colorBits = 15;
	modeinfo->blueOffset = 0;
	modeinfo->greenOffset = 5;
	modeinfo->redOffset = 10;
	modeinfo->blueWeight = 5;
	modeinfo->greenWeight = 5;
	modeinfo->redWeight = 5;
    modeinfo->bitsPerPixel = modeinfo->bytesPerPixel * 8;
    modeinfo->lineWidth = __svgalib_vga_info->xbytes;
    
}

static int cirrus_saveregs(unsigned char regs[])
{
    cirrus_unlock();		/* May be locked again by other programs (e.g. X) */
    /* Save extended CRTC registers. */
    regs[CIRRUSREG_CR(0x19)] = __svgalib_inCR(0x19);
    regs[CIRRUSREG_CR(0x1A)] = __svgalib_inCR(0x1A);
    regs[CIRRUSREG_CR(0x1B)] = __svgalib_inCR(0x1B);
	regs[CIRRUSREG_CR(0x1D)] = __svgalib_inCR(0x1D);
    /* Save extended graphics registers. */
    regs[CIRRUSREG_GR(0x09)] = __svgalib_inGR(0x09);
    regs[CIRRUSREG_GR(0x0A)] = __svgalib_inGR(0x0A);
    regs[CIRRUSREG_GR(0x0B)] = (0x0B);
    /* Save extended sequencer registers. */
    regs[CIRRUS_SR7] = __svgalib_inSR(0x07);
    regs[CIRRUS_VCLK3NUMERATOR] = __svgalib_inSR(0x0E);
    regs[CIRRUS_DRAMCONTROL] = __svgalib_inSR(0x0F);
	regs[CIRRUS_PERFTUNING] = __svgalib_inSR(0x16);
	regs[CIRRUS_SR17] = __svgalib_inSR(0x17);
    regs[CIRRUS_VCLK3DENOMINATOR] = __svgalib_inSR(0x1E);
	regs[CIRRUS_MCLKREGISTER] = __svgalib_inSR(0x1F);
    /* Save Hicolor DAC register. */
	outb(0, 0x3c6);
	outb(0xff, 0x3c6);
	inb(0x3c6);
	inb(0x3c6);
	inb(0x3c6);
	inb(0x3c6);
	regs[CIRRUSREG_DAC] = inb(0x3c6);

    return CIRRUS_TOTAL_REGS - VGA_TOTAL_REGS;
}

void writehicolordac(unsigned char c)
{
    outb(0, 0x3c6);
    outb(0xff, 0x3c6);
    inb(0x3c6);
    inb(0x3c6);
    inb(0x3c6);
    inb(0x3c6);
    outb(c, 0x3c6);
    inb(0x3c8);
}

/* Unlock chipset-specific registers */
void cirrus_unlock()
{
    int vgaIOBase, temp;
    /* Are we Mono or Color? */
    vgaIOBase = (inb(0x3CC) & 0x01) ? 0x3D0 : 0x3B0;
    outb(0x06, SEQ_I);
    outb(0x12, SEQ_D);		/* unlock cirrus special */

    /* Put the Vert. Retrace End Reg in temp */
    outb(0x11, vgaIOBase + 4);
    temp = inb(vgaIOBase + 5);

    /* Put it back with PR bit set to 0 */
    /* This unprotects the 0-7 CRTC regs so */
    /* they can be modified, i.e. we can set */
    /* the timing. */

    outb(temp & 0x7F, vgaIOBase + 5);
}

/* Set chipset-specific registers */

void cirrus_setregs(const unsigned char regs[])
{
/*      #ifdef DEBUG
   printf("Setting Cirrus extended registers.\n");
   #endif
 */
    cirrus_unlock();		/* May be locked again by other programs (eg. X) */

    /* Write extended CRTC registers. */
    __svgalib_outCR(0x19, regs[CIRRUSREG_CR(0x19)]);
    __svgalib_outCR(0x1A, regs[CIRRUSREG_CR(0x1A)]);
    __svgalib_outCR(0x1B, regs[CIRRUSREG_CR(0x1B)]);
	__svgalib_outCR(0x1D, regs[CIRRUSREG_CR(0x1D)]);
    /* Write extended graphics registers. */
    __svgalib_outGR(0x09, regs[CIRRUSREG_GR(0x09)]);
    __svgalib_outGR(0x0A, regs[CIRRUSREG_GR(0x0A)]);
    __svgalib_outGR(0x0B, regs[CIRRUSREG_GR(0x0B)]);
    /* Write Truecolor DAC register. */
	writehicolordac(regs[CIRRUS_HIDDENDAC]);
    /* Write extended sequencer registers. */
    /* Be careful to put the sequencer clocking mode in a safe state. */
    __svgalib_outSR(0x07, (regs[CIRRUS_SR7] & ~0x0F) | 0x01);
    __svgalib_outSR(0x0E, regs[CIRRUS_VCLK3NUMERATOR]);
    __svgalib_outSR(0x1E, regs[CIRRUS_VCLK3DENOMINATOR]);
    __svgalib_outSR(0x07, regs[CIRRUS_SR7]);
    __svgalib_outSR(0x0F, regs[CIRRUS_DRAMCONTROL]);
	__svgalib_outSR(0x16, regs[CIRRUS_PERFTUNING]);
	__svgalib_outSR(0x17, regs[CIRRUS_SR17]);
	__svgalib_outSR(0x1F, regs[CIRRUS_MCLKREGISTER]);
}

/* Set a mode */

/* Local, called by cirrus_setmode(). */

static void cirrus_initializemode(unsigned char *moderegs, ModeTiming * modetiming, ModeInfo * modeinfo)
{

    /* Get current values. */
    cirrus_saveregs(moderegs);

    /* Set up the standard VGA registers for a generic SVGA. */
    __svgalib_setup_VGA_registers(moderegs, modetiming, modeinfo);

    /* Set up the extended register values, including modifications */
    /* of standard VGA registers. */

/* Graphics */
    moderegs[CIRRUS_GRAPHICSOFFSET1] = 0;	/* Reset banks. */
    moderegs[CIRRUS_GRAPHICSOFFSET2] = 0;
    moderegs[CIRRUS_GRB] = 0;	/* 0x01 enables dual banking. */
	/* Enable 16K granularity. */
	moderegs[CIRRUS_GRB] |= 0x20;
    moderegs[VGA_SR2] = 0xFF;	/* Plane mask */

/* CRTC */
    if (modetiming->VTotal >= 1024 && !(modetiming->flags & INTERLACED))
	/*
	 * Double the vertical timing. Used for 1280x1024 NI.
	 * The CrtcVTimings have already been adjusted
	 * by __svgalib_getmodetiming() because of the GREATER_1024_DIVIDE_VERT
	 * flag.
	 */
	moderegs[VGA_CR17] |= 0x04;
    moderegs[CIRRUS_CR1B] = 0x22;
    if (cirrus_chiptype >= CLGD5434)
	/* Clear display start bit 19. */
	SETBITS(moderegs[CIRRUS_CR1D], 0x80, 0);
    /* CRTC timing overflows. */
    moderegs[CIRRUS_CR1A] = 0;
    SETBITSFROMVALUE(moderegs[CIRRUS_CR1A], 0xC0,
		     modetiming->CrtcVSyncStart + 1, 0x300);
    SETBITSFROMVALUE(moderegs[CIRRUS_CR1A], 0x30,
		     modetiming->CrtcHSyncEnd, (0xC0 << 3));
    moderegs[CIRRUS_CR19] = 0;	/* Interlaced end. */
    if (modetiming->flags & INTERLACED) {
	moderegs[CIRRUS_CR19] =
	    ((modetiming->CrtcHTotal / 8) - 5) / 2;
	moderegs[CIRRUS_CR1A] |= 0x01;
    }
/* Scanline offset */
    if (modeinfo->bytesPerPixel == 4) {
	/* At 32bpp the chip does an extra multiplication by two. */
	if (cirrus_chiptype >= CLGD5436) {
		/* Do these chipsets multiply by 4? */
		moderegs[VGA_SCANLINEOFFSET] = modeinfo->lineWidth >> 5;
		SETBITSFROMVALUE(moderegs[CIRRUS_CR1B], 0x10,
				 modeinfo->lineWidth, 0x2000);
	}
	else {
		moderegs[VGA_SCANLINEOFFSET] = modeinfo->lineWidth >> 4;
		SETBITSFROMVALUE(moderegs[CIRRUS_CR1B], 0x10,
				 modeinfo->lineWidth, 0x1000);
	}
    } else if (modeinfo->bitsPerPixel == 4)
	/* 16 color mode (planar). */
	moderegs[VGA_SCANLINEOFFSET] = modeinfo->lineWidth >> 1;
    else {
	moderegs[VGA_SCANLINEOFFSET] = modeinfo->lineWidth >> 3;
	SETBITSFROMVALUE(moderegs[CIRRUS_CR1B], 0x10,
			 modeinfo->lineWidth, 0x800);
    }

/* Clocking */
    moderegs[VGA_MISCOUTPUT] |= 0x0C;	/* Use VCLK3. */
    moderegs[CIRRUS_VCLK3NUMERATOR] =
	fixed_clock_numerator[modetiming->selectedClockNo];
    moderegs[CIRRUS_VCLK3DENOMINATOR] =
	fixed_clock_denominator[modetiming->selectedClockNo];

/* DAC register and Sequencer Mode */
    {
	unsigned char DAC, SR7;
	DAC = 0x00;
	SR7 = 0x00;
	if (modeinfo->bytesPerPixel > 0)
	    SR7 = 0x01;		/* Packed-pixel mode. */
	if (modeinfo->bytesPerPixel == 2) {
	    int rgbmode;
	    rgbmode = 0;	/* 5-5-5 RGB. */
	    if (modeinfo->colorBits == 16)
		rgbmode = 1;	/* Add one for 5-6-5 RGB. */
	    if (cirrus_chiptype >= CLGD5426) {
		/* Pixel clock (double edge) mode. */
		DAC = 0xD0 + rgbmode;
		SR7 = 0x07;
	    } else {
		/* Single-edge (double VCLK). */
		DAC = 0xF0 + rgbmode;
		SR7 = 0x03;
	    }
	}
	if (modeinfo->bytesPerPixel >= 3) {
	    /* Set 8-8-8 RGB mode. */
	    DAC = 0xE5;
	    SR7 = 0x05;
	    if (modeinfo->bytesPerPixel == 4)
		SR7 = 0x09;
	}
	if (modeinfo->bytesPerPixel == 1 && (modetiming->flags & HADJUSTED)) {
	    /* Palette clock doubling mode on 5434 8bpp. */
	    DAC = 0x4A;
	    SR7 = 0x07;
	}

	moderegs[CIRRUS_HIDDENDAC] = DAC;
	moderegs[CIRRUS_SR7] = SR7;
    }

/* DRAM control and CRT FIFO */
    if (cirrus_chiptype >= CLGD5422)
	/* Enable large CRT FIFO. */
	moderegs[CIRRUS_DRAMCONTROL] |= 0x20;
    if (cirrus_memory == 2048 && cirrus_chiptype <= CLGD5429)
	/* Enable DRAM Bank Select. */
	moderegs[CIRRUS_DRAMCONTROL] |= 0x80;
    if (cirrus_chiptype >= CLGD5424) {
	/* CRT FIFO threshold setting. */
	unsigned char threshold;
	threshold = 8;
	/* XXX Needs more elaborate setting. */
	SETBITS(moderegs[CIRRUS_PERFTUNING], 0x0F, threshold);
    }

	if (programmedMCLK != actualMCLK
	    && modeinfo->bytesPerPixel > 0)
	    /* Program higher MCLK for packed-pixel modes. */
	    moderegs[CIRRUS_MCLKREGISTER] = programmedMCLK;
}


int cirrus_pattern_address;	/* Pattern with 1's (8 bytes) */
int cirrus_bitblt_pixelwidth;
void __svgalib_cirrusaccel_init(int bpp, int width_in_pixels) {

    
    /* [Setup accelerator screen pitch] */
    /* [Prepare any required off-screen space] */
    cirrus_bitblt_pixelwidth = PIXELWIDTH16;

    SETSRCPITCH(__svgalib_accel_screenpitchinbytes);
    SETDESTPITCH(__svgalib_accel_screenpitchinbytes);
    SETROP(0x0D);

    cirrus_pattern_address = cirrus_memory * 1024 - 8;

    cirrus_setpage_2M(cirrus_pattern_address / 65536);
    gr_writel(0xffffffff, cirrus_pattern_address & 0xffff);
    gr_writel(0xffffffff, (cirrus_pattern_address & 0xffff) + 4);
    cirrus_setpage_2M(0);

    /* Enable memory-mapped I/O. */
    __svgalib_outSR(0x17, __svgalib_inSR(0x17) | 0x04);
}

/***************************** operation funtion ***************************************************/

/* Bank switching function -- set 64K page number */
void cirrus_setpage_2M(int page)
{
    /* Cirrus banking register has been set to 16K granularity */
    outw((page << 10) + 0x09, GRA_I);
}

/* Set display start address (not for 16 color modes) */
/* Cirrus supports any address in video memory (up to 2Mb) */
void cirrus_setdisplaystart(int address) {
    outw(0x0d + ((address >> 2) & 0x00ff) * 256, CRT_IC);    /* sa2-sa9 */
    outw(0x0c + ((address >> 2) & 0xff00), CRT_IC);    /* sa10-sa17 */
    inb(0x3da);            /* set ATC to addressing mode */
    outb(0x13 + 0x20, ATT_IW);    /* select ATC reg 0x13 */
    /* Cirrus specific bits 0,1 and 18,19,20: */
    outb((inb(ATT_R) & 0xf0) | (address & 3), ATT_IW);
    /* write sa0-1 to bits 0-1; other cards use bits 1-2 */
    outb(0x1b, CRT_IC);
    outb((inb(CRT_DC) & 0xf2)
         | ((address & 0x40000) >> 18)    /* sa18: write to bit 0 */
         | ((address & 0x80000) >> 17)    /* sa19: write to bit 2 */
         | ((address & 0x100000) >> 17), CRT_DC);    /* sa20: write to bit 3 */
    outb(0x1d, CRT_IC);

    outb((inb(CRT_DC) & 0x7f) | ((address & 0x200000) >> 14), CRT_DC);    /* sa21: write to bit 7 */
    
}

/* Set logical scanline length (usually multiple of 8) */
/* Cirrus supports multiples of 8, up to 4088 */
void cirrus_setlogicalwidth(int width) {
    outw(0x13 + (width >> 3) * 256, CRT_IC);    /* lw3-lw11 */
    outb(0x1b, CRT_IC);
    outb((inb(CRT_DC) & 0xef) | ((width & 0x800) >> 7), CRT_DC);
    /* write lw12 to bit 4 of Sequencer reg. 0x1b */
}

void cirrus_setlinear(int addr)
{
    int val;
    outb(0x07, SEQ_I);
    val = inb(SEQ_D);
    outb((val & 0x0f) | (addr << 4), SEQ_D);
}


int cirrus_init()
{
    //test the device
    int oldlockreg;
    int lockreg;

    outb(0x06, SEQ_I);
    oldlockreg = inb(SEQ_D);

    cirrus_unlock();

    /* If it's a Cirrus at all, we should be */
    /* able to read back the lock register */

    outb(0x06, SEQ_I);
    lockreg = inb(SEQ_D);

    /* Ok, if it's not 0x12, we're not a Cirrus542X. */
    if (lockreg != 0x12) {
	outb(0x06, SEQ_I);
	outb(oldlockreg, SEQ_D);
	return 0;
    }
    /* The above check seems to be weak, so we also check the chip ID. */

    outb(0x27, CRT_IC);
    if(0x2E != inb(CRT_DC) >> 2) {
	    outb(0x06, SEQ_I);
	    outb(oldlockreg, SEQ_D);
	    return 0;
    }
    //pass the check
    printf("pass the chip check start initial!");
    cirrus_unlock();
	cirrus_chiprev = 0;
    //the type only have CLGD5436 as CLGD5446
	cirrus_chiptype = CLGD5436;
    cirrus_memory = 4096;
	
	actualMCLK = __svgalib_inSR(0x1F) & 0x3F;
    programmedMCLK = actualMCLK;
	
    DRAMbandwidth = 4 * (14318 * (int) programmedMCLK / 16);

    /*
     * Calculate highest acceptable DRAM bandwidth to be taken up
     * by screen refresh. Satisfies
     *     total bandwidth >= refresh bandwidth * 1.1
     */
    DRAMbandwidthLimit = (DRAMbandwidth * 10) / 11;

    
/* begin: Initialize card specs. */
    cardspecs.videoMemory = cirrus_memory;
	/* 5429, 5430, 5434 have VCLK spec of 86 MHz. */
	cardspecs.maxPixelClock4bpp = 86000;
	cardspecs.maxPixelClock16bpp = 86000;	
	cardspecs.maxPixelClock24bpp = 86000 / 3;
	cardspecs.maxPixelClock8bpp = 135300;
	cardspecs.maxPixelClock32bpp = 86000;
        
    cardspecs.maxPixelClock8bpp = min(cardspecs.maxPixelClock8bpp, DRAMbandwidthLimit);
    cardspecs.maxPixelClock16bpp = min(cardspecs.maxPixelClock16bpp, DRAMbandwidthLimit / 2);
    cardspecs.maxPixelClock24bpp = min(cardspecs.maxPixelClock24bpp, DRAMbandwidthLimit / 3);
    cardspecs.maxPixelClock32bpp = min(cardspecs.maxPixelClock32bpp, DRAMbandwidthLimit / 4);
    cardspecs.flags = INTERLACE_DIVIDE_VERT | GREATER_1024_DIVIDE_VERT;
    /* Initialize clocks (only fixed set for now). */
    cardspecs.nClocks = NU_FIXED_CLOCKS;
    cardspecs.clocks = cirrus_fixed_clocks;
    cardspecs.maxHorizontalCrtc = 2040;
    /* Disable 16-color SVGA modes (don't work correctly). */
    cardspecs.maxPixelClock4bpp = 0;
/* end: Initialize card specs. */
    MMIO_POINTER = (unsigned char *) 0xb8000;

return 0;
}


//this funtion set all the register in the device
int cirrus_setmode(info_t* vga_info) {
    unsigned char moderegs[CIRRUS_TOTAL_REGS];
    ModeTiming modetiming;
    ModeInfo modeinfo;

    //fill in the modeinfo by the vga_info
    __svgalib_createModeInfoStructureForSvgalibMode(&modeinfo, vga_info);

    //fill in the modetiming by mode info and cardspecs
    __svgalib_getmodetiming(&modetiming, &modeinfo, &cardspecs);
    
    //fill in the moderegs by modetiming and modeinfo
    cirrus_initializemode(moderegs, &modetiming, &modeinfo);

    //write the moderegs into HW regs
    __svgalib_setregs(moderegs);       /* Set standard regs. */
    cirrus_setregs(moderegs);    /* Set extended regs. */

    //set the basic info for accel
    __svgalib_accel_screenpitch = modeinfo.lineWidth / modeinfo.bytesPerPixel;
    __svgalib_accel_bytesperpixel = modeinfo.bytesPerPixel;
    __svgalib_accel_screenpitchinbytes = modeinfo.lineWidth;
    __svgalib_accel_mode = 1;
    //initial the accel mode
    __svgalib_cirrusaccel_init(modeinfo.bitsPerPixel,
                               modeinfo.lineWidth / modeinfo.bytesPerPixel);

    return 0;
}

//this is the structure for raster operation map
static unsigned char cirrus_rop_map[] = {0x0D, 0x6D, 0x05,0x59,0x0B};

int __svgalib_accel_screenpitch;
int __svgalib_accel_bytesperpixel;
int __svgalib_accel_screenpitchinbytes;
int __svgalib_accel_mode;
//this is the base address for the MMIO for BitBLT
unsigned char *MMIO_POINTER;


//this funtion using the BitBLT engine to achieve the memory copy in the frame buffer
void __svgalib_cirrusaccel_mmio_ScreenCopy(int x1, int y1, int x2, int y2, int width, int height){
    //set RASTER OP
    MMIOFINISHBACKGROUNDBLITS();
    MMIOSETROP(cirrus_rop_map[ROP_COPY]);

    int srcaddr, destaddr, dir;
    width *= __svgalib_accel_bytesperpixel;
    srcaddr = BLTBYTEADDRESS(x1, y1);
    destaddr = BLTBYTEADDRESS(x2, y2);
    dir = FORWARDS;
    if ((y1 < y2 || (y1 == y2 && x1 < x2))
        && y1 + height > y2) {
        srcaddr += (height - 1) * __svgalib_accel_screenpitchinbytes + width - 1;
        destaddr += (height - 1) * __svgalib_accel_screenpitchinbytes + width - 1;
        dir = BACKWARDS;
    }
    MMIOFINISHBACKGROUNDBLITS();
    MMIOSETSRCADDR(srcaddr);
    MMIOSETDESTADDR(destaddr);
    MMIOSETWIDTH(width);
    MMIOSETHEIGHT(height);
    MMIOSETBLTMODE(dir);
    MMIOSTARTBLT();
    MMIOWAITUNTILFINISHED();
}

//this function achieve the fill in one color using the BitBLT buffer
void __svgalib_cirrusaccel_mmio_FillBox(int x, int y, int width, int height, int color)
{
    MMIOFINISHBACKGROUNDBLITS();
    MMIOSETFOREGROUNDCOLOR16(color);
    MMIOSETROP(cirrus_rop_map[ROP_COPY]);

    int destaddr;
    destaddr = BLTBYTEADDRESS(x, y);
    width *= __svgalib_accel_bytesperpixel;
    MMIOFINISHBACKGROUNDBLITS();
    MMIOSETSRCADDR(cirrus_pattern_address);
    MMIOSETDESTADDR(destaddr);
    MMIOSETWIDTH(width);
    MMIOSETHEIGHT(height);
    MMIOSETBLTMODE(0x80 | PATTERNCOPY | cirrus_bitblt_pixelwidth);
    MMIOSTARTBLT();
}

//this function achieve the fill in one color using the BitBLT buffer
void __svgalib_cirrusaccel_mmio_Reverse(int x, int y, int width, int height)
{
    MMIOFINISHBACKGROUNDBLITS();
    MMIOSETROP(cirrus_rop_map[ROP_INVERT]);

    int destaddr;
    destaddr = BLTBYTEADDRESS(x, y);
    width *= __svgalib_accel_bytesperpixel;
    MMIOFINISHBACKGROUNDBLITS();
    MMIOSETSRCADDR(cirrus_pattern_address);
    MMIOSETDESTADDR(destaddr);
    MMIOSETWIDTH(width);
    MMIOSETHEIGHT(height);
    MMIOSETBLTMODE(cirrus_bitblt_pixelwidth);
    MMIOSTARTBLT();
}

//this funtion wait the BitBLT engine to finish
void __svgalib_cirrusaccel_mmio_Sync() {
    MMIOWAITUNTILFINISHED();
}

