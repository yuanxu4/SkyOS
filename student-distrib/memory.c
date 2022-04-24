#include "memory.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define is_power_two(num) (!(num & (num - 1)))
#define left_child(index) ((index << 1) + 1)
#define right_child(index) ((index << 1) + 2)
#define parent(index) (((index + 1) >> 1) - 1)
#define get_size(order) (1 << (order))

memory_system_t mem_sys;

int32_t memory_init()
{
    int32_t i;
    page_table.pte[PT_SIZE - 1] |= 1; // set entry as present to store buddy system struct
    page_table.pte[PT_SIZE - 2] |= 1; // set entry as present to store new PT for sysyem
    mem_sys.buddy_sys = (buddy_system_t *)(SIZE_4MB - SIZE_4KB);
    mem_sys.pt_for_bd = (PT_t *)(SIZE_4MB - (SIZE_4KB << 1));
    init_buddy_sys(MAX_ORDER_PAGE);
    page_table.pte[PT_SIZE - 3] |= 1; // set 2 entries as present to store off slab
    page_table.pte[PT_SIZE - 4] |= 1; // set 2 entries as present to store off slab
    mem_sys.off_slab_array = (slab_t *)(SIZE_4MB - (SIZE_4KB << 2));
    memset_word(mem_sys.off_slab_array, 0, SIZE_4KB << 1);
    mem_sys.num_off_slab = 0;
    // init first 8 caches with 16B, 32B, ... ,2KB, last 8 caches unused
    for (i = 0; i < (MAX_NUM_CACHE >> 1); i++)
    {
        cache_init(get_size(i + 4), i);
        cache_init(-1, i + (MAX_NUM_CACHE >> 1));
    }

    return 0;
}

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

int32_t align_size(int32_t size, int32_t align_order)
{
    return ((size + get_size(align_order) - 1) >> align_order) << align_order;
}

void *high_level_alloc(int32_t size)
{
    void *ret_addr;
    if ((size <= 0) || (size > SIZE_4MB))
    {
        return NULL;
    }
    size = align_size(size, 2); // align to 4
    if (size < SIZE_4KB)
    {
        ret_addr = cache_alloc(size);
        if (ret_addr == NULL)
        {
            ret_addr = bd_alloc(size);
        }
    }
    else
    {
        ret_addr = bd_alloc(size);
    }
    return ret_addr;
}

buddy_system_t *init_buddy_sys(int32_t order)
{
    int32_t i;
    int16_t node_order;
    int32_t pte;
    // garbage check
    if ((order < 1) || (order > MAX_ORDER_PAGE))
    {
        return NULL;
    }
    // set new system
    memset_word(mem_sys.buddy_sys, -1, MAX_NUM_NODE + 1);
    mem_sys.buddy_sys->max_order = order;

    node_order = order + 1;
    for (i = 0; i < get_size(1 + order) - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            node_order -= 1;
        }
        mem_sys.buddy_sys->node_array[i] = node_order;
    }
    // ************************************************

    page_directory.pde[BASE_PD_INDEX] = (int32_t)mem_sys.pt_for_bd | 0x00000003; // set to 4KB mode, r/w, kernel, present
    // set pages in buddy system to r/w, kernel, not present
    pte = BASE_ADDR_BD_SYS | 0x00000002;
    for (i = 0; i < PT_SIZE; i++)
    {
        mem_sys.pt_for_bd->pte[i] = pte;
        pte += SIZE_4KB;
    }

    return mem_sys.buddy_sys;
}

