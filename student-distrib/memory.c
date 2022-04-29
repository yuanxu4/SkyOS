#include "memory.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define is_power_two(num) (!((num) & (num - 1)))
#define left_child(index) ((index << 1) + 1)
#define right_child(index) ((index << 1) + 2)
#define parent(index) (((index + 1) >> 1) - 1)
#define get_size(order) (1 << (order))

memory_system_t mem_sys;

int32_t memory_init()
{
    int32_t i = 1;
    int32_t num_pte = PAGE_NUM_FOR_OFF_SLAB;
    // set pte as present to store off slab
    while (num_pte >= i)
    {
        page_table.pte[PT_SIZE - i] |= 1;
        i++;
    }
    mem_sys.off_slab_array = (slab_t *)(SIZE_4MB - (SIZE_4KB * num_pte));
    // memset(mem_sys.off_slab_array, 0, (SIZE_4KB * num_pte));
    mem_sys.num_off_slab = 0;
    // set pte as present to store page table for pages in buddy system
    num_pte += MAX_NUM_PAGE / PT_SIZE;
    while (num_pte >= i)
    {
        page_table.pte[PT_SIZE - i] |= 1;
        i++;
    }
    mem_sys.pt_for_bd = (PT_t *)(SIZE_4MB - (SIZE_4KB * num_pte));
    printf("pt_for_bd:%#x\n", mem_sys.pt_for_bd);
    // memset(mem_sys.pt_for_bd, 0, (SIZE_4KB * num_pte));
    int32_t j;
    for (j = 0; j < num_pte - PAGE_NUM_FOR_OFF_SLAB; j++)
    {
        page_directory.pde[BASE_PD_INDEX + j] = (int32_t)(mem_sys.pt_for_bd + j) | 0x00000007; // set to 4KB mode, r/w, kernel, present
    }
    int32_t k;
    // set pages in buddy system to r/w, kernel, not present
    int32_t pte = BASE_ADDR_BD_SYS | 0x00000006;
    uint32_t *pte_addr = ((uint32_t *)mem_sys.pt_for_bd);
    for (k = 0; k < MAX_NUM_PAGE; k++)
    {
        *pte_addr = pte;
        pte += SIZE_4KB;
        pte_addr++;
    }
    // set entry as present to store buddy system struct
    num_pte += (MAX_NUM_NODE + 1) / SIZE_4KB;
    while (num_pte >= i)
    {
        page_table.pte[PT_SIZE - i] |= 1;
        i++;
    }
    mem_sys.buddy_sys = (buddy_system_t *)(SIZE_4MB - (SIZE_4KB * num_pte));
    // memset(mem_sys.buddy_sys, 0, (SIZE_4KB * num_pte));
    // init buddy system
    init_buddy_sys(MAX_ORDER_PAGE);
    // init first 8 caches with 16B, 32B, ... ,2KB, last 8 caches unused
    for (i = 0; i < (MAX_NUM_CACHE >> 1); i++)
    {
        cache_init(get_size(i + 4), i);
        cache_init(-1, i + (MAX_NUM_CACHE >> 1));
    }
    return 0;
}

