/*
 * paging.c
 *
 * Description:
 * sourse code of paging
 *
 * ECE 391 SP 2022
 * History:
 * create file              - Mar 19, keyi
 * Temporarily deprecated   - Mar 19, keyi
 *      try to implement init paging with assembly
 */

#include "paging.h"
#include "lib.h"

int32_t one_bit_check(uint8_t flag)
{
    return !(flag >> 1);
}

int32_t set_PDE_4kB(PDE_4KB_t *pde, uint32_t pt, uint8_t can_write, uint8_t user, uint8_t present)
{
    if ((pde == NULL) || (pt == 0) || (pt & 0x00000FFF))
    {
        printf("set_PDE_4kB: Invaild pointer\n");
        return -1;
    }
    if ((can_write >> 1) || (user >> 1) || (present >> 1))
    {
        printf("set_PDE_4kB: Invaild flag\n");
        return -1;
    }
    *(uint32_t *)pde = (pt & 0xFFFFF000) | (can_write << 1) | (user << 2) | (present);
    return 0;
}

int32_t set_PTE_4kB(PTE_4KB_t *pte, uint32_t page, uint8_t can_write, uint8_t user, uint8_t present)
{
    if ((pte == NULL) || (page == 0) || (page & 0x00000FFF))
    {
        printf("set_PTE_4kB: Invaild pointer\n");
        return -1;
    }
    if ((can_write >> 1) || (user >> 1) || (present >> 1))
    {
        printf("set_PTE_4kB: Invaild flag\n");
        return -1;
    }
    *(uint32_t *)pte = (page & 0xFFFFF000) | (can_write << 1) | (user << 2) | (present);
    return 0;
}

int32_t clear_PDE_4MB(PDE_4MB_t *pde)
{
    if (pde == NULL)
    {
        printf("clear_PDE_4MB: NULL\n");
        return -1;
    }
    *(uint32_t *)pde = 0x00000080; // bit 7 (Page size) is set since 4MB
    return 0;
}

int32_t clear_PDE_4kB(PDE_4KB_t *pde)
{
    if (pde == NULL)
    {
        printf("clear_PDE_4kB: NULL\n");
        return -1;
    }
    *(uint32_t *)pde = 0x00000000; // bit 7 (Page size) is 0 since 4KB
    return 0;
}

int32_t clear_PTE_4kB(PTE_4KB_t *pte)
{
    if (pte == NULL)
    {
        printf("clear_PDE_4kB: NULL\n");
        return -1;
    }
    *(uint32_t *)pte = 0x00000000;
    return 0;
}