// find the smaller child node which still >= order, alaways have a child >= order
int32_t choose_child(int32_t index, int8_t order)
{
    int32_t left_index = left_child(index);
    int8_t left_order = mem_sys.buddy_sys->node_array[left_index];
    int32_t right_index = right_child(index);
    int8_t right_order = mem_sys.buddy_sys->node_array[right_index];
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
    if (mem_sys.buddy_sys->node_array[0] < order)
    {
        return NULL;
    }
    // search down recursively to find the node to alloc
    for (node_order = mem_sys.buddy_sys->max_order; node_order != order; node_order -= 1)
    {
        index = choose_child(index, order);
    }
    // update node and get offset of addr
    mem_sys.buddy_sys->node_array[index] = -1;
    offset = (index + 1) * get_size(order) - get_size(mem_sys.buddy_sys->max_order);
    // update node order recursively up
    while (index)
    {
        index = parent(index);
        mem_sys.buddy_sys->node_array[index] = max(mem_sys.buddy_sys->node_array[left_child(index)], mem_sys.buddy_sys->node_array[right_child(index)]);
    }
    // todo: set alloced page to present*******************************
    for (size = get_size(order) - 1; size >= 0; size--)
    {
        mem_sys.pt_for_bd->pte[offset + size] |= 1;
    }

    mem_sys.pt_for_bd->pte[offset] |= 0x00000001;
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
    if ((offset < 0) || (offset > get_size(mem_sys.buddy_sys->max_order)))
    {
        return -1;
    }
    // init to bottom node order and node index
    node_order = 0;
    index = offset + get_size(mem_sys.buddy_sys->max_order) - 1;
    // search up recursively to find the node and order to free
    while (mem_sys.buddy_sys->node_array[index] != -1)
    {
        node_order++;
        index = parent(index);
        if (index < 0)
        {
            return -1;
        }
    }
    // restore node
    mem_sys.buddy_sys->node_array[index] = node_order;
    // todo: set pte to not present***********************************
    for (size = get_size(node_order) - 1; size >= 0; size--)
    {
        mem_sys.pt_for_bd->pte[offset + size] |= 1;
    }
    // update node order recursively up
    while (index)
    {
        index = parent(index);
        node_order++;
        left_order = mem_sys.buddy_sys->node_array[left_child(index)];
        right_order = mem_sys.buddy_sys->node_array[right_child(index)];
        if (left_order == right_order)
        {
            mem_sys.buddy_sys->node_array[index] = left_order + 1;
        }
        else
        {
            mem_sys.buddy_sys->node_array[index] = max(left_order, right_order);
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
    if ((offset < 0) || (offset > get_size(mem_sys.buddy_sys->max_order)))
    {
        return -1;
    }
    index = offset + get_size(mem_sys.buddy_sys->max_order) - 1;
    node_order = 0;
    while (mem_sys.buddy_sys->node_array[index] != -1)
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

void *bd_get_start(void *addr)
{
    int32_t offset;
    int32_t index;
    int8_t node_order;
    offset = ((int32_t)addr - BASE_ADDR_BD_SYS) >> 12;
    // garbage check
    if ((offset < 0) || (offset > get_size(mem_sys.buddy_sys->max_order)))
    {
        return NULL;
    }
    index = offset + get_size(mem_sys.buddy_sys->max_order) - 1;
    node_order = 0;
    while (mem_sys.buddy_sys->node_array[index] != -1)
    {
        node_order++;
        index = parent(index);
        if (index < 0)
        {
            return NULL;
        }
    }
    offset = (index + 1) * get_size(node_order) - get_size(mem_sys.buddy_sys->max_order);
    return (void *)(offset + BASE_ADDR_BD_SYS);
}

int32_t bd_display()
{
    int32_t num_node = get_size(1 + mem_sys.buddy_sys->max_order);
    int32_t curr_size = num_node;
    int32_t curr_order = mem_sys.buddy_sys->max_order + 1;
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
        printf("%*d", curr_size, mem_sys.buddy_sys->node_array[i]);
    }
    printf("\n");
    for (i = 0, curr_size = num_node, curr_order = mem_sys.buddy_sys->max_order + 1; i < num_node - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            curr_size >>= 1;
            curr_order--;
            idx = 0;
            printf("\n%d(%.2d): ", curr_order, curr_size);
        }
        if (mem_sys.buddy_sys->node_array[i] > -1)
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

int32_t get_slab_strcut_size()
{
    slab_t *slab;
    return (int32_t)((int8_t *)(slab + 1) - (int8_t *)slab);
}

int32_t find_unuse_off_slab()
{
    int32_t i;
    for (i = 0; i < MAX_NUM_OFF_SLAB; i++)
    {
        if (mem_sys.off_slab_array[i].cache == NULL)
        {
            return i;
        }
    }
    return -1;
}

int32_t slab_init(slab_t *new_slab, slab_t *next, mem_cache_t *cache, int32_t num_obj, void *obj_start, int32_t off_slab_index)
{
    int32_t i;
    new_slab->next = NULL;
    new_slab->cache = cache;
    new_slab->obj_size = new_slab->cache->obj_size;
    new_slab->off_slab_index = off_slab_index;
    new_slab->total_num = num_obj;
    new_slab->free_num = num_obj;
    new_slab->next_free_index = 0;
    new_slab->obj_start = obj_start;
    for (i = 0; i < num_obj; i++)
    {
        new_slab->free_id_list[i] = (int8_t)(i + 1);
    }
    new_slab->free_id_list[num_obj] = (int8_t)MAX_NUM_OBJ; // list end
    return 0;
}

mem_cache_t *cache_init(int32_t obj_size, int32_t index)
{
    int32_t left_size;
    int32_t num_obj;
    int32_t slab_type;
    int32_t curr_order;
    slab_t *new_slab;
    mem_cache_t *cache;
    int32_t i;
    int32_t off_slab_index;
    // garbage check
    if (index < 0 || index > MAX_NUM_CACHE)
    {
        return NULL;
    }
    cache = &mem_sys.cache_array[index];
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
        mem_sys.cache_size_array[index] = -1;
    }
    obj_size = align_size(obj_size, 2);                // align to 4
    slab_type = (obj_size >= (SIZE_4KB >> 4)) ? 1 : 0; // size >= 256, off slab

    for (curr_order = 0; curr_order <= MAX_ORDER_SLAB; curr_order++)
    {
        cache_estimate(curr_order, obj_size, slab_type, &left_size, &num_obj);
        if (num_obj == 0)
        {
            continue;
        }
        // leftsize <= slab size/8
        if (left_size <= ((SIZE_4KB << curr_order) >> 3))
        {
            break;
        }
    }
    if (num_obj == 0)
    {
        return NULL; // slab too small
    }
    if (slab_type == 1 && left_size >= get_slab_strcut_size())
    {
        slab_type = 0;
        left_size -= get_slab_strcut_size();
    }
    if (slab_type == 1)
    { // off slab
        off_slab_index = find_unuse_off_slab();
        if (off_slab_index == -1)
        {
            return NULL;
        }
        new_slab = &mem_sys.off_slab_array[off_slab_index];
        slab_init(new_slab, NULL, cache, num_obj, bd_alloc(curr_order), off_slab_index);
        mem_sys.num_off_slab++;
    }
    else
    { // on slab
        new_slab = (slab_t *)bd_alloc(curr_order);
        slab_init(new_slab, NULL, cache, num_obj, (void *)(new_slab + 1), -1);
    }
    cache->slabs_head = new_slab;
    cache->slab_type = slab_type;
    cache->page_order = curr_order;
    cache->obj_size = obj_size;
    cache->total_num = num_obj;
    cache->free_num = num_obj;
    cache->cache_id = index;
    mem_sys.cache_size_array[index] = obj_size;
    return cache;
}

int32_t cache_estimate(int32_t page_order, int32_t obj_size, int32_t slab_type, int32_t *left_size, int32_t *num_obj)
{
    int32_t temp_left_size;
    int32_t temp_num_obj;
    int32_t slab_size;
    slab_size = SIZE_4KB << page_order;
    // off type

    if (slab_type)
    {
        temp_num_obj = slab_size / obj_size;
    }
    else
    {
        temp_num_obj = (slab_size - get_slab_strcut_size()) / obj_size;
    }
    temp_num_obj = min(temp_num_obj, MAX_NUM_OBJ);
    *num_obj = temp_num_obj;
    *left_size = slab_size - temp_num_obj * obj_size;
    return 0;
};

void *slab_get_obj(slab_t *slab)
{
    void *ret_addr;
    if (slab == NULL)
    {
        return NULL;
    }
    ret_addr = slab->obj_start + slab->next_free_index * slab->obj_size;
    slab->next_free_index = slab->free_id_list[slab->next_free_index];
    slab->free_num--;
    slab->cache->free_num--;
    return ret_addr;
};

void *cache_alloc(int32_t size)
{
    int32_t good_size;
    int32_t good_index;
    int32_t first_free_index;
    slab_t *good_slab;
    slab_t *prev_slab;
    mem_cache_t *cache;
    int32_t curr_size;
    int32_t off_slab_index;
    void *ret_addr;

    int32_t i;
    good_size = SIZE_4KB;
    first_free_index = -1;
    for (i = 0; i < MAX_NUM_CACHE; i++)
    {
        curr_size = mem_sys.cache_size_array[i];
        if (first_free_index == -1 && curr_size == -1)
        {
            first_free_index = i;
            break;
        }
        if ((curr_size > size) && (curr_size < good_size))
        {
            good_size = curr_size;
            good_index = i;
        }
    }
    if (good_size == SIZE_4KB)
    {
        return NULL;
    }
    good_slab = NULL;
    // if size < 3/4 * good_size, not good enough, try to create new cache
    if (((good_size - size) << 2) > good_size)
    {
        if (first_free_index != -1)
        {
            good_slab = cache_init(size, first_free_index)->slabs_head;
        }
    }
    // good enough or create fail
    if (good_slab == NULL)
    {
        // alloc in existing slab
        good_slab = mem_sys.cache_array[good_index].slabs_head;
        cache = good_slab->cache;
        prev_slab = NULL;
        while (good_slab != NULL && good_slab->free_num == 0)
        {
            prev_slab = good_slab;
            good_slab = good_slab->next;
        }
        // all slab full, grow cache
        if (good_slab == NULL)
        {
            good_slab = cache_grow(cache, prev_slab);
        }
    }
    ret_addr = slab_get_obj(good_slab);
    return ret_addr;
}

int32_t slab_put_obj(slab_t *slab, void *obj)
{
    int32_t index;
    index = (obj - slab->obj_start) / slab->obj_size;
    slab->free_id_list[index] = slab->next_free_index;
    slab->next_free_index = index;
    slab->free_num++;
    slab->cache->free_num++;
    return 0;
}

int32_t cache_free(void *addr)
{
    void *start;
    slab_t *slab;
    int32_t i;
    start = bd_get_start(addr);
    i = 0;
    // check if obj belongs to the off slab
    while ((i < MAX_NUM_OFF_SLAB) && (mem_sys.off_slab_array[i].obj_start != start))
    {
        i++;
    }
    if (i == MAX_NUM_OFF_SLAB)
    { // in on slab, start is slab head
        slab = (slab_t *)start;
    }
    else
    { // in off slab
        slab = &mem_sys.off_slab_array[i];
    }
    slab_put_obj(slab, addr);
    return 0;
}

int32_t cache_shrink(mem_cache_t *cache)
{
    slab_t *curr_slab;
    slab_t *prev_slab;
    int32_t slab_type;
    curr_slab = cache->slabs_head;
    prev_slab = NULL;
    slab_type = cache->slab_type;
    if (cache == NULL || curr_slab == NULL)
    {
        return -1;
    }
    while (curr_slab != NULL)
    {
        // remove empry slab
        if (curr_slab->free_num == curr_slab->total_num)
        {
            cache->total_num -= curr_slab->total_num;
            cache->free_num -= curr_slab->total_num;
            // remove from linked list
            if (prev_slab == NULL)
            { // head
                cache->slabs_head = curr_slab->next;
            }
            else
            {
                prev_slab->next = curr_slab->next;
            }
            // free space
            if (slab_type == 1)
            { // off slab
                slab_init(curr_slab, NULL, NULL, -1, NULL, -1);
                mem_sys.num_off_slab--;
            }
            // for on slab, will be free directly since curr_slab==obj_start
            bd_free((void *)curr_slab->obj_start);
        }
        prev_slab = curr_slab;
        curr_slab = curr_slab->next;
    }
    return 0;
};

slab_t *cache_grow(mem_cache_t *cache, slab_t *end_slab)
{
    int32_t off_slab_index;
    slab_t *new_slab;
    if (cache->slab_type == 1)
    { // off slab
        off_slab_index = find_unuse_off_slab();
        if (off_slab_index == -1)
        {
            return NULL;
        }
        new_slab = &mem_sys.off_slab_array[off_slab_index];
        slab_init(new_slab, NULL, cache, end_slab->total_num, bd_alloc(cache->page_order), off_slab_index);
        mem_sys.num_off_slab++;
    }
    else
    { // on slab
        new_slab = (slab_t *)bd_alloc(cache->page_order);
        slab_init(new_slab, NULL, cache, end_slab->total_num, (void *)(new_slab + 1), -1);
    }
    end_slab->next = new_slab;
    cache->free_num += end_slab->total_num;
    cache->total_num += end_slab->total_num;
    return new_slab;
};
