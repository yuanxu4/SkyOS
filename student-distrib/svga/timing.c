/*
 * timing.c
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



/* 
 * Generic mode timing module.
 */

#include "vga.h"

/* Standard mode timings. */
//the timing mode for our device and mode
MonitorModeTiming TIMMING_OUR_MODE = {65000, 1024, 1032, 1176, 1344, 768, 771, 777, 806, NHSYNC | NVSYNC, NULL};


#define CLOCK_ALLOWANCE 10

#define PROGRAMMABLE_CLOCK_MAGIC_NUMBER 0x1234



int findclock(int clock, CardSpecs *cardspecs)
{
    int i;
    /* Find a clock that is close enough. */
    for (i = 0; i < cardspecs->nClocks; i++) {
	    int diff;
	    diff = cardspecs->clocks[i] - clock;
	    if (diff < 0)
	        diff = -diff;
	    if (diff * 1000 / clock < CLOCK_ALLOWANCE)
	        return i;
    }
    /* Try programmable clocks if available. */
    if (cardspecs->flags & CLOCK_PROGRAMMABLE) {
	    int diff;
	    diff = cardspecs->matchProgrammableClock(clock) - clock;
	    if (diff < 0)
	        diff = -diff;
	    if (diff * 1000 / clock < CLOCK_ALLOWANCE)
	        return PROGRAMMABLE_CLOCK_MAGIC_NUMBER;
    }
    /* No close enough clock found. */
    return -1;
}

//set the timing based on our display mode just copy from the svgalib
int __svgalib_getmodetiming(ModeTiming * modetiming, ModeInfo * modeinfo,
		  CardSpecs * card)
{
    int desiredclock;
    MonitorModeTiming *besttiming=NULL;

    /*
     * Check user defined timings first.
     * If there is no match within these, check the standard timings.
     */
    besttiming = &TIMMING_OUR_MODE;
    /*
     * Copy the selected timings into the result, which may
     * be adjusted for the chipset.
     */

    modetiming->flags = besttiming->flags;
    modetiming->pixelClock = besttiming->pixelClock;	/* Formal clock. */

    /*
     * We know a close enough clock is available; the following is the
     * exact clock that fits the mode. This is probably different
     * from the best matching clock that will be programmed.
     */
    desiredclock = besttiming->pixelClock;

    /* Fill in the best-matching clock that will be programmed. */
    modetiming->selectedClockNo = findclock(desiredclock, card);
   
	modetiming->programmedClock = card->clocks[modetiming->selectedClockNo];
    modetiming->HDisplay = besttiming->HDisplay;
    modetiming->HSyncStart = besttiming->HSyncStart;
    modetiming->HSyncEnd = besttiming->HSyncEnd;
    modetiming->HTotal = besttiming->HTotal;
    
	modetiming->CrtcHDisplay = besttiming->HDisplay;
	modetiming->CrtcHSyncStart = besttiming->HSyncStart;
	modetiming->CrtcHSyncEnd = besttiming->HSyncEnd;
	modetiming->CrtcHTotal = besttiming->HTotal;
    
    modetiming->VDisplay = besttiming->VDisplay;
    modetiming->VSyncStart = besttiming->VSyncStart;
    modetiming->VSyncEnd = besttiming->VSyncEnd;
    modetiming->VTotal = besttiming->VTotal;

    //todo: we can simplify this if
    if (modetiming->flags & DOUBLESCAN){
	modetiming->VDisplay <<= 1;
	modetiming->VSyncStart <<= 1;
	modetiming->VSyncEnd <<= 1;
	modetiming->VTotal <<= 1;
    }
    modetiming->CrtcVDisplay = modetiming->VDisplay;
    modetiming->CrtcVSyncStart = modetiming->VSyncStart;
    modetiming->CrtcVSyncEnd = modetiming->VSyncEnd;
    modetiming->CrtcVTotal = modetiming->VTotal;
    if (((modetiming->flags & INTERLACED)
	 && (card->flags & INTERLACE_DIVIDE_VERT))
	|| (modetiming->VTotal >= 1024
	    && (card->flags & GREATER_1024_DIVIDE_VERT))) {
	/*
	 * Card requires vertical CRTC timing to be halved for
	 * interlaced modes, or for all modes with vertical
	 * timing >= 1024.
	 */
	modetiming->CrtcVDisplay /= 2;
	modetiming->CrtcVSyncStart /= 2;
	modetiming->CrtcVSyncEnd /= 2;
	modetiming->CrtcVTotal /= 2;
	modetiming->flags |= VADJUSTED;
    }
    return 0;			/* Succesful. */
}


