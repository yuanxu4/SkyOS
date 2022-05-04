#ifndef _VIDMEM_H
#define _VIDMEM_H

#include "x86_desc.h"

#define VID_PAGE_INDEX 33
int32_t sys_vidmap(uint8_t **screen_start);

#endif //_PAGING_H