int32_t memory_shrink()
{
    int32_t i;
    for (i = 0; i < MAX_NUM_CACHE; i++)
    {
        if (mem_sys.cache_size_array[i] == -1)
        {
            continue;
        }
        cache_shrink(&mem_sys.cache_array[i]);
    }
    for (i = 0; i < MAX_NUM_CACHE; i++)
    {
        if (mem_sys.cache_size_array[i] == -1)
        {
            continue;
        }
        cache_display(&mem_sys.cache_array[i], i);
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

void *k_alloc(int32_t size, int32_t priv)
{
    void *ret_addr;
    if ((size <= 0) || (size > SIZE_4MB))
    {
        return NULL;
    }
    size = align_size(size, 2); // align to 4
    if (size < SIZE_4KB)
    {
        ret_addr = cache_alloc(size, priv);
        if (ret_addr == NULL)
        {
            ret_addr = bd_alloc(get_order(size / SIZE_4KB), priv);
        }
    }
    else
    {
        ret_addr = bd_alloc(get_order(size / SIZE_4KB), priv);
    }
    return ret_addr;
}

int32_t k_free(void *addr)
{
    // printf("k_free at %x\n", addr);
    int32_t ret = cache_free(addr);
    if (ret == -2)
    {
        return -2;
    }
    if (ret == -1 && bd_free(addr) == -1)
    {
        printf("k_free fail\n");
        return -1;
    }
    printf("k_free succ\n");
    return 0;
}

buddy_system_t *init_buddy_sys(int32_t order)
{
    // garbage check
    if ((order < 1) || (order > MAX_ORDER_PAGE))
    {
        return NULL;
    }
    // set mapping
    int32_t i;
    // init fields
    mem_sys.buddy_sys->max_order = order;
    // init the order fo every node, get_size(1 + order) = num of nodes
    int16_t node_order = order + 1;
    for (i = 0; i < get_size(1 + order) - 1; i++)
    {
        if (is_power_two(i + 1))
        {
            node_order -= 1;
        }
        mem_sys.buddy_sys->node_array[i] = node_order;
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

void *bd_alloc(int8_t order, int32_t priv)
{
    // printf("bd_alloc %d pages\n", get_size(order));
    if (priv < 0 && priv > 1)
    {
        return NULL;
    }
    // size to alloc too large
    if (mem_sys.buddy_sys->node_array[0] < order)
    {
        return NULL;
    }
    // search down recursively to find the node to alloc
    int8_t node_order; // the expected order of node, the max order of every layer.
    int32_t index = 0; // node index
    for (node_order = mem_sys.buddy_sys->max_order; node_order != order; node_order -= 1)
    {
        index = choose_child(index, order);
    }
    // update node and get offset of addr
    mem_sys.buddy_sys->node_array[index] = -1;
    int32_t offset = (index + 1) * get_size(order) - get_size(mem_sys.buddy_sys->max_order);
    // update node order recursively up
    while (index)
    {
        index = parent(index);
        mem_sys.buddy_sys->node_array[index] = max(mem_sys.buddy_sys->node_array[left_child(index)], mem_sys.buddy_sys->node_array[right_child(index)]);
    }
    // todo: set alloced page to present*******************************
    int32_t i;
    PTE_4KB_t *pte_addr = ((PTE_4KB_t *)mem_sys.pt_for_bd) + offset + get_size(order) - 1;
    for (i = get_size(order) - 1; i >= 0; i--)
    {
        pte_addr->p = 1;
        pte_addr->u_s = priv;
        pte_addr->r_w = 1;
        // printf("pte: %#x    ",*pte_addr);
        // printf("pte_addr: %#x\n",pte_addr);
        pte_addr--;
    }
    // return addr
    // printf("bd_alloc at %#x, priv: %d\n", (offset * SIZE_4KB + BASE_ADDR_BD_SYS), priv);
    return (void *)(offset * SIZE_4KB + BASE_ADDR_BD_SYS);
}

int32_t bd_free(void *addr)
{
    int32_t offset = ((int32_t)addr - BASE_ADDR_BD_SYS) >> 12;
    // garbage check
    if ((offset < 0) || (offset > get_size(mem_sys.buddy_sys->max_order)))
    {
        return -1;
    }
    // init to bottom node order and node index
    int8_t node_order = 0;
    int32_t index = offset + get_size(mem_sys.buddy_sys->max_order) - 1;
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
    int32_t i;
    PTE_4KB_t *pte_addr = ((PTE_4KB_t *)mem_sys.pt_for_bd) + offset + get_size(node_order) - 1;
    for (i = get_size(node_order) - 1; i >= 0; i--)
    {
        pte_addr->p = 0;
        pte_addr--;
    }
    // update node order recursively up
    int8_t left_order;
    int8_t right_order;
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
    int32_t offset = ((int32_t)addr - BASE_ADDR_BD_SYS) >> 12;
    // garbage check
    if ((offset < 0) || (offset > get_size(mem_sys.buddy_sys->max_order)))
    {
        return -1;
    }
    int32_t index = offset + get_size(mem_sys.buddy_sys->max_order) - 1;
    int8_t node_order = 0;
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
    int32_t offset = ((int32_t)addr - BASE_ADDR_BD_SYS) >> 12;
    // garbage check
    if ((offset < 0) || (offset > get_size(mem_sys.buddy_sys->max_order)))
    {
        return NULL;
    }
    int32_t index = offset + get_size(mem_sys.buddy_sys->max_order) - 1;
    int8_t node_order = 0;
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
    return (void *)(offset * SIZE_4KB + BASE_ADDR_BD_SYS);
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
    return (int32_t)(slab + 1) - (int32_t)slab;
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
        new_slab->free_id_list[i] = (uint8_t)(i + 1);
    }
    // new_slab->free_id_list[num_obj] = (int8_t)MAX_NUM_OBJ; // list end
    return 0;
}

mem_cache_t *cache_init(int32_t obj_size, int32_t index)
{
    // garbage check
    if (index < 0 || index > MAX_NUM_CACHE)
    {
        return NULL;
    }
    mem_cache_t *cache = &mem_sys.cache_array[index];
    // init as invaild
    if (obj_size <= 0)
    {
        cache->slabs_head = NULL;
        cache->slab_type = -1;
        cache->page_order = -1;
        cache->obj_size = -1;
        cache->num_per_slab = -1;
        cache->total_num = -1;
        cache->free_num = -1;
        cache->cache_id = -1;
        mem_sys.cache_size_array[index] = -1;
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
        off_slab_index = find_unuse_off_slab();
        if (off_slab_index == -1)
        {
            return NULL;
        }
        new_slab = &mem_sys.off_slab_array[off_slab_index];
        slab_init(new_slab, NULL, cache, num_obj, obj_size, bd_alloc(curr_order, 1), off_slab_index);
        mem_sys.num_off_slab++;
    }
    else
    { // on slab
        new_slab = (slab_t *)bd_alloc(curr_order, 1);
        slab_init(new_slab, NULL, cache, num_obj, obj_size, (void *)(new_slab + 1), -1);
    }
    // init cache
    cache->slabs_head = new_slab;
    cache->slab_type = slab_type;
    cache->page_order = curr_order;
    cache->obj_size = obj_size;
    cache->num_per_slab = num_obj;
    cache->total_num = num_obj;
    cache->free_num = num_obj;
    cache->cache_id = index;
    mem_sys.cache_size_array[index] = obj_size;
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
    slab->free_id_list[slab->next_free_index] = MAX_NUM_OBJ; // mark as used
    slab->free_num--;
    slab->cache->free_num--;
    return (void *)ret_addr;
};

// size < SIZE_4KB
void *cache_alloc(int32_t size, int32_t priv)
{
    int32_t i;
    mem_cache_t *cache;
    int32_t good_size = SIZE_4KB;  // a good existing size which > given size
    int32_t good_index;            // the id of cache with good_size
    int32_t curr_size;             // curr size in loop
    int32_t first_free_index = -1; // the first free cache id, used if create new cache
    for (i = 0; i < MAX_NUM_CACHE; i++)
    {
        curr_size = mem_sys.cache_size_array[i];
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
            cache = cache_init(size, first_free_index);
            good_slab = cache->slabs_head;
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
        good_slab = mem_sys.cache_array[good_index].slabs_head;
        cache = &mem_sys.cache_array[good_index];
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
            good_slab = cache_grow(cache, prev_slab);
        }
    }
    void *ret_val = slab_get_obj(good_slab);
    return ret_val;
}

int32_t slab_put_obj(slab_t *slab, void *obj)
{
    int32_t index = (int32_t)((uint8_t *)obj - (uint8_t *)slab->obj_start) / slab->obj_size;
    // have free
    if (slab->free_id_list[index] != MAX_NUM_OBJ)
    {
        return -1;
    }
    slab->free_id_list[index] = slab->next_free_index;
    slab->next_free_index = index;
    slab->free_num++;
    slab->cache->free_num++;
    return 0;
}

int32_t cache_free(void *addr)
{
    slab_t *slab;
    // get the start addr of pages
    void *start = bd_get_start(addr);
    if (start == NULL)
    {
        return -1;
    }
    int32_t i = 0;
    // check if obj belongs to the off slab, can find obj_start = start if off slab
    while ((i < MAX_NUM_OFF_SLAB) && (mem_sys.off_slab_array[i].obj_start != start))
    {
        i++;
    }
    if (i == MAX_NUM_OFF_SLAB)
    { // in on slab, start is slab head
        slab = (slab_t *)start;
        // check if a vaild slab, if not, addr may not alloced by slab
        if (((slab_t *)slab->obj_start - 1) != (void *)start)
        {
            return -1;
        }
    }
    else
    { // in off slab
        slab = &mem_sys.off_slab_array[i];
    }
    int32_t ret_val = slab_put_obj(slab, addr);
    return ret_val;
}

int32_t cache_shrink(mem_cache_t *cache)
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
                mem_sys.num_off_slab--;
            }
            // for on slab, will be free directly since curr_slab==obj_start
            bd_free((void *)rm_slab->obj_start);
        }
        else
        {
            prev_slab = curr_slab;
            curr_slab = curr_slab->next;
        }
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
        slab_init(new_slab, NULL, cache, cache->num_per_slab, cache->obj_size, bd_alloc(cache->page_order, 1), off_slab_index);
        mem_sys.num_off_slab++;
    }
    else
    { // on slab
        new_slab = (slab_t *)bd_alloc(cache->page_order, 1);
        slab_init(new_slab, NULL, cache, cache->num_per_slab, cache->obj_size, (void *)(new_slab + 1), -1);
    }
    if (end_slab == NULL)
    {
        cache->slabs_head = new_slab;
    }
    else
    {
        end_slab->next = new_slab;
    }
    cache->free_num += end_slab->total_num;
    cache->total_num += end_slab->total_num;
    return new_slab;
};

