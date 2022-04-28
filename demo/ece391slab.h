#if !defined(ECE391SLAB_H)
#define ECE391SLAB_H

#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define NULL 0
#define MAX_ORDER_PAGE 11                  // max order of pages in the buddy system
#define MAX_NUM_PAGE (1 << MAX_ORDER_PAGE) // max num of pages in the buddy system
#define SIZE_2KB 0x800
#define MAX_NUM_NODE ((MAX_NUM_PAGE << 1) - 1) // 2047, max num of nodes in the buddy system
#define BASE_ADDR_BD_SYS 0x40000000            // 1GB, base addr of buddy system
#define BASE_PD_INDEX 256                      // 1GB/4MB-1, pde index of base addr
#define MAX_NUM_OBJ 256                        // 4K/16, max num of obj in one slab
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

struct slab
{                       // size is 288B
    slab_t *next;       // next slab
    mem_cache_t *cache; // the cache manages this slab
    // slab_t *head; // the first slab
    int32_t obj_size;                 // size of obj
    int32_t off_slab_index;           // used if it is a off slab, -1 of on slab
    int32_t total_num;                // num of total objs in this slab
    int32_t free_num;                 // num of free objs in this slab
    int32_t next_free_index;          // index of next free obj
    void *obj_start;                  // start addr of objs
    int8_t free_id_list[MAX_NUM_OBJ]; // store the id of next free obj, -1 for using obj
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
    int16_t total_num;  // num of total objs in cache
    int16_t free_num;   // num of free objs in cache
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

int32_t memory_init(memory_system_t *mem_sys)
{
    buddy_system_t buddy_sys;
    slab_t off_slab_array[MAX_NUM_OFF_SLAB];
    mem_sys->buddy_sys = &buddy_sys;
    mem_sys->off_slab_array = &off_slab_array[0];
    mem_sys->num_off_slab = 0;
    /*
    // int32_t i = 1;
    // int32_t num_pte = PAGE_NUM_FOR_OFF_SLAB;
    // // set pte as present to store off slab
    // while (num_pte >= i)
    // {
    //     page_table.pte[PT_SIZE - i] |= 1;
    //     i++;
    // }
    // mem_sys->off_slab_array = (slab_t *)(SIZE_4MB - (SIZE_4KB * num_pte));
    // // memset(mem_sys->off_slab_array, 0, (SIZE_4KB * num_pte));
    // mem_sys->num_off_slab = 0;
    // // set pte as present to store page table for pages in buddy system
    // num_pte += MAX_NUM_PAGE / PT_SIZE;
    // while (num_pte >= i)
    // {
    //     page_table.pte[PT_SIZE - i] |= 1;
    //     i++;
    // }
    // mem_sys->pt_for_bd = (PT_t *)(SIZE_4MB - (SIZE_4KB * num_pte));
    // // memset(mem_sys->pt_for_bd, 0, (SIZE_4KB * num_pte));
    // int32_t j;
    // for (j = 0; j < num_pte - PAGE_NUM_FOR_OFF_SLAB; j++)
    // {
    //     page_directory.pde[BASE_PD_INDEX + j] = (int32_t)(mem_sys->pt_for_bd + j) | 0x00000007; // set to 4KB mode, r/w, kernel, present
    // }
    // int32_t k;
    // // set pages in buddy system to r/w, kernel, not present
    // int32_t pte = BASE_ADDR_BD_SYS | 0x00000002;
    // uint32_t *pte_addr = ((uint32_t *)mem_sys->pt_for_bd);
    // for (k = 0; k < MAX_NUM_PAGE; k++)
    // {
    //     *pte_addr = pte;
    //     pte += SIZE_4KB;
    //     pte_addr++;
    // }
    // // set entry as present to store buddy system struct
    // num_pte += (MAX_NUM_NODE + 1) / SIZE_4KB;
    // while (num_pte >= i)
    // {
    //     page_table.pte[PT_SIZE - i] |= 1;
    //     i++;
    // }
    // mem_sys->buddy_sys = (buddy_system_t *)(SIZE_4MB - (SIZE_4KB * num_pte));
    // memset(mem_sys->buddy_sys, 0, (SIZE_4KB * num_pte));
    */
    // init buddy system
    init_buddy_sys(mem_sys->buddy_sys, MAX_ORDER_PAGE);
    // init first 8 caches with 16B, 32B, ... ,2KB, last 8 caches unused
    int32_t i;
    for (i = 0; i < (MAX_NUM_CACHE >> 1); i++)
    {
        cache_init(mem_sys, get_size(i + 6), i);
        cache_init(mem_sys, -1, i + (MAX_NUM_CACHE >> 1));
    }
    return 0;
}

// return the order n, 2^n>=num>2^(n-1)
// assume size <2^31
int8_t get_order(uint32_t num)
{
    int8_t order = 0;
    if (is_power_two(num))
    {
        while (num > 1)
        {
            num >>= 1;
            order++;
        }
    }
    else
    {
        while (num > 0)
        {
            num >>= 1;
            order++;
        }
    }
    return order;
}

int32_t align_size(int32_t size, int32_t align_order)
{
    return ((size + get_size(align_order) - 1) >> align_order) << align_order;
}

void *k_alloc(memory_system_t *mem_sys, int32_t size)
{
    void *ret_addr;
    if ((size <= 0) || (size > SIZE_4MB))
    {
        return NULL;
    }
    size = align_size(size, 2); // align to 4
    if (size < SIZE_4KB)
    {
        ret_addr = cache_alloc(mem_sys, size);
        if (ret_addr == NULL)
        {
            ret_addr = bd_alloc(mem_sys->buddy_sys, get_order(size / SIZE_4KB));
        }
    }
    else
    {
        ret_addr = bd_alloc(mem_sys->buddy_sys, get_order(size / SIZE_4KB));
    }
    return ret_addr;
}

int32_t k_free(memory_system_t *mem_sys, void *addr)
{
    print((uint8_t *)"k_free at %x\n", addr);
    if (cache_free(mem_sys, addr) == -1 && bd_free(mem_sys->buddy_sys, addr) == -1)
    {
        printf("k_free fail\n");
        return -1;
    }
    printf("k_free succ\n");
    return 0;
}

uint32_t base_addr = NULL;

buddy_system_t *init_buddy_sys(buddy_system_t *buddy_sys, int32_t order)
{
    // print((uint8_t*)"start init\n");
    // garbage check
    if ((order < 1) || (order > MAX_ORDER_PAGE))
    {
        return NULL;
    }
    int32_t i;
    // init fields
    buddy_sys->max_order = order;
    // init the order fo every node, get_size(1 + order) = num of nodes
    int16_t node_order = order + 1;
    for (i = 0; i < get_size(1 + order) - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            node_order -= 1;
        }
        buddy_sys->node_array[i] = node_order;
        // print((uint8_t*)"%d ",buddy_sys->node_array[i]);
    }
    base_addr = (int32_t)ece391_alloc(get_size(order) * SIZE_4KB);
    print((uint8_t *)"base addr:%#x\n", base_addr);
    return buddy_sys;
}

