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
#include "paging.h"
#include "vidmem.h"
#include "pit.h"
#include "keyboard.h"

page_usage_array_t page_array; // manage pages
static run_queue_t* run_queue_head;
static uint32_t num_task_in_queue;

extern void flush_TLB();   // defined in boot.S
extern PCB_t *curr_task(); // defined in boot.
extern uint32_t cur_terminal_id;
extern terminal_t *curr_terminal;
// extern PT_t kernel_pt;

int32_t add_task_to_run_queue(PCB_t *new_task);
int32_t remove_task_from_run_queue(PCB_t *new_task);
// extern  terminal_t *curr_terminal;

// int32_t print_pcb(PCB_t *task)
// {
//     printf("task_name:%s\n", task->task_name);
//     printf("pid:%d\n", task->pid);
//     printf("parent:%x\n", task->parent);
//     printf("saved_esp:%x\n", task->saved_esp);
//     printf("saved_ebp:%x\n", task->saved_ebp);
//     printf("esp:%x\n", task->esp);
//     printf("ebp:%x\n", task->ebp);
//     printf("kernel_ebp:%x\n", task->kernel_ebp);
//     printf("eip:%x\n", task->eip);
// }

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
    // printf("%s\n", command);
    uint8_t *args = NULL;
    while (*command != '\0')
    {
        if (*command == ' ')
        {
            *command = '\0';    // terminate cmd, only store the exe file name
            args = command + 1; // output args string
            break;
        }
        command++;
    }
    return args;
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
    new_task->args = args;
    new_task->task_name = name;
    new_task->vidmap = 0;

    init_file_array(&new_task->fd_array);
    // if the only one task(shell)
    // if (page_array.num_using == 1)
    // {
    //     new_task->parent = NULL;
    // }
    // else
    // {
    //     new_task->parent = curr_task();
    // }
    curr_terminal->num_task++;
    if (curr_terminal->num_task == 1)
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
 * int32_t system_execute(const uint8_t *command)
 * try execute a file
 * Inputs:  command -- including the name of the task and arguments
 * Outputs: None
 * Side Effects:
 * return value: -1 if the command cannot be executed, 256 if the program dies by an exception,
 * or a value in the range 0 to 255 if the program executes a halt system call
 */
