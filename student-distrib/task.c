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

page_usage_array_t page_array; // manage pages

// inode array for pipeline
// char *tem_in_array[3] = {"in1.txt", "in2.txt", "in3.txt"};
char *tem_out_array[3] = {"out1.txt", "out2.txt", "out3.txt"};
// uint32_t tem_in_inode_array[3] = {-1, -1, -1};
uint32_t tem_out_inode_array[3] = {-1, -1, -1};
uint32_t exist_inode_array[3] = {-1, -1, -1};
uint8_t *exist_fname_array[3];

extern void flush_TLB();   // defined in boot.S
extern PCB_t *curr_task(); // defined in boot.S

// int32_t print_pcb(PCB_t *task)
// {
//     printf("task_name:%s\n", task->task_name);
//     printf("pid:%d\n", task->pid);
//     printf("parent:%x\n", task->parent);
//     printf("saved_esp:%x\n", task->saved_esp);
//     printf("saved_ebp:%x\n", task->saved_ebp);
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
uint8_t *parse_args(uint8_t *command, int32_t *type)
{
    // printf("%s\n", command);
    uint8_t *args = NULL;
    *type = 0;
    while (*command != '\0')
    {
        if (*command == '|')
        {
            *type = 5;
            args = command;
            return args;
        }
        if (*command == ' ')
        {
            *command = '\0';    // terminate cmd, only store the exe file name
            args = command + 1; // output args string
            while (*args != '\0')
            {
                if (*args == ' ')
                {
                    args++;
                }
                else if (*args == '>')
                {
                    *type = 1;
                    if (*(args + 1) == '>')
                    {
                        *type = 2;
                    }
                    break;
                }
                else if (*args == '|')
                {
                    *type = 3;
                    break;
                }
                else
                {
                    break;
                }
            }
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

uint8_t *skip_space(uint8_t *tep)
{
    while (*tep != '\0')
    {
        if (*tep == ' ')
        {
            tep++;
        }
        else
        {
            break;
        }
    }
    return tep;
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
PCB_t *create_task(uint8_t *name, uint8_t *args, int32_t *cmd_type)
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
    new_task->task_name = name;
    new_task->vidmap = 0;

    init_file_array(&new_task->fd_array);
    uint8_t *tep;
    uint8_t *fname;
    dentry_t file_dentry;
    int32_t io_type = 0;
    switch (*cmd_type)
    {
    case 1: // '>'
    {
        tep = args + 1;
        tep = skip_space(tep);
        dir_write(0, tep, MAX_LEN_FILE_NAME);
        if (0 == read_dentry_by_name((uint8_t *)tep, &file_dentry))
        {
            set_entry(&new_task->fd_array, STDOUT_FD, 2, file_dentry.file_name);
            new_task->fd_array.entries[STDOUT_FD].inode = file_dentry.inode_num;
        }
        else
        {
            printf("redirect fail\n");
        }
        break;
    }
    case 2: // '>>'
    {
        tep = args + 2;
        tep = skip_space(tep);
        dir_write(0, tep, MAX_LEN_FILE_NAME);
        if (0 == read_dentry_by_name((uint8_t *)tep, &file_dentry))
        {
            set_entry(&new_task->fd_array, STDOUT_FD, 2, file_dentry.file_name);
            new_task->fd_array.entries[STDOUT_FD].inode = file_dentry.inode_num;
            new_task->fd_array.entries[STDOUT_FD].file_position = get_file_size(file_dentry.inode_num);
        }
        else
        {
            printf("redirect fail\n");
        }
        break;
    }
    case 3: // exe '|'
    {
        // tep = args;
        // if (*tep == '|')
        // {
        //     io_type = 1;
        // }
        // tep = skip_space(tep);
        // fname = (uint8_t *)tem_in_array[0];
        // dir_write(0, fname, MAX_LEN_FILE_NAME);
        // if (0 == read_dentry_by_name((uint8_t *)fname, &file_dentry))
        // {
        //     set_entry(&new_task->fd_array, STDIN_FD, 2, file_dentry.file_name);
        //     new_task->fd_array.entries[STDIN_FD].inode = file_dentry.inode_num;
        //     tem_in_inode_array[0] = file_dentry.inode_num;
        //     new_task->fd_array.entries[STDIN_FD].file_position = 0;
        // }

        fname = (uint8_t *)tem_out_array[0];
        dir_write(0, fname, MAX_LEN_FILE_NAME);
        if (0 == read_dentry_by_name((uint8_t *)fname, &file_dentry))
        {
            set_entry(&new_task->fd_array, STDOUT_FD, 2, file_dentry.file_name);
            new_task->fd_array.entries[STDOUT_FD].inode = file_dentry.inode_num;
            tem_out_inode_array[0] = file_dentry.inode_num;
            new_task->fd_array.entries[STDOUT_FD].file_position = 0;
        }
        break;
    }
    case 4: // file '|'
    {
        // tep = args;
        // if (*tep == '|')
        // {
        //     io_type = 1;
        //     tep++;
        // }
        // tep = skip_space(tep);
        set_entry(&new_task->fd_array, STDIN_FD, 2, exist_fname_array[0]);
        new_task->fd_array.entries[STDIN_FD].inode = exist_inode_array[0];
        new_task->fd_array.entries[STDIN_FD].file_position = 0;
        // if (io_type == 1)
        // {
        //     fname = (uint8_t *)tem_out_array[0];
        //     dir_write(0, fname, MAX_LEN_FILE_NAME);
        //     if (0 == read_dentry_by_name((uint8_t *)fname, &file_dentry))
        //     {
        //         set_entry(&new_task->fd_array, STDOUT_FD, 2, file_dentry.file_name);
        //         new_task->fd_array.entries[STDOUT_FD].inode = file_dentry.inode_num;
        //         tem_out_inode_array[0] = file_dentry.inode_num;
        //         new_task->fd_array.entries[STDOUT_FD].file_position = 0;
        //     }
        // }
        /* code */
        break;
    }
    case 5: // exe '|' out
    {
        fname = (uint8_t *)tem_out_array[0];
        dir_write(0, fname, MAX_LEN_FILE_NAME);
        if (0 == read_dentry_by_name((uint8_t *)fname, &file_dentry))
        {
            set_entry(&new_task->fd_array, STDIN_FD, 2, file_dentry.file_name);
            new_task->fd_array.entries[STDIN_FD].inode = file_dentry.inode_num;
            tem_out_inode_array[0] = file_dentry.inode_num;
            new_task->fd_array.entries[STDIN_FD].file_position = 0;
        }
        break;
    }
    default:
        break;
    }

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
    dentry_t task_dentry; // copied task dentry
    uint8_t *args;        // arguments
    int32_t cmd_type = 0;
    int32_t cmd_type1 = 0;
    uint8_t *args1;
    // int32_t page_id;      // new page id
    PCB_t *new_task; // new task
    uint32_t eip;
    uint8_t args_array[MAX_ARGS + 1];
    uint8_t name_array[MAX_LEN_FILE_NAME + 1];
    uint8_t name_array1[MAX_LEN_FILE_NAME + 1];
    memset((void *)args_array, (int32_t) "\0", MAX_ARGS + 1);
    memset((void *)name_array, (int32_t) "\0", MAX_LEN_FILE_NAME + 1);
    memset((void *)name_array1, (int32_t) "\0", MAX_LEN_FILE_NAME + 1);
    // Parse args
    args = parse_args((uint8_t *)command, &cmd_type);
    if (cmd_type == 5)
    {
        command++;
        command = skip_space((uint8_t *)command);
        args = parse_args((uint8_t *)command, &cmd_type1);
    }

    // if (args == command)
    // {
    //     strcpy((int8_t *)name_array, (const int8_t *)command);
    //     strcpy((int8_t *)args_array, (const int8_t *)args);
    // }
    // else
    // {
    // printf("%s\n", command);
    strcpy((int8_t *)name_array, (const int8_t *)command);
    if (args != NULL)
    {
        // printf("%s\n", args);
        strcpy((int8_t *)args_array, (const int8_t *)args);
    }
    // }

    // get the dentry in fs
    if (read_dentry_by_name(command, &task_dentry) == -1)
    {
        printf("[INFO] Cannot execute non-exist file\n");
        return -1;
    }
    // Check for executable
    if (!is_exe_file(&task_dentry))
    {
        if (cmd_type == 3)
        {
            exist_inode_array[0] = task_dentry.inode_num;
            strcpy((int8_t *)name_array1, (const int8_t *)command);
            exist_fname_array[0] = name_array1;
            cmd_type = 4;

            if (*args == '|')
            {
                args++;
            }
            args = skip_space(args);
            args1 = parse_args((uint8_t *)args, &cmd_type1); // args :exe, args1:args
            if (read_dentry_by_name(args, &task_dentry) == -1 || !is_exe_file(&task_dentry))
            {
                printf("[INFO] Cannot execute non-exist/unexecutable file\n");
                return -1;
            }
            strcpy((int8_t *)name_array, (const int8_t *)args);
            if (args1 != NULL)
            {
                // printf("%s\n", args);
                strcpy((int8_t *)args_array, (const int8_t *)args1);
            }
            new_task = create_task(name_array, args_array, &cmd_type);
        }
        else
        {
            printf("[INFO] Cannot execute unexecutable file\n");
            return -1;
        }
    }
    else
    {
        // Create PCB, Set up paging
        new_task = create_task(name_array, args_array, &cmd_type);
    }
    if (new_task == NULL)
    {
        printf("[INFO] Cannot execute more than %d programs!\n", MAX_NUM_TASK);
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
        popl %%esi   \n\
        "
                 :
                 : "a"(USER_DS), "b"(USER_EBP), "c"(USER_CS), "d"(eip)
                 : "memory");
    // before return, check for pipeline
    uint8_t *tmp;
    if (cmd_type == 3)
    {
        tmp = args_array;
        if (*tmp == '|')
        {
            // tmp++;
            // tmp = skip_space(tmp);
            system_execute(tmp);
        }
    }
    else if (cmd_type == 5)
    {
        del_file((uint8_t *)tem_out_array[0]);
    }

    // todo
    asm volatile("            \n\
        movl %%esi, %%eax \n\
        leave \n\
        ret \n\
        "
                 :
                 :
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
    PCB_t *parent;
    if (page_array.num_using == 0)
    {
        printf("[INFO] nothing to halt\n");
        // system_execute((uint8_t *)"shell");
        return -1;
    }
    if (curr_task()->vidmap == 1)
    {
        PDE_4KB_t *user_vid_pde = (PDE_4KB_t *)(&page_directory.pde[VID_PAGE_INDEX]);
        clear_PDE_4KB(user_vid_pde);
    }
    // try to deactivate task, get parent task
    parent = deactivate_task(curr_task());
    if (parent == NULL)
    {
        system_execute((uint8_t *)"shell");
    }
    // Restore parent paging
    restore_task_page(parent->pid);
    // Write Parent process’ info back to TSS
    tss.esp0 = curr_task()->saved_esp;
    // restore stack and return value
    asm volatile("         \n\
            movl %%edx, %%esp   \n\
            movl %%ecx, %%ebp   \n\
            andl $0, %%eax    \n\
            movb %%bl, %%al   \n\
            pushl %%eax   \n\
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
