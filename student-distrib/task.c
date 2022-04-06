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

extern void FLUSH_TLB();   // defined in boot.S
extern PCB_t *curr_task(); // defined in boot.S

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
 * return value: -1 for fail, 0 for succ
 */
int32_t set_task_page()
{
    int32_t page_id = find_unused_page(); // assign an unused page
    uint32_t base_addr;                   // physical base addr

    if (page_id == -1)
    {
        return -1;
    }
    // set pde
    base_addr = KERNEL_UPPER_ADDR + page_id * SIZE_4MB;
    page_directory.pde[TASK_VIR_IDX] = base_addr | TASK_PAGE_INFO;
    FLUSH_TLB();            // flush tlb since remapping
    page_array.num_using++; // incr # using
    return 0;
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
        return (PCB_t *)KERNEL_UPPER_ADDR - (id + 1) * SIZE_8KB;
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
    int32_t page_id;
    PCB_t *task_ptr;
    page_id = set_task_page();
    if (page_id == -1)
    {
        return NULL;
    }
    task_ptr = get_task_ptr(page_id);
    if (task_ptr == NULL)
    {
        return NULL;
    }
    task_ptr->pid = page_id;
    task_ptr->state = IN_USE;
    task_ptr->kernel_ebp = ((uint32_t)task_ptr) + SIZE_8KB - 1;
    task_ptr->kernel_esp = task_ptr->kernel_ebp;
    task_ptr->args = args;
    init_file_array(&task_ptr->fd_array);
    task_ptr->task_name = name;
    // if the only one task(shell)
    if (page_array.num_using == 1)
    {
        task_ptr->parent = NULL;
    }
    else
    {
        task_ptr->parent = curr_task();
    }
    return task_ptr;
}

/*

todo
    Context Switch

*/
int32_t system_execute(const uint8_t *command)
{
    dentry_t task_dentry;
    uint8_t *args;
    int32_t page_id;
    PCB_t *new_task;
    uint32_t len;

    args = parse_args((uint8_t *)command);

    if (read_dentry_by_name(command, &task_dentry) == -1)
    {
        return -1;
    }
    if (!is_exe_file(&task_dentry))
    {
        return -1;
    }
    new_task = create_task((uint8_t *)command, args);
    if (new_task == NULL)
    {
        return -1;
    }
    len = strlen((int8_t *)new_task->task_name);
    new_task->kernel_esp -= len;
    new_task->task_name = (uint8_t *)strcpy((int8_t *)new_task->kernel_esp, (int8_t *)new_task->task_name);
    if (new_task->args != NULL)
    {
        len = strlen((int8_t *)new_task->args);
        new_task->kernel_esp -= len;
        new_task->args = (uint8_t *)strcpy((int8_t *)new_task->kernel_esp, (int8_t *)new_task->args);
        file_load(&task_dentry, (uint8_t *)TASK_VIR_ADDR + TASK_VIR_OFFSET);
    }
    // todo: Context Switch
    tss.ss0 = KERNEL_DS;
    tss.esp0 = new_task->kernel_esp;
    // ...

    return 0;
}

int32_t deactivate_task_page(int32_t page_id)
{
    if (!page_array.pages[page_id])
    {
        return -1;
    }
    page_array.pages[page_id] = NOT_IN_USE;
    page_array.num_using--;
    return 0;
}

PCB_t *deactivate_task(PCB_t *task)
{
    task->state = NOT_IN_USE;
    deactivate_file_array(&task->fd_array);
    deactivate_task_page(task->pid);
    return task->parent;
}

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
    FLUSH_TLB(); // flush tlb since remapping
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
    // never halt shell
    if (curr_task()->parent == NULL)
    {
        return -1;
    }
    parent = deactivate_task(curr_task());
    restore_task_page(parent->pid);

    // todo
    tss.esp0 = parent->kernel_esp;
    // ...

    return 0;
}
