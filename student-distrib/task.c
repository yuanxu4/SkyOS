/*
 * task.c
 *
 * Description:
 * sourse code of task
 *
 * ECE 391 SP 2022
 * History:
 * create file              - Mar 31, keyi
 */

#include "task.h"
#include "lib.h"

static page_usage_array_t page_array; // manage pages

extern void flush_TLB();   // defined in boot.S
extern PCB_t *curr_task(); // defined in boot.S

int32_t print_pcb(PCB_t *task)
{
    printf("task_name:%s\n", task->task_name);
    printf("pid:%d\n", task->pid);
    printf("parent:%x\n", task->parent);
    printf("saved_esp:%x\n", task->saved_esp);
    printf("saved_ebp:%x\n", task->saved_ebp);
    printf("kernel_ebp:%x\n", task->kernel_ebp);
    printf("eip:%x\n", task->eip);
}

/*
 * int32_t init_task_page_array()
 * init page_array
 * Inputs: none
 * Outputs: None
 * Side Effects: init page_array
 * return value: 1 for succ
 */
int32_t init_task_page_array()
{
    page_array.num_using = 0;
    int i;
    for (i = 0; i < MAX_NUM_TASK; i++)
    {
        page_array.pages[i] = NOT_IN_USE;
    }
    return 1;
}

/*
 * int32_t find_unused_page()
 * help function to find an unused page id
 * Inputs:  none
 * Outputs: None
 * Side Effects: none
 * return value: an unused file desc or -1 for failure
 */
int32_t find_unused_page()
{
    int id;
    for (id = 0; id < MAX_NUM_TASK; id++)
    {
        if (!page_array.pages[id])
        {
            return id;
        }
    }
    return -1;
}

/*
 * int32_t set_task_page()
 * init page_array
 * Inputs: none
 * Outputs: None
 * Side Effects: init page_array
 * return value: -1 for fail, page_id for succ
 */
int32_t set_task_page()
{
    int32_t page_id = find_unused_page(); // assign an unused page
    uint32_t base_addr;                   // physical base addr

    if (page_id == -1)
    {
        return -1;
    }
    page_array.pages[page_id] = IN_USE;
    // set pde
    base_addr = KERNEL_UPPER_ADDR + page_id * SIZE_4MB;
    page_directory.pde[TASK_VIR_IDX] = base_addr | TASK_PAGE_INFO;
    flush_TLB();            // flush tlb since remapping
    page_array.num_using++; // incr # using
    return page_id;
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
    uint8_t *args;
    while (*command != '\0')
    {
        if (*command == ' ')
        {
            *command = '\0';    // terminate cmd, only store the exe file name
            args = command + 1; // output args string
            return args;
        }
        command++;
    }
    return NULL;
}

/*
 * PCB_t *get_task_ptr(int32_t id)
 * get a pointer to task
 * Inputs:  id - the pid of task
 * Outputs: None
 * Side Effects:
 * return value: the pointer to the new task, NULL for failure
 */
PCB_t *get_task_ptr(int32_t id)
{
    if ((id >= 0) & (id < MAX_NUM_TASK))
    {
        return (PCB_t *)(KERNEL_UPPER_ADDR - (id + 1) * SIZE_8KB);
    }
    return NULL;
}

/*
 * PCB_t *create_task(uint8_t *name, uint8_t *args)
 * create a new task
 * Inputs:  name -- the name of the task
 *          args -- pointer to arguments
 * Outputs: None
 * Side Effects:
 * return value: the pointer to the new task, NULL for failure
 */
PCB_t *create_task(uint8_t *name, uint8_t *args)
{
    int32_t page_id; // new page id
    PCB_t *new_task; // new task
    // set am unused page
    page_id = set_task_page();
    if (page_id == -1)
    {
        return NULL;
    }
    // get the pointer to new task
    new_task = get_task_ptr(page_id);
    if (new_task == NULL)
    {
        return NULL;
    }
    // set up task
    new_task->pid = page_id;
    new_task->state = IN_USE;
    new_task->kernel_ebp = ((uint32_t)new_task) + SIZE_8KB - 4; // -4 for safty
    // new_task->kernel_esp = new_task->kernel_ebp;
    new_task->args = args;
    init_file_array(&new_task->fd_array);
    new_task->task_name = name;
    // if the only one task(shell)
    if (page_array.num_using == 1)
    {
        new_task->parent = NULL;
    }
    else
    {
        new_task->parent = curr_task();
    }
    return new_task;
}

