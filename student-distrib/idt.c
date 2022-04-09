
/*** include files ***/
#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "asmlink.h"
#include "file_system.h"

/*** functions ***/

/*
 * idt_init
 * description: this function is used to initialize the IDT and load into idtr
 *              init fomate consult: https://pdos.csail.mit.edu/6.828/2005/lec/lec8-slides.pdf?msclkid=a862cd27a88c11ec9f17f997ae89db36
 * input: none
 * output: idt
 * return: none
 */
void idt_init()
{
    int i;

    /** init exceptions **/
    for (i = IDT_BY_INTEL_START; i <= IDT_BY_INTEL_END; i++)
    {
        idt[i].present = 0;
        idt[i].dpl = 0;
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved4 = 0;
        idt[i].seg_selector = KERNEL_CS;
    }

    /** init interupts **/
    for (i = IDT_BY_PIC_START; i <= IDT_BY_PIC_END; i++)
    {
        idt[i].present = 0;
        idt[i].dpl = 0;
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved4 = 0;
        idt[i].seg_selector = KERNEL_CS;
    }

    /** init APIC vectors **/
    for (i = IDT_BY_PIC_END + 1; i < IDT_BY_SYSCALL_START; i++)
    {
        idt[i].present = 0;
        idt[i].dpl = 0;
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved4 = 0;
        idt[i].seg_selector = KERNEL_CS;
    }

    /** init syscall vectors **/
    idt[IDT_BY_SYSCALL_START].present = 0;
    idt[IDT_BY_SYSCALL_START].dpl = 3; // for syscal the dpl is 3
    idt[IDT_BY_SYSCALL_START].reserved0 = 0;
    idt[IDT_BY_SYSCALL_START].size = 1;
    idt[IDT_BY_SYSCALL_START].reserved1 = 1;
    idt[IDT_BY_SYSCALL_START].reserved2 = 1;
    idt[IDT_BY_SYSCALL_START].reserved4 = 0;
    idt[IDT_BY_SYSCALL_START].seg_selector = KERNEL_CS;

    /** init more APIC vectors **/
    for (i = IDT_BY_SYSCALL_START + 1; i < NUM_VEC; i++)
    {
        idt[i].present = 0;
        idt[i].dpl = 0;
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved4 = 0;
        idt[i].seg_selector = KERNEL_CS;
    }

    /*** setup exception ***/
    SET_IDT_ENTRY(idt[0], IDT_EXCEPTION_0);
    SET_IDT_ENTRY(idt[1], IDT_EXCEPTION_1);
    SET_IDT_ENTRY(idt[2], IDT_EXCEPTION_2);
    SET_IDT_ENTRY(idt[3], IDT_EXCEPTION_3);
    SET_IDT_ENTRY(idt[4], IDT_EXCEPTION_4);
    SET_IDT_ENTRY(idt[5], IDT_EXCEPTION_5);
    SET_IDT_ENTRY(idt[6], IDT_EXCEPTION_6);
    SET_IDT_ENTRY(idt[7], IDT_EXCEPTION_7);
    SET_IDT_ENTRY(idt[8], IDT_EXCEPTION_8);
    SET_IDT_ENTRY(idt[9], IDT_EXCEPTION_9);
    SET_IDT_ENTRY(idt[10], IDT_EXCEPTION_10);
    SET_IDT_ENTRY(idt[11], IDT_EXCEPTION_11);
    SET_IDT_ENTRY(idt[12], IDT_EXCEPTION_12);
    SET_IDT_ENTRY(idt[13], IDT_EXCEPTION_13);
    SET_IDT_ENTRY(idt[14], IDT_EXCEPTION_14);
    SET_IDT_ENTRY(idt[15], IDT_EXCEPTION_15);
    SET_IDT_ENTRY(idt[16], IDT_EXCEPTION_16);
    SET_IDT_ENTRY(idt[17], IDT_EXCEPTION_17);
    SET_IDT_ENTRY(idt[18], IDT_EXCEPTION_18);
    SET_IDT_ENTRY(idt[19], IDT_EXCEPTION_19);
    SET_IDT_ENTRY(idt[20], IDT_EXCEPTION_20);
    SET_IDT_ENTRY(idt[21], IDT_EXCEPTION_21);
    SET_IDT_ENTRY(idt[28], IDT_EXCEPTION_28);
    SET_IDT_ENTRY(idt[29], IDT_EXCEPTION_29);
    SET_IDT_ENTRY(idt[30], IDT_EXCEPTION_30);
    SET_IDT_ENTRY(idt[31], IDT_EXCEPTION_31);
    /** enable exception **/
    for (i = IDT_BY_INTEL_START; i <= IDT_BY_INTEL_END; i++)
    {
        idt[i].present = 1;
    }

    /*** set up interupt and enable ***/
    SET_IDT_ENTRY(idt[IDT_BY_KEYBOARD], IDT_INTERUPT_33);
    idt[IDT_BY_KEYBOARD].present = 1;
    SET_IDT_ENTRY(idt[IDT_BY_RTC], IDT_INTERUPT_40);
    idt[IDT_BY_RTC].present = 1;

    /*** set up syscall and enable ***/
    SET_IDT_ENTRY(idt[IDT_BY_SYSCALL_START], IDT_SYSCALL);
    idt[IDT_BY_SYSCALL_START].present = 1;

    /*** load IDT to IDTR ***/
    lidt(idt_desc_ptr);
}

