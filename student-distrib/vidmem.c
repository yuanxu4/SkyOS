
#include "lib.h"
#include "x86_desc.h"
#include "vidmem.h"
#include "task.h"
#include "keyboard.h"
#include "file_system.h"
#include "types.h"
#include "paging.h"

// 33*4 = 132 for the page derectory to 132-136mb
#define VID_USER_START_ADDR (TASK_VIR_ADDR_END + VIDEO_MEM_INDEX * SIZE_4KB) // 132MB + 0xb8 * 4K

static PT_t user_vid_pt;
//***todo: make PT_t aligned 4KB in define!!!!

extern void flush_TLB();   // defined in boot.S
extern PCB_t *curr_task(); // defined in boot.S
//***todo: put the curr_task into a *.h file which can not define it in every file

/*
 * system_vidmap(uint8_t **screen_start)
 * this is the syscall to call the make the user vidmem point to the physical vidmem
 * input: screen_start-- virtual address in the running program
 * return: 0 for success, -1 for fail
 * side effect: screen start will change
 */
int32_t sys_vidmap(uint8_t **screen_start)
{
    if (screen_start == NULL)
    {
        printf("[INFO] invalid pointer for screen start\n");
        return -1;
    }
    // check the screen start in valid user space
    if ((int)screen_start < TASK_VIR_ADDR || (int)screen_start >= TASK_VIR_ADDR_END)
    {
        printf("[INFO] screen_start's is out the user space which is invalid\n");
        return -1;
    }
    curr_task()->vidmap = 1;

    // set the page directory and page table
    PDE_4KB_t *user_vid_pde = (PDE_4KB_t *)(&page_directory.pde[VID_PAGE_INDEX]);
    set_PTE_4KB((PTE_4KB_t *)(&user_vid_pt.pte[VIDEO_MEM_INDEX]), VIDEO_MEM_INDEX * SIZE_4KB, 1, 1, 1);
    set_PDE_4KB(user_vid_pde, (uint32_t)(&user_vid_pt), 1, 1, 1);
    flush_TLB();
    *screen_start = (uint8_t *)VID_USER_START_ADDR;
    return 0;
}
