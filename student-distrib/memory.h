#ifndef _MEMORY_H
#define _MEMORY_H

#include "types.h"
#include "x86_desc.h"
#include "paging.h"
#include "lib.h"

#define MAX_ORDER 10                  // 10, max order of pages in the buddy system
#define MAX_NUM_PAGE (1 << MAX_ORDER) // 1024, max num of pages in the buddy system
#define SIZE_2KB 0x800
#define MAX_NUM_NODE ((MAX_NUM_PAGE << 1) - 1) // 2047, max num of nodes in the buddy system
#define BASE_ADDR_BD_SYS 0x40000000            // 1GB, base addr of buddy system
#define BASE_PD_INDEX 256                      // 1GB/4MB-1, pde index of base addr

typedef struct buddy_system
{
    int8_t max_order; // total number of page
    int8_t node_array[MAX_NUM_NODE];
} buddy_system_t;

buddy_system_t *init_buddy_sys(int32_t order);
void *bd_alloc(int32_t order);
int32_t bd_free(void *addr);
int32_t bd_display();
int32_t bd_get_size(void *addr);

void *high_level_alloc(int32_t size);

#endif /* _MEMORY_H */