// find the smaller child node which still >= order, alaways have a child >= order
int32_t choose_child(buddy_system_t *buddy_sys, int32_t index, int8_t order)
{
    int32_t left_index = left_child(index);
    int8_t left_order = buddy_sys->node_array[left_index];
    int32_t right_index = right_child(index);
    int8_t right_order = buddy_sys->node_array[right_index];
    if (left_order < order)
    {
        return right_index;
    }
    if (right_order < order)
    {
        return left_index;
    }
    return ((left_order <= right_order) ? left_index : right_index);
}

void *bd_alloc(buddy_system_t *buddy_sys, int32_t order)
{
    // size to alloc too large
    if (buddy_sys->node_array[0] < order)
    {
        print((uint8_t *)"alloc fail\n");
        return NULL;
    }
    // search down recursively to find the node to alloc
    int8_t node_order; // the expected order of node, the max order of every layer.
    int32_t index = 0; // node index
    for (node_order = buddy_sys->max_order; node_order != order; node_order -= 1)
    {
        index = choose_child(buddy_sys, index, order);
    }
    // update node and get offset of addr
    buddy_sys->node_array[index] = -1;
    int32_t offset = (index + 1) * get_size(order) - get_size(buddy_sys->max_order);
    // update node order recursively up
    while (index)
    {
        index = parent(index);
        buddy_sys->node_array[index] = max(buddy_sys->node_array[left_child(index)], buddy_sys->node_array[right_child(index)]);
    }
    // return addr
    return (void *)(offset * SIZE_4KB + base_addr);
}

