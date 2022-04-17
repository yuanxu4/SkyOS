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

/*
 * int32_t one_bit_check(uint8_t flag)
 * check if the flag's bit 1-7 are 0s
 * Inputs:  flag - the input flag
 * Outputs: None
 * Side Effects: None
 * return value: 1 if flag is 0/1 else 0
 */
int32_t one_bit_check(uint8_t flag)
{
    return !(flag >> 1);
}

/*
 * int32_t set_PDE_4KB(PDE_4KB_t *pde, uint32_t pt, uint8_t can_write, uint8_t user, uint8_t present)
 * set a 4KB PDE
 * Inputs:  pde - the addr of pde to set
 *          pt - the addr of pt, bit 20-31 in pde
 *          can_write - r/w flag, bit 1
 *          user - u/s flag, bit 2
 *          present - p flag, bit 0
 * Outputs: None
 * Side Effects: None
 * return value: 0 if succ, -1 else
 */
int32_t set_PDE_4KB(PDE_4KB_t *pde, uint32_t pt, uint8_t can_write, uint8_t user, uint8_t present)
{
    if ((pde == NULL) || (pt == 0) || (pt & 0x00000FFF))
    {
        printf("set_PDE_4KB: Invaild pointer\n");
        return -1;
    }
    if ((can_write >> 1) || (user >> 1) || (present >> 1))
    {
        printf("set_PDE_4KB: Invaild flag\n");
        return -1;
    }
    *(uint32_t *)pde = (pt & 0xFFFFF000) | (can_write << 1) | (user << 2) | (present);
    return 0;
}

/*
 * int32_t set_PTE_4KB(PTE_4KB_t *pte, uint32_t page, uint8_t can_write, uint8_t user, uint8_t present)
 * set a 4KB PTE
 * Inputs:  pte - the addr of pte to set
 *          page - the addr of page, bit 20-31 in pte
 *          can_write - r/w flag, bit 1
 *          user - u/s flag, bit 2
 *          present - p flag, bit 0
 * Outputs: None
 * Side Effects: None
 * return value: 0 if succ, -1 else
 */
int32_t set_PTE_4KB(PTE_4KB_t *pte, uint32_t page, uint8_t can_write, uint8_t user, uint8_t present)
{
    if ((pte == NULL) || (page == 0) || (page & 0x00000FFF))
    {
        printf("set_PTE_4KB: Invaild pointer\n");
        return -1;
    }
    if ((can_write >> 1) || (user >> 1) || (present >> 1))
    {
        printf("set_PTE_4KB: Invaild flag\n");
        return -1;
    }
    *(uint32_t *)pte = (page & 0xFFFFF000) | (can_write << 1) | (user << 2) | (present);
    return 0;
}

/*
 * int32_t clear_PDE_4MB(PDE_4MB_t *pde)
 * clear a 4MB PDE
 * Inputs:  pde - the addr of pde to clear
 * Outputs: None
 * Side Effects: None
 * return value: 0 if succ, -1 else
 */
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

/*
 * int32_t clear_PDE_4KB(PDE_4KB_t *pde)
 * clear a 4KB PDE
 * Inputs:  pde - the addr of pde to clear
 * Outputs: None
 * Side Effects: None
 * return value: 0 if succ, -1 else
 */
int32_t clear_PDE_4KB(PDE_4KB_t *pde)
{
    if (pde == NULL)
    {
        printf("clear_PDE_4KB: NULL\n");
        return -1;
    }
    *(uint32_t *)pde = 0x00000000; // bit 7 (Page size) is 0 since 4KB
    return 0;
}

/*
 * int32_t clear_PTE_4KB(PTE_4KB_t *pte)
 * clear a 4KB PTE
 * Inputs:  pte - the addr of pte to clear
 * Outputs: None
 * Side Effects: None
 * return value: 0 if succ, -1 else
 */
int32_t clear_PTE_4KB(PTE_4KB_t *pte)
{
    if (pte == NULL)
    {
        printf("clear_PDE_4KB: NULL\n");
        return -1;
    }
    *(uint32_t *)pte = 0x00000000;
    return 0;
}