/*

todo
    Context Switch

*/
int32_t system_execute(const uint8_t *command)
{
    dentry_t task_dentry; // copied task dentry
    uint8_t *args;        // arguments
    // int32_t page_id;      // new page id
    PCB_t *new_task; // new task
    uint32_t eip;
    // uint32_t len; // tem len
    // Parse args
    args = parse_args((uint8_t *)command);
    // get the dentry in fs

    if (read_dentry_by_name(command, &task_dentry) == -1)
    {
        return -1;
    }
    // Check for executable
    if (!is_exe_file(&task_dentry))
    {
        return -1;
    }
    // Create PCB, Set up paging
    new_task = create_task((uint8_t *)command, args);
    if (new_task == NULL)
    {
        return -1;
    }

    // Losd file into memory
    file_load(&task_dentry, (uint8_t *)TASK_VIR_ADDR + TASK_VIR_OFFSET);
    // todo: Context Switch
    eip = get_eip(&task_dentry);
    new_task->eip = eip;
    // save esp ebp
    asm volatile("          \n\
        movl %%ebp, %%eax   \n\
        movl %%esp, %%ebx   \n\
        "
                 : "=a"(new_task->saved_ebp), "=b"(new_task->saved_esp));
    // set tss
    tss.ss0 = KERNEL_DS;
    tss.esp0 = new_task->kernel_ebp;
    // print_pcb(new_task);
    // ...
    asm volatile("            \n\
        movw    %%ax, %%ds  /* set ds to user ds */     \n\
        pushl   %%eax   /* 1 iret USER_DS */          \n\
        pushl   %%ebx   /* 2 iret esp */         \n\
        pushfl                 \n\
        popl    %%ebx          \n\
        orl     $0x0200, %%ebx \n\
        pushl   %%ebx   /* 3 set saved IF for restoring*/          \n\
        pushl   %%ecx   /* 4 iret CS*/        \n\
        pushl   %%edx   /* 5 iret eip*/         \n\
        iret   /* go to user stack*/        \n\
        EXE_RET: /* after halt*/ \n\
        leave \n\
        ret \n\
        "
                 :
                 : "a"(USER_DS), "b"(USER_EBP), "c"(USER_CS), "d"(eip)
                 : "memory");
    return 0;
}

/*
 * int32_t deactivate_task_page(int32_t page_id)
 * deactivate a task page
 * Inputs: page_id -- The id of task page
 * Outputs: None
 * Side Effects: deactivate a task page
 * return value: 0 when succuss
 */
int32_t deactivate_task_page(int32_t page_id)
{
    // not in use
    if (!page_array.pages[page_id])
    {
        return -1;
    }
    // used to not, decr
    page_array.pages[page_id] = NOT_IN_USE;
    page_array.num_using--;
    return 0;
}

/*
 * PCB_t *deactivate_task(PCB_t *task)
 * deactivate a task
 * Inputs: task -- The pointer to task
 * Outputs: None
 * Side Effects: deactivate a task
 * return value: NULL when failure
 */
PCB_t *deactivate_task(PCB_t *task)
{
    // never shutdown shell
    if (task == NULL)
    {
        return NULL;
    }
    // not running
    if (!task->state)
    {
        return NULL;
    }
    // deactivate file array, task page
    task->state = NOT_IN_USE;
    deactivate_file_array(&task->fd_array);
    deactivate_task_page(task->pid);
    return task->parent;
}

/*
 * int32_t restore_task_page(int32_t page_id)
 * restore a task page
 * Inputs: page_id -- The id of task page
 * Outputs: None
 * Side Effects: restore a task page
 * return value: 0 when succuss
 */
int32_t restore_task_page(int32_t page_id)
{
    if (!page_array.pages[page_id])
    {
        return -1;
    }
    uint32_t base_addr; // physical base addr
    // set pde
    base_addr = KERNEL_UPPER_ADDR + page_id * SIZE_4MB;
    page_directory.pde[TASK_VIR_IDX] = base_addr | TASK_PAGE_INFO;
    flush_TLB(); // flush tlb since remapping
    return 0;
}

/*

todo
    Restore parent data
    Restore parent paging
    Close any relevant FDs
    Jump to execute return

*/
int32_t system_halt(uint8_t status)
{
    PCB_t *parent;
    if (page_array.num_using == 0)
    {
        printf("halt nothing");
        return -1;
    }
    // try to deactivate task, get parent task
    parent = deactivate_task(curr_task());
    if (parent == NULL)
    {
        system_execute("shell");
    }
    // Restore parent paging
    restore_task_page(parent->pid);

    // todo
    // Write Parent process’ info back to TSS
    tss.esp0 = curr_task()->saved_esp;
    // print_pcb(curr_task());
    // print_pcb(parent);
    // restore stack
    asm volatile("         \n\
            movl %%ebx, %%esp   \n\
            movl %%ecx, %%ebp   \n\
            movl %%edx, %%eax   \n\
            jmp EXE_RET  /* jump to exe ret*/             \n\
            "
                 : /* no output*/
                 : "b"(curr_task()->saved_esp), "c"(curr_task()->saved_ebp), "d"(status)
                 : "eax");
    // should never return
    printf("ERROR, reach end of halt()");
    return 0;
}