int32_t bd_free(buddy_system_t *buddy_sys, void *addr)
{
    int32_t offset = ((int32_t)addr - (int32_t)base_addr) / SIZE_4KB;
    // garbage check
    if ((offset < 0) || (offset > get_size(buddy_sys->max_order)))
    {
        print((uint8_t *)"free fail: offset: %d\n", offset);
        return -1;
    }
    // init to bottom node order and node index
    int8_t node_order = 0;
    int32_t index = offset + get_size(buddy_sys->max_order) - 1;
    // search up recursively to find the node and order to free
    while (buddy_sys->node_array[index] != -1)
    {
        node_order++;
        index = parent(index);
        if (index < 0)
        {
            return -1;
        }
    }
    // restore node
    buddy_sys->node_array[index] = node_order;
    // update node order recursively up
    int8_t left_order;
    int8_t right_order;
    while (index)
    {
        index = parent(index);
        node_order++;
        left_order = buddy_sys->node_array[left_child(index)];
        right_order = buddy_sys->node_array[right_child(index)];
        if (left_order == right_order)
        {
            buddy_sys->node_array[index] = left_order + 1;
        }
        else
        {
            buddy_sys->node_array[index] = max(left_order, right_order);
        }
    }
    // ece391_free(addr);
    return 0;
}

int32_t bd_get_size(buddy_system_t *buddy_sys, void *addr)
{
    int32_t offset = ((int32_t)addr - base_addr) >> 12;
    // garbage check
    if ((offset < 0) || (offset > get_size(buddy_sys->max_order)))
    {
        return -1;
    }
    int32_t index = offset + get_size(buddy_sys->max_order) - 1;
    int8_t node_order = 0;
    while (buddy_sys->node_array[index] != -1)
    {
        node_order++;
        index = parent(index);
        if (index < 0)
        {
            return -1;
        }
    }
    return get_size(node_order);
}

void *bd_get_start(buddy_system_t *buddy_sys, void *addr)
{
    int32_t offset = ((int32_t)addr - base_addr) >> 12;
    // garbage check
    if ((offset < 0) || (offset > get_size(buddy_sys->max_order)))
    {
        return NULL;
    }
    int32_t index = offset + get_size(buddy_sys->max_order) - 1;
    int8_t node_order = 0;
    while (buddy_sys->node_array[index] != -1)
    {
        node_order++;
        index = parent(index);
        if (index < 0)
        {
            return NULL;
        }
    }
    offset = (index + 1) * get_size(node_order) - get_size(buddy_sys->max_order);
    return (void *)(offset * SIZE_4KB + base_addr);
}
int32_t print_int(int32_t i)
{
    uint8_t buf[BUFSIZE];
    ece391_itoa(1, buf, 10);
    ece391_fdputs(1, buf);
    return 0;
}

