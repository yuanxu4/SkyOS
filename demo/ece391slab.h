#if !defined(ECE391SLAB_H)
#define ECE391SLAB_H

#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define MAX_ORDER_PAGE 5                   // max order of pages in the buddy system
#define MAX_NUM_PAGE (1 << MAX_ORDER_PAGE) // max num of pages in the buddy system
#define SIZE_2KB 0x800
#define MAX_NUM_NODE ((MAX_NUM_PAGE << 1) - 1) // 2047, max num of nodes in the buddy system
#define BASE_ADDR_BD_SYS 0x40000000            // 1GB, base addr of buddy system
#define BASE_PD_INDEX 256                      // 1GB/4MB-1, pde index of base addr
#define MAX_NUM_OBJ 255                        // 4K/16, max num of obj in one slab
#define MIN_ORDER_OBJ 4                        // <=> 16 bytes, min size of obj
#define MAX_ORDER_OBJ 11                       // <=> 2K bytes, max size of obj
#define MAX_ORDER_SLAB 2                       // <=> 4 pages in one slab max
#define MAX_NUM_CACHE 16                       // max num of cache
#define PAGE_NUM_FOR_OFF_SLAB 2                // num of pages to store off slabs
#define MAX_NUM_OFF_SLAB 25                    // max num of off slab
#define BUFSIZE 1024
#define SIZE_4KB 4096
#define SIZE_4MB 0x400000

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define is_power_two(num) (!((num) & (num - 1)))
#define left_child(index) ((index << 1) + 1)
#define right_child(index) ((index << 1) + 2)
#define parent(index) (((index + 1) >> 1) - 1)
#define get_size(order) (1 << (order))

typedef struct buddy_system
{
    int8_t max_order; // total number of page
    int8_t node_array[MAX_NUM_NODE];
    uint32_t base_addr;
} buddy_system_t;

buddy_system_t *init_buddy_sys(buddy_system_t *buddy_sys, int32_t order);
void *bd_alloc(buddy_system_t *buddy_sys, int32_t order);
int32_t bd_free(buddy_system_t *buddy_sys, void *addr);
int32_t bd_display(buddy_system_t *buddy_sys);
int32_t bd_get_size(buddy_system_t *buddy_sys, void *addr);
void *bd_get_start(buddy_system_t *buddy_sys, void *addr);

typedef struct slab slab_t;
typedef struct mem_cache mem_cache_t;
typedef struct memory_system memory_system_t;

void *k_alloc(memory_system_t *mem_sys, int32_t size);
int32_t k_free(memory_system_t *mem_sys, void *addr);
int32_t memory_shrink(memory_system_t *mem_sys);

struct slab
{                       // size is 288B
    slab_t *next;       // next slab
    mem_cache_t *cache; // the cache manages this slab
    // slab_t *head; // the first slab
    int32_t obj_size;                  // size of obj
    int32_t off_slab_index;            // used if it is a off slab, -1 of on slab
    int32_t total_num;                 // num of total objs in this slab
    int32_t free_num;                  // num of free objs in this slab
    int32_t next_free_index;           // index of next free obj
    void *obj_start;                   // start addr of objs
    uint8_t free_id_list[MAX_NUM_OBJ]; // store the id of next free obj, -1 for using obj
};

int32_t slab_init(slab_t *new_slab, slab_t *next, mem_cache_t *cache, int32_t num_obj, int32_t obj_size, void *obj_start, int32_t off_slab_index);
void *slab_get_obj(slab_t *slab);
int32_t slab_put_obj(slab_t *slab, void *obj);
int32_t slab_display(slab_t *slab, int32_t index);
struct mem_cache
{                       // size is 12B
    slab_t *slabs_head; // point to the first slab
    int8_t slab_type;   // 0 for on slab (slab struct and objs are continuous), 1 for off slab
    int8_t page_order;  // the order of page managed by one slab
    int16_t obj_size;   // size of obj
    int16_t num_per_slab;
    int16_t total_num; // num of total objs in cache
    int16_t free_num;  // num of free objs in cache
    int32_t cache_id;
};

mem_cache_t *cache_init(memory_system_t *mem_sys, int32_t obj_size, int32_t index);
int32_t cache_estimate(int32_t page_order, int32_t obj_size, int32_t slab_type, int32_t *left_size, int32_t *num_obj);
void *cache_alloc(memory_system_t *mem_sys, int32_t size);
int32_t cache_free(memory_system_t *mem_sys, void *addr);
slab_t *cache_grow(memory_system_t *mem_sys, mem_cache_t *cache, slab_t *end_slab);
int32_t cache_shrink(memory_system_t *mem_sys, mem_cache_t *cache);
int32_t cache_display(mem_cache_t *cache, int32_t index);
struct memory_system
{
    buddy_system_t *buddy_sys;
    // PT_t *pt_for_bd;
    mem_cache_t cache_array[MAX_NUM_CACHE];
    int16_t cache_size_array[MAX_NUM_CACHE];
    slab_t *off_slab_array;
    int32_t num_off_slab;
};

int32_t memory_init(memory_system_t *mem_sys);

// static uint32_t base_addr = NULL;

uint8_t *parse_args(uint8_t *command);
#endif /* ECE391SLAB_H */
