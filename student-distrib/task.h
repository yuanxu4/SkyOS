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

#define MAX_ARGS 128 // pending

typedef struct PCB PCB_t;
struct PCB
{
    // pending

    uint8_t pid;   // unique identifier of the process
    PCB_t *parent; // parent process
    int32_t state; // the state of the process
    file_array_t fd_array;
    uint32_t saved_esp;
    uint32_t saved_ebp;
    // uint32_t flags;
    // uint8_t *process_name; // process executable name
    uint8_t args[MAX_ARGS]; // arguments of process
};

#endif // _TASK_H