int32_t bd_display(buddy_system_t *buddy_sys)
{
    int32_t num_node = get_size(1 + buddy_sys->max_order);
    // print((uint8_t*)"num_node: %d\n", num_node);
    int32_t curr_size = num_node;
    int32_t node_order;
    int32_t curr_order = buddy_sys->max_order + 1;
    int32_t i, j;
    uint8_t cs[2] = {'/', '\\'};
    int32_t idx = 0;
    uint8_t c;
    for (i = 0; i < num_node - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            curr_size >>= 1;
            curr_order--;
            idx = 0;
            ece391_fdputc(1, (uint8_t *)"\n");
            print((uint8_t *)"%d: ", curr_order);
        }
        node_order = buddy_sys->node_array[i];
        if (node_order < 0)
        {
            for (j = 0; j < curr_size - 1; j++)
            {
                ece391_fdputc(1, (uint8_t *)" ");
            }
            ece391_fdputc(1, (uint8_t *)"x");
        }
        else
        {
            if (get_size(node_order) < 10)
            {
                for (j = 0; j < curr_size - 1; j++)
                {
                    ece391_fdputc(1, (uint8_t *)" ");
                }
                print((uint8_t *)"%d", get_size(node_order));
            }
            else
            {
                for (j = 0; j < curr_size - 2; j++)
                {
                    ece391_fdputc(1, (uint8_t *)" ");
                }
                print((uint8_t *)"%d", get_size(node_order));
            }
        }
    }
    ece391_fdputs(1, (uint8_t *)"\n");
    for (i = 0, curr_size = num_node, curr_order = buddy_sys->max_order + 1; i < num_node - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            curr_size >>= 1;
            curr_order--;
            idx = 0;
            ece391_fdputc(1, (uint8_t *)"\n");
            print((uint8_t *)"%d: ", curr_order);
        }
        if (buddy_sys->node_array[i] > -1)
        {
            c = '-';
        }
        else
        {
            c = cs[idx];
            idx ^= 0x1;
        }
        for (j = 0; j < curr_size; j++)
        {
            ece391_fdputc(1, (uint8_t *)&c);
        }
    }
    ece391_fdputc(1, (uint8_t *)"\n");
    return 0;
}

/*
 * int32_t parse_args(uint8_t *command)
 * split executable name and arguments
 * Inputs:  command - the input command
 * Outputs: None
 * Side Effects: change command
 * return value: the pointer to arguments
 */
uint8_t *parse_args(uint8_t *command)
{
    // printf("%s\n", command);
    uint8_t *args = NULL;
    while (*command != '\0')
    {
        if (*command == ' ')
        {
            *command = '\0';    // terminate cmd, only store the exe file name
            args = command + 1; // output args string
            break;
        }
        command++;
    }
    return args;
}

int32_t get_slab_strcut_size()
{
    slab_t *slab;
    return (int32_t)(slab + 1) - (int32_t)slab;
}

int32_t find_unuse_off_slab(memory_system_t *mem_sys)
{
    int32_t i;
    for (i = 0; i < MAX_NUM_OFF_SLAB; i++)
    {
        if (mem_sys->off_slab_array[i].cache == NULL)
        {
            return i;
        }
    }
    return -1;
}

int32_t slab_init(slab_t *new_slab, slab_t *next, mem_cache_t *cache, int32_t num_obj, int32_t obj_size, void *obj_start, int32_t off_slab_index)
{
    int32_t i;
    new_slab->next = next;
    new_slab->cache = cache;
    new_slab->obj_size = obj_size;
    new_slab->off_slab_index = off_slab_index;
    new_slab->total_num = num_obj;
    new_slab->free_num = num_obj;
    new_slab->next_free_index = 0;
    new_slab->obj_start = obj_start;
    for (i = 0; i < num_obj; i++)
    {
        new_slab->free_id_list[i] = (int8_t)(i + 1);
    }
    // new_slab->free_id_list[num_obj] = (int8_t)MAX_NUM_OBJ; // list end
    return 0;
}