/*
 * print_exception
 * description: it is a help function for idt test to print the exception num and into the blue screen
 * input: exception_num
 * output: the detected exception and blue screen
 * return: none
 */
void print_exception(uint32_t exception_num)
{
    clear();
    printf(" Detect exception %x\n", exception_num);
    printf(" --------pretend it is a BLUE SCREEN---------\n");
    while (1)
    {
    }
}

/*
 * print_syscall
 * description: it is a help function for idt test to print the syscall num and into the blue screen
 * input: exception_num
 * output: the syscall num
 * return: none
 */
void print_syscall(uint32_t syscall_num)
{
    clear();
    printf(" Your syscall is %x, but not implement now\n", syscall_num);
    while (1)
    {
    }
}

/*
 * syscall_err
 * description: it will disp a err info on screen when user call a invalid system call
 * input: invalid_call
 * output: num is not a syscall
 * return: none
 */
void syscall_err(uint32_t invalid_call){
    clear();
    printf("System call %x is not valid check twice!!!\n", invalid_call);
    while(1){}
}

/*
 * system_opne
 * description: it classify the file type and call the corresponing handler
 * input: filename
 *        syscall
 * output: none
 * return 0 for fail other for success
 */
fastcall int32_t system_open(uint32_t syscall, uint32_t* filename ){
    /*** check file valid or not ***/
    int file_check;
    dentry_t current_dentry;
    file_check = read_dentry_by_name(filename, &current_dentry);
    if(file_check == -1){   //if check fail then show it is not a visible file
        printf("invalid file!!!\n");
        return -1;
    }

    /*** call the hanler function by file type ***/
    uint32_t file_type = current_dentry.file_type;
    switch (file_type)
    {
    case 0:
        return rtc_open(filename);
        break;

    case 1:
        printf("open dir not implenent now\n");
        break;

    case 2:
        return file_sys_open(filename);
        break;
    
    default:
        printf("invalid file type, check again!!!\n");
        break;
    }

    return -1;
}

fastcall int32_t system_close(int32_t fd){
    return file_sys_close(fd);
}

fastcall int32_t system_write(int32_t fd, const void *buf, int32_t nbytes){
    return file_sys_write(fd, buf, nbytes);
}

fastcall int32_t system_read(int32_t fd, void *buf, int32_t nbytes){
    return file_sys_read(fd, buf, nbytes);
}

fastcall int32_t system_halt (uint8_t status){
    clear();
    printf(" halt %x\n", status);
    printf(" --------pretend it is a BLUE SCREEN---------\n");
    while(1){}
}
