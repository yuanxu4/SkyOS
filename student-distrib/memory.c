#include "memory.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define is_power_two(num) (!(num & (num - 1)))
#define left_child(index) ((index << 1) + 1)
#define right_child(index) ((index << 1) + 2)
#define parent(index) (((index + 1) >> 1) - 1)

buddy_system_t *buddy_sys = NULL;
PT_t *pt_for_bd = NULL;

// return the order n, 2^n>=num>2^(n-1)
// assume size <2^31
int8_t get_order(uint32_t num)
{
    int8_t order = 0;
    num -= 1;
    num |= (num >> 1);
    num |= (num >> 2);
    num |= (num >> 4);
    num |= (num >> 8);
    num |= (num >> 16);
    num++; // get n, which is a power of 2 and n>=num>n/2
    while (num > 1)
    {
        num >>= 1;
        order++;
    }
    return order;
}

void *high_level_alloc(int32_t size)
{
    if ((size <= 0) || (size > SIZE_4MB))
    {
        return NULL;
    }
    if (size < SIZE_4KB)
    {
        // slab alloc
    }
    else
    {
        // buddy alloc
    }

    return NULL;
}

buddy_system_t *init_buddy_sys(int32_t order)
{
    int32_t i;
    buddy_system_t *new_sys;
    int16_t node_order;
    int32_t pte;
    // garbage check
    if ((order < 1) || (order > MAX_ORDER))
    {
        return NULL;
    }
    // if has inited
    if (buddy_sys != NULL)
    {
        return buddy_sys;
    }
    // set new system
    page_table.pte[PT_SIZE - 1] |= 1; // set entry as present to store buddy system struct
    page_table.pte[PT_SIZE - 2] |= 1; // set entry as present to store new PT for sysyem
    new_sys = (buddy_system_t *)(SIZE_4MB - SIZE_4KB);
    memset_dword(new_sys, -1, MAX_NUM_NODE + 1);
    buddy_sys = new_sys;
    new_sys->max_order = order;

    node_order = order + 1;
    for (i = 0; i < (1 << (order + 1)) - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            node_order -= 1;
        }
        new_sys->node_array[i] = node_order;
    }
    // ************************************************
    pt_for_bd = (PT_t *)(SIZE_4MB - (SIZE_4KB << 1));
    page_directory.pde[BASE_PD_INDEX] = (SIZE_4MB - (SIZE_4KB << 1)) | 0x00000003; // set to 4KB mode, r/w, kernel, present
    // set pages in buddy system to r/w, kernel, not present
    pte = BASE_ADDR_BD_SYS | 0x00000002;
    for (i = 0; i < PT_SIZE; i++)
    {
        pt_for_bd->pte[i] = pte;
        pte += SIZE_4KB;
    }

    return new_sys;
}

// find the smaller child node which still >= order, alaways have a child >= order
int32_t choose_child(int32_t index, int8_t order)
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

void *bd_alloc(int32_t order)
{
    int8_t node_order;
    int32_t size;
    int32_t index;
    int32_t offset;
    // too large
    if (buddy_sys->node_array[0] < order)
    {
        return NULL;
    }
    // search down recursively to find the node to alloc
    for (node_order = buddy_sys->max_order; node_order != order; node_order -= 1)
    {
        index = choose_child(index, order);
    }
    // update node and get offset of addr
    buddy_sys->node_array[index] = -1;
    offset = (index + 1) * (1 << order) - (1 << buddy_sys->max_order);
    // update node order recursively up
    while (index)
    {
        index = parent(index);
        buddy_sys->node_array[index] = max(buddy_sys->node_array[left_child(index)], buddy_sys->node_array[right_child(index)]);
    }
    // todo: set alloced page to present*******************************
    for (size = (1 << order) - 1; size >= 0; size--)
    {
        pt_for_bd->pte[offset + size] |= 1;
    }

    pt_for_bd->pte[offset] |= 0x00000001;
    // return addr
    return (void *)(offset + BASE_ADDR_BD_SYS);
}

int32_t bd_free(void *addr)
{
    int8_t node_order;
    int32_t index;
    int32_t offset;
    int32_t size;
    int8_t left_order;
    int8_t right_order;
    offset = ((int32_t)addr - BASE_ADDR_BD_SYS) >> 12;
    // garbage check
    if ((offset < 0) || (offset > (1 << buddy_sys->max_order)))
    {
        return -1;
    }
    // init to bottom node order and node index
    node_order = 0;
    index = offset + (1 << buddy_sys->max_order) - 1;
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
    // todo: set pte to not present***********************************
    for (size = (1 << node_order) - 1; size >= 0; size--)
    {
        pt_for_bd->pte[offset + size] |= 1;
    }
    // update node order recursively up
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
    return 0;
}

int32_t bd_get_size(void *addr)
{
    int32_t offset;
    int32_t index;
    int8_t node_order;
    offset = ((int32_t)addr - BASE_ADDR_BD_SYS) >> 12;
    // garbage check
    if ((offset < 0) || (offset > (1 << buddy_sys->max_order)))
    {
        return -1;
    }
    index = offset + (1 << buddy_sys->max_order) - 1;
    node_order = 0;
    while (buddy_sys->node_array[index] != -1)
    {
        node_order++;
        index = parent(index);
        if (index < 0)
        {
            return -1;
        }
    }
    return (1 << node_order);
}

int32_t bd_display()
{
    int32_t num_node = (2 << buddy_sys->max_order);
    int32_t curr_size = num_node;
    int32_t curr_order = buddy_sys->max_order + 1;
    int32_t i, j;
    char cs[2] = {'/', '\\'};
    int32_t idx = 0;
    char c;
    for (i = 0; i < num_node - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            curr_size >>= 1;
            curr_order--;
            idx = 0;
            printf("\n%d(%.2d): ", curr_order, curr_size);
        }
        printf("%*d", curr_size, buddy_sys->node_array[i]);
    }
    printf("\n");
    for (i = 0, curr_size = num_node, curr_order = buddy_sys->max_order + 1; i < num_node - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            curr_size >>= 1;
            curr_order--;
            idx = 0;
            printf("\n%d(%.2d): ", curr_order, curr_size);
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
            printf("%c", c);
        }
    }
    printf("\n");
    return 0;
}