mem_cache_t *cache_init(memory_system_t *mem_sys, int32_t obj_size, int32_t index)
{
    // garbage check
    if (index < 0 || index > MAX_NUM_CACHE)
    {
        return NULL;
    }
    mem_cache_t *cache = &mem_sys->cache_array[index];
    // init as invaild
    if (obj_size <= 0)
    {
        cache->slabs_head = NULL;
        cache->slab_type = -1;
        cache->page_order = -1;
        cache->obj_size = -1;
        cache->total_num = -1;
        cache->free_num = -1;
        cache->cache_id = -1;
        mem_sys->cache_size_array[index] = -1;
        return cache;
    }
    obj_size = align_size(obj_size, 2);                        // align to 4
    int32_t slab_type = (obj_size >= (SIZE_4KB >> 4)) ? 1 : 0; // size >= 256, off slab
    int32_t curr_order;                                        // the order of num of pages managed by a slab
    int32_t left_size;                                         // the unused size in the slab page
    int32_t num_obj;                                           // the num of obj in the slab
    // try to find an appropriate slab size
    for (curr_order = 0; curr_order <= MAX_ORDER_SLAB; curr_order++)
    {
        cache_estimate(curr_order, obj_size, slab_type, &left_size, &num_obj);
        if (num_obj == 0)
        {
            continue; // curr slab size is too small
        }
        // leftsize <= slab size/8 or num obj reaches max, it is acceptable
        if (left_size <= ((SIZE_4KB << curr_order) >> 3) || (num_obj == MAX_NUM_OBJ))
        {
            break;
        }
    }
    // cannot find big enough slab
    if (num_obj == 0)
    {
        return NULL;
    }
    // for off slab can transfer to on slab
    if (slab_type == 1 && left_size >= get_slab_strcut_size())
    {
        slab_type = 0;
        left_size -= get_slab_strcut_size();
    }
    // init slab according type
    slab_t *new_slab;
    int32_t off_slab_index; // index for off slab
    if (slab_type == 1)
    { // off slab
        off_slab_index = find_unuse_off_slab(mem_sys);
        if (off_slab_index == -1)
        {
            return NULL;
        }
        new_slab = &mem_sys->off_slab_array[off_slab_index];
        slab_init(new_slab, NULL, cache, num_obj, obj_size, bd_alloc(mem_sys->buddy_sys, curr_order), off_slab_index);
        mem_sys->num_off_slab++;
    }
    else
    { // on slab
        new_slab = (slab_t *)bd_alloc(mem_sys->buddy_sys, curr_order);
        slab_init(new_slab, NULL, cache, num_obj, obj_size, (void *)(new_slab + 1), -1);
    }
    // init cache
    cache->slabs_head = new_slab;
    cache->slab_type = slab_type;
    cache->page_order = curr_order;
    cache->obj_size = obj_size;
    cache->total_num = num_obj;
    cache->free_num = num_obj;
    cache->cache_id = index;
    mem_sys->cache_size_array[index] = obj_size;
    return cache;
}

int32_t cache_estimate(int32_t page_order, int32_t obj_size, int32_t slab_type, int32_t *left_size, int32_t *num_obj)
{
    // get total size for obj in slab according to the type
    int32_t slab_size = SIZE_4KB << page_order;
    slab_size = (slab_type == 1) ? slab_size : (slab_size - get_slab_strcut_size());
    // get num obj
    int32_t temp_num_obj = min((slab_size / obj_size), MAX_NUM_OBJ);
    *num_obj = temp_num_obj;
    *left_size = slab_size - temp_num_obj * obj_size;
    return 0;
};

void *slab_get_obj(slab_t *slab)
{
    if (slab == NULL)
    {
        return NULL;
    }
    // get addr with next_free_index
    uint8_t *ret_addr = (uint8_t *)slab->obj_start + slab->next_free_index * slab->obj_size;
    // update next_free_index from free_id_list
    slab->next_free_index = slab->free_id_list[slab->next_free_index];
    slab->free_id_list[slab->next_free_index] = -1; // mark as used
    slab->free_num--;
    slab->cache->free_num--;
    return (void *)ret_addr;
};

