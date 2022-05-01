#if !defined(ECE391BUDDY_H)
#define ECE391BUDDY_H

#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"

#define NULL 0
#define MAX_ORDER_PAGE 6                       // max order of pages in the buddy system
#define MAX_NUM_PAGE (1 << MAX_ORDER_PAGE)     // max num of pages in the buddy system
#define MAX_NUM_NODE ((MAX_NUM_PAGE << 1) - 1) // 2047, max num of nodes in the buddy system
#define BUFSIZE 1024
#define SIZE_4KB 4096

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

#endif /* ECE391BUDDY_H */