int32_t slab_display(slab_t *slab, int32_t index)
{
    // printf("----[slab %#x] ", slab);
    if (slab->off_slab_index >= 0)
    {
        printf("OFF id %d |", slab->off_slab_index);
    }
    else
    {
        printf("ON type |");
    }
    printf("obj size %d |", slab->obj_size);
    printf("# free %d/%d |", slab->free_num, slab->total_num);
    printf("next free id %d |", slab->next_free_index);

    // printf("cache %#x |", slab->cache);
    printf("\n        ");
    int32_t obj_index = 0;
    for (obj_index = 0; obj_index < slab->total_num; obj_index++)
    {
        if (slab->free_id_list[obj_index] == MAX_NUM_OBJ)
        {
            printf("x");
        }
        else
        {
            // printf("_%d",slab->free_id_list[obj_index]);
            printf("_");
        }
    }
    printf("\n");
    return 0;
}

int32_t cache_display(mem_cache_t *cache, int32_t index)
{
    if (index < 0)
    {
        printf("[cache] ");
    }
    else
    {
        printf("[cache %d] ", index);
    }
    if (cache->slab_type == 0)
    {
        printf("[ON] |");
    }
    else
    {
        printf("[OFF] |");
    }
    printf("slab size %d |", SIZE_4KB * get_size(cache->page_order));
    printf("obj size %d |", cache->obj_size);
    // printf("# obj %d |", cache->total_num);
    printf("# free %d/%d |", cache->free_num, cache->total_num);
    // printf("head %#x |", cache->slabs_head);
    printf("\n        ");
    slab_t *curr_slab = cache->slabs_head;
    while (curr_slab != NULL)
    {
        if (curr_slab->free_num == 0)
        {
            printf("x");
        }
        else if (curr_slab->free_num == curr_slab->total_num)
        {
            printf("_");
        }
        else
        {
            printf("-");
        }
        curr_slab = curr_slab->next;
    }
    printf("\n");
    curr_slab = cache->slabs_head;
    int32_t slab_index = 0;
    while (curr_slab != NULL)
    {
        slab_display(curr_slab, slab_index);
        curr_slab = curr_slab->next;
    }
    return 0;
}