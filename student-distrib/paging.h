/*
 * paging.h
 *
 * Description:
 * head file of paging
 *
 * ECE 391 SP 2022
 * History:
 * create file              - Mar 19, keyi
 * Temporarily deprecated   - Mar 19, keyi
 *      try to implement init paging with assembly
 */

#ifndef _PAGING_H
#define _PAGING_H

#include "x86_desc.h"

int32_t set_PDE_4kB(PDE_4KB_t *pde, uint32_t pt, uint8_t can_write, uint8_t user, uint8_t present);
int32_t set_PTE_4kB(PTE_4KB_t *pte, uint32_t page, uint8_t can_write, uint8_t user, uint8_t present);

int32_t clear_PDE_4MB(PDE_4MB_t *pde);
int32_t clear_PDE_4kB(PDE_4KB_t *pde);
int32_t clear_PTE_4kB(PTE_4KB_t *pte);

// help func
int32_t one_bit_check(uint8_t flag);

#endif //_PAGING_H
