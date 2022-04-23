/*
 * task.h
 *
 * Description:
 * head file of task
 *
 * ECE 391 SP 2022
 * History:
 * create file              - Mar 31, keyi
 */

#ifndef _TASK_H
#define _TASK_H

#include "types.h"
#include "x86_desc.h"
#include "file_system.h"

#define MAX_ARGS 128                            // pending
#define MAX_NUM_TASK 2                          // max num of running tasks
#define TASK_VIR_ADDR 0x08000000                // base virtual addr for task
#define TASK_VIR_ADDR_END 0x08400000            // base virtual addr for task
#define TASK_VIR_IDX (TASK_VIR_ADDR / SIZE_4MB) // the index of the pde of TASK_VIR_ADDR in pd
#define TASK_VIR_OFFSET 0x00048000              // offset within page of task
#define TASK_PAGE_INFO 0x87                     // flag info of pde for task set PS, U/S, R/W, P flags
#define KERNEL_UPPER_ADDR 0x800000              // 8MB, the upper addr of kernel page
#define SIZE_8KB 0x2000                         // size of 8KB
#define USER_EBP (TASK_VIR_ADDR + SIZE_4MB - 4) // the base addr of user stack, -4 for safty

typedef struct PCB PCB_t;
struct PCB
{
    // pending

    uint8_t pid;   // unique identifier of the process, current = page id
    PCB_t *parent; // parent process
    int32_t state; // the state of the process
    file_array_t fd_array;
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t kernel_ebp; // ebp of this task in kernel
    uint32_t eip;
    uint32_t vidmap; // to check whether the vidmap is create
    // uint32_t kernel_esp; // esp of this task in kernel
    //  uint32_t flags;
    uint8_t *task_name; // process executable name
    uint8_t *args;      // arguments of process
};

typedef struct page_usage_array
{
    int32_t num_using;           // number of using page
    int32_t pages[MAX_NUM_TASK]; //
    // the index is called file descriptor
} page_usage_array_t;

extern page_usage_array_t page_array; // manage pages

int32_t init_task_page_array();
int32_t set_task_page();
uint8_t *parse_args(uint8_t *command);
PCB_t *create_task(uint8_t *name, uint8_t *args);
PCB_t *get_task_ptr(int32_t id);
PCB_t *deactivate_task(PCB_t *task);
int32_t deactivate_task_page(int32_t page_id);
int32_t restore_task_page(int32_t page_id);
// int32_t print_pcb(PCB_t *task);

int32_t system_execute(const uint8_t *command);
int32_t system_halt(uint8_t status);
int32_t system_getargs(uint8_t *buf, int32_t nbytes);

#endif // _TASK_H