// size < SIZE_4KB
void *cache_alloc(memory_system_t *mem_sys, int32_t size)
{
    int32_t i;
    int32_t good_size = SIZE_4KB;  // a good existing size which > given size
    int32_t good_index;            // the id of cache with good_size
    int32_t curr_size;             // curr size in loop
    int32_t first_free_index = -1; // the first free cache id, used if create new cache
    for (i = 0; i < MAX_NUM_CACHE; i++)
    {
        curr_size = mem_sys->cache_size_array[i];
        // update first_free_index
        if (first_free_index == -1 && curr_size == -1)
        {
            first_free_index = i;
        }
        // update good cache if size < curr_size < good_size
        if ((curr_size > size) && (curr_size < good_size))
        {
            good_size = curr_size;
            good_index = i;
        }
    }
    slab_t *good_slab = NULL;
    // if size < 3/4 * good_size or size > all existing size, not good enough, try to create new cache
    if (((good_size - size) << 2) > good_size || good_size == SIZE_4KB)
    {
        if (first_free_index != -1)
        {
            good_slab = cache_init(mem_sys, size, first_free_index)->slabs_head;
        }
    }
    // never create new slab or create fail
    if (good_slab == NULL)
    {
        // create fail and size > all existing size
        if (good_size == SIZE_4KB)
        {
            return NULL;
        }
        // alloc in existing slab
        good_slab = mem_sys->cache_array[good_index].slabs_head;
        mem_cache_t *cache = good_slab->cache;
        slab_t *prev_slab = NULL;
        // find avaliable slab
        while (good_slab != NULL && good_slab->free_num == 0)
        {
            prev_slab = good_slab;
            good_slab = good_slab->next;
        }
        // all slab full, grow cache
        if (good_slab == NULL)
        {
            good_slab = cache_grow(mem_sys, cache, prev_slab);
        }
    }
    void *ret_val = slab_get_obj(good_slab);
    return ret_val;
}

int32_t slab_put_obj(slab_t *slab, void *obj)
{
    int32_t index = (int32_t)((uint8_t *)obj - (uint8_t *)slab->obj_start) / slab->obj_size;
    // have free
    if (slab->free_id_list[index] != -1)
    {
        return -1;
    }
    slab->free_id_list[index] = slab->next_free_index;
    slab->next_free_index = index;
    slab->free_num++;
    slab->cache->free_num++;
    return 0;
}

int32_t cache_free(memory_system_t *mem_sys, void *addr)
{
    slab_t *slab;
    // get the start addr of pages
    void *start = bd_get_start(mem_sys->buddy_sys, addr);
    if (start == NULL)
    {
        return -1;
    }
    int32_t i = 0;
    // check if obj belongs to the off slab, can find obj_start = start if off slab
    while ((i < MAX_NUM_OFF_SLAB) && (mem_sys->off_slab_array[i].obj_start != start))
    {
        i++;
    }
    if (i == MAX_NUM_OFF_SLAB)
    { // in on slab, start is slab head
        slab = (slab_t *)start;
        // check if a vaild slab, if not, addr may not alloced by slab
        if (slab->obj_start != start)
        {
            return -1;
        }
    }
    else
    { // in off slab
        slab = &mem_sys->off_slab_array[i];
    }
    int32_t ret_val = slab_put_obj(slab, addr);
    return ret_val;
}

int32_t cache_shrink(memory_system_t *mem_sys, mem_cache_t *cache)
{
    if (cache == NULL)
    {
        return -1;
    }
    slab_t *curr_slab = cache->slabs_head;
    slab_t *prev_slab = NULL;
    slab_t *rm_slab = NULL;
    int32_t slab_type = cache->slab_type;
    while (curr_slab != NULL)
    {
        // remove empty slab
        if (curr_slab->free_num == curr_slab->total_num)
        {
            rm_slab = curr_slab;
            cache->total_num -= rm_slab->total_num;
            cache->free_num -= rm_slab->total_num;
            // remove from linked list
            if (prev_slab == NULL)
            { // head
                cache->slabs_head = rm_slab->next;
                prev_slab = prev_slab;
                curr_slab = cache->slabs_head;
            }
            else
            {
                prev_slab->next = rm_slab->next;
                prev_slab = prev_slab;
                curr_slab = prev_slab->next;
            }
            // free space
            if (slab_type == 1)
            { // off slab
                slab_init(rm_slab, NULL, NULL, -1, -1, NULL, -1);
                mem_sys->num_off_slab--;
            }
            // for on slab, will be free directly since curr_slab==obj_start
            bd_free(mem_sys->buddy_sys, (void *)rm_slab->obj_start);
        }
        else
        {
            prev_slab = curr_slab;
            curr_slab = curr_slab->next;
        }
    }
    return 0;
};