int32_t system_execute(const uint8_t *command)
{
    
    cli();
    
    dentry_t task_dentry; // copied task dentry
    uint8_t *args;        // arguments
    // int32_t page_id;      // new page id
    PCB_t *new_task; // new task
    uint32_t eip;
    uint8_t args_array[MAX_ARGS + 1];
    uint8_t name_array[MAX_LEN_FILE_NAME + 1];
    memset((void*)args_array, (int32_t)"\0", MAX_ARGS + 1);
    memset((void*)name_array, (int32_t)"\0", MAX_LEN_FILE_NAME + 1);
    // uint32_t len; // tem len
    // Parse args
    args = parse_args((uint8_t *)command);
    // printf("%s\n", command);
    strcpy((int8_t *)name_array, (const int8_t *)command);
    if (args != NULL )
    {
        // printf("%s\n", args);
        if (*args != '\0')
        {
            strcpy((int8_t *)args_array, (const int8_t *)args);
        }
    }

    
    // get the dentry in fs
    if (read_dentry_by_name(command, &task_dentry) == -1)
    {
        printf("[INFO] Cannot execute non-exist file\n");
        return -1;
    }
    // Check for executable
    if (!is_exe_file(&task_dentry))
    {
        printf("[INFO] Cannot execute unexecutable file\n");
        return -1;
    }
    // Create PCB, Set up paging
    new_task = create_task(name_array, args_array);
    if (new_task == NULL)
    {
        printf("[INFO] Cannot execute more than %d programs!\n", MAX_NUM_TASK);
        return -1;
    }

    
    /* new_task->parent = current_task */
    // curr_terminal->num_task++;
    new_task->terminal = curr_terminal;
    if (page_array.num_using == 1)
    {
        ONTO_TERMINAL(printf_sche("terminal<1>\n"));
    }
    /* add to schedule run_queue */
    if (new_task->parent == NULL)
    {
        add_task_to_run_queue(new_task); 
    }
    else
    {
        remove_task_from_run_queue(new_task->parent);
        add_task_to_run_queue(new_task);
    }
    
    video_mem_map_task(new_task);
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



    // prepare for iret
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
 * int32_t system_halt(uint8_t status)
 * halt current task
 * Inputs:  status -- status returned to execute
 * Outputs: None
 * Side Effects:
 * return value: returning the specified value to its parent process
 */
int32_t system_halt(uint8_t status)
{
    cli();
    PCB_t *parent;
    if (page_array.num_using == 0||(curr_terminal->num_task == 0))
    {
        printf("[INFO] nothing to halt\n");
        // system_execute((uint8_t *)"shell");
        return -1;
    }
    if(curr_task()->vidmap == 1) {
        PDE_4KB_t *user_vid_pde = (PDE_4KB_t *)(&page_directory.pde[VID_PAGE_INDEX]);
        clear_PDE_4KB(user_vid_pde);
    }

    /* remove current task from run_queue */
    remove_task_from_run_queue(curr_task());
    curr_task()->terminal->num_task--;
    // try to deactivate task, get parent task
    parent = deactivate_task(curr_task());
    if (parent == NULL)
    {
        /* system_execute has contained add to queue */        
        system_execute((uint8_t *)"shell");
    }
    else
    {
        add_task_to_run_queue(parent);
    }

    // Restore parent paging
    restore_task_page(parent->pid);
    // Write Parent process’ info back to TSS
    tss.esp0 = curr_task()->saved_esp;
    video_mem_map_task(parent);
    // restore stack and return valueo
    // sti();
    asm volatile("         \n\
            movl %%edx, %%esp   \n\
            movl %%ecx, %%ebp   \n\
            andl $0, %%eax    \n\
            movb %%bl, %%al   \n\
            jmp EXE_RET  /* jump to exe ret*/             \n\
            "
                 : /* no output*/
                 : "d"(curr_task()->saved_esp), "c"(curr_task()->saved_ebp), "b"(status)
                 : "eax");
    // should never return
    printf("ERROR, reach end of halt()");
    return 0;
}

/*
 * int32_t system_getargs(uint8_t *buf, int32_t nbytes)
 * reads the program’s command line arguments into a user-level buffer.
 * Inputs:  buf -- user-level buffer
 *          nbytes -- length to copy
 * Outputs: None
 * Side Effects:
 * return value: 0 for succ, -1 for failure
 */
int32_t system_getargs(uint8_t *buf, int32_t nbytes)
{
    uint8_t *args = curr_task()->args;
    if (args == NULL)
    {
        return -1;
    }
    if (strlen((int8_t *)args) >= nbytes)
    {
        return -1;
    }
    // printf("%s\n", args);
    // printf("%x\n", args);
    strncpy((int8_t *)buf, (int8_t *)args, nbytes);
    return 0;
}

/*############################### SCHEDULE for CP5 ##########################*/
/*
 * int32_t sche_init()
 * reads the program’s command line arguments into a user-level buffer.
 * Inputs:  buf -- user-level buffer
 *          nbytes -- length to copy
 * Outputs: None
 * Side Effects:
 * return value: 0 for succ, -1 for failure
 */
int32_t sche_init()
{
    /* init run_queue */
    run_queue_head = NULL;
    // pit_init();
    return 0;
        
}

/*
 * int32_t add_task_to_run_queue(PCB_t *new_task)
 * add task to the head of queue.
 * Inputs:  new_task -- next active task
 * Outputs: None
 * Side Effects:
 * return value: 0 for succ, -1 for failure
 */

int32_t add_task_to_run_queue(PCB_t *new_task)
{
    /* if no task in the queue */
    if (run_queue_head == NULL)
    {
        run_queue_head = &(new_task->run_list_node);
        new_task->run_list_node.next = &(new_task->run_list_node);
        new_task->run_list_node.pre = &(new_task->run_list_node);

    }
    else
    {        
        /* add to the head of running list */          
        run_queue_t* last_node = run_queue_head->pre;
        run_queue_head->pre =  &(new_task->run_list_node);
        last_node->next = &(new_task->run_list_node);
        new_task->run_list_node.next = run_queue_head;
        new_task->run_list_node.pre = last_node;
        run_queue_head = &(new_task->run_list_node);          
    }
    num_task_in_queue++; 
    return 0;
}

/*
 * int32_t remove_task_from_run_queue(PCB_t *new_task)
 * delete task from the queue.
 * Inputs:  new_task -- next active task
 * Outputs: None
 * Side Effects:
 * return value: 0 for succ, -1 for failure
 */
int32_t remove_task_from_run_queue(PCB_t *new_task)
{
    if (run_queue_head == NULL)
    {
        printf("[INFO]ERROR: NO task can be removed \n");
    }
    else if (num_task_in_queue == 1)     //only 1 in list
    {
        run_queue_head = NULL;        
    }
    else
    {    
        (new_task->run_list_node.pre)->next = new_task->run_list_node.next;
        (new_task->run_list_node.next)->pre = new_task->run_list_node.pre;
        /* if old_task is the first */
        if (run_queue_head == &(new_task->run_list_node))
        {
            run_queue_head = new_task->run_list_node.next;
        }
    }
    num_task_in_queue--;
    return 0;

}

/*
 * void start_task(void)
 * called in kernel to start scheduling
 * Inputs: None 
 * Outputs: None
 * Side Effects: start first shell
 * return value: 
 */
void start_task()
{
    sche_init();
    terminal_init();
    pit_init();  
    /* execute first task */
    
    system_execute((uint8_t *)"shell"); 
    
}

/*
 * int32_t task_switch(uint8_t status)
 * switch to next program, used for scheduling
 * Inputs:  status -- status returned to execute
 * Outputs: None
 * Side Effects:
 * return value: returning the specified value to its parent process
 */
int32_t task_switch()
{

    /* if there is no task running */
    if (page_array.num_using == 0)
    {
        return 0;
    }
    /* switch terminal and check if task has running */
    
    PCB_t *next_task = (PCB_t*)((uint32_t)(curr_task()->run_list_node.next)&(0xFFFFE000));

    // save esp ebp(in kernel)
    asm volatile("          \n\
        movl %%ebp, %%eax   \n\
        movl %%esp, %%ebx   \n\
        "
                 : "=a"(curr_task()->ebp), "=b"(curr_task()->esp));
                 
    /* everytime switch task then remap */
    video_mem_map_task(next_task);

    set_vidmap();
    if (curr_terminal->num_task == 0)
    {
        ONTO_TERMINAL(printf_sche("terminal<%d>\n",cur_terminal_id));
        system_execute((uint8_t *)"shell");
    }
    
    /* if only one task running */
    if ((curr_task()->run_list_node.next == &(curr_task()->run_list_node))||(num_task_in_queue == 1))
    {
        return 0;
    }
  
    /* set target location*/
    // print_pcb(curr_task());
    uint32_t next_esp = next_task->esp;
    uint32_t next_ebp = next_task->ebp; 
    tss.esp0 = next_task->kernel_ebp;
    asm volatile(
            "               \n\
            movl %%eax, %%esp  \n\
            movl %%ebx, %%ebp  \n\
            "
                :
                 : "a"(next_esp), "b"(next_ebp)
                 : "memory");
                 
    tss.ss0 = KERNEL_DS;      
    
    
    // print_pcb(next_task);

    return 0;
}


/* video_mem_map_task(PCB_t *next_task)
 * description: used for schedule, called in switching task
 * map the virtual video Memory to correct video page
 * map next program ptr 
 * Inputs: PCB_t * next_task -- next task
 * Outputs: 0 success
 * Side Effects: 
 * if now scheduled running terminal is in current terminal -- direct mapping
 * if now scheduled running terminal is not in current terminal -- remapping
 *
 */
int32_t video_mem_map_task(PCB_t *next_task)
{  
    uint32_t base_addr = KERNEL_UPPER_ADDR + next_task->pid * SIZE_4MB;
    page_directory.pde[TASK_VIR_IDX] = base_addr | TASK_PAGE_INFO;
    

    if (next_task->terminal->terminal_id == cur_terminal_id)
    {
        set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX]),VIDEO_MEM_INDEX*SIZE_4KB, 1, 0, 1);
    }
    else
    {
        uint32_t term_id = next_task->terminal->terminal_id;
        set_PTE_4KB((PTE_4KB_t *)(&page_table.pte[VIDEO_MEM_INDEX]),(VIDEO_MEM_INDEX+term_id)*SIZE_4KB, 1, 0, 1);
    }
    flush_TLB();
    return 0;
}
