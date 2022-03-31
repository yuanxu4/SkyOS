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

#include "types.h"
#include "x86_desc.h"
#include "file_system.h"

#define MAX_ARGS 128

typedef struct PCB PCB_t;
struct PCB
{
    // pending
    uint8_t *process_name; // process executable name
    uint8_t pid;           // unique identifier for the process
    PCB_t *parent;         // parent process
    int32_t state;         // the state of the process
    uint32_t flags;
    file_array_t file_array;
    uint8_t args[MAX_ARGS]; // arguments of process
};