slab_t *cache_grow(memory_system_t *mem_sys, mem_cache_t *cache, slab_t *end_slab)
{
    int32_t off_slab_index;
    slab_t *new_slab;
    if (cache->slab_type == 1)
    { // off slab
        off_slab_index = find_unuse_off_slab(mem_sys);
        if (off_slab_index == -1)
        {
            return NULL;
        }
        new_slab = &mem_sys->off_slab_array[off_slab_index];
        slab_init(new_slab, NULL, cache, end_slab->total_num, cache->obj_size, bd_alloc(mem_sys->buddy_sys, cache->page_order), off_slab_index);
        mem_sys->num_off_slab++;
    }
    else
    { // on slab
        new_slab = (slab_t *)bd_alloc(mem_sys->buddy_sys, cache->page_order);
        slab_init(new_slab, NULL, cache, end_slab->total_num, cache->obj_size, (void *)(new_slab + 1), -1);
    }
    end_slab->next = new_slab;
    cache->free_num += end_slab->total_num;
    cache->total_num += end_slab->total_num;
    cache_display(cache, -1);
    return new_slab;
};

int32_t slab_display(slab_t *slab, int32_t index)
{
    print((uint8_t *)"\n--[slab %d] ", index);
    if (slab->off_slab_index >= 0)
    {
        print((uint8_t *)"OFF id[%d] ", slab->off_slab_index);
    }
    else
    {
        print((uint8_t *)"ON type ");
    }
    print((uint8_t *)"obj size[%d]", slab->obj_size);
    print((uint8_t *)"# of obj[%d]", slab->total_num);
    print((uint8_t *)"# of free obj[%d]", slab->free_num);
    print((uint8_t *)"\n        ");
    int32_t obj_index = 0;
    for (obj_index = 0; obj_index < slab->total_num; slab->total_num++)
    {
        if (slab->free_id_list[obj_index])
        {
            print((uint8_t *)"x");
        }
        else
        {
            print((uint8_t *)"_");
        }
    }
    return 0;
}

int32_t cache_display(mem_cache_t *cache, int32_t index)
{
    if (index < 0)
    {
        print((uint8_t *)"[cache] ");
    }
    else
    {
        print((uint8_t *)"[cache %d] ", index);
    }
    if (cache->slab_type == 0)
    {
        print((uint8_t *)"type[ON] ");
    }
    else
    {
        print((uint8_t *)"type[OFF] ");
    }
    print((uint8_t *)"slab size[%d]", SIZE_4KB * get_size(cache->page_order));
    print((uint8_t *)"obj size[%d]", cache->obj_size);
    print((uint8_t *)"# of obj[%d]", cache->total_num);
    print((uint8_t *)"# of free obj[%d]", cache->free_num);
    print((uint8_t *)"\n        ");
    slab_t *curr_slab = cache->slabs_head;
    while (curr_slab != NULL)
    {
        if (curr_slab->free_num == 0)
        {
            print((uint8_t *)"x");
        }
        else if (curr_slab->free_num == curr_slab->total_num)
        {
            print((uint8_t *)"_");
        }
        else
        {
            print((uint8_t *)"-");
        }
        curr_slab = curr_slab->next;
    }
    curr_slab = cache->slabs_head;
    int32_t slab_index = 0;
    while (curr_slab != NULL)
    {
        slab_display(curr_slab, slab_index);
        curr_slab = curr_slab->next;
    }

    return 0;
}

#endif /* ECE391SLAB_H */
