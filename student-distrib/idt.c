
/*** include files ***/
#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"

/*** functions ***/

/*
 * idt_init
 * description: this function is used to initialize the IDT and load into idtr
 *              init fomate consult: https://pdos.csail.mit.edu/6.828/2005/lec/lec8-slides.pdf?msclkid=a862cd27a88c11ec9f17f997ae89db36
 * input: none
 * output: idt
 * return: none
 */
void idt_init(){
    int i;

    /** init exceptions **/
    for(i = IDT_BY_INTEL_START; i <= IDT_BY_INTEL_END; i ++){
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
    for(i = IDT_BY_PIC_START; i <= IDT_BY_PIC_END; i ++){
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
    for(i = IDT_BY_PIC_END + 1; i < IDT_BY_SYSCALL_START; i ++){
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
    idt[IDT_BY_SYSCALL_START].dpl = 3;
    idt[IDT_BY_SYSCALL_START].reserved0 = 0;
    idt[IDT_BY_SYSCALL_START].size = 1;
    idt[IDT_BY_SYSCALL_START].reserved1 = 1;
    idt[IDT_BY_SYSCALL_START].reserved2 = 1;
    idt[IDT_BY_SYSCALL_START].reserved4 = 0;
    idt[IDT_BY_SYSCALL_START].seg_selector = KERNEL_CS;

    /** init more APIC vectors **/
    for(i = IDT_BY_SYSCALL_START + 1; i < NUM_VEC; i ++){
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
    for(i = IDT_BY_INTEL_START; i <= IDT_BY_INTEL_END; i ++){
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

void print_exception(uint32_t exception_num){
    //clear();
    printf(" Detect exception %x\n", exception_num);
    printf(" --------pretend it is a BLUE SCREEN---------\n");
    while(1){}

}

void print_syscall(uint32_t syscall_num){
    //clear();
    printf(" Your syscall is %x, but not implement now\n", syscall_num);
    while(1){}
}
