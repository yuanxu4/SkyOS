#include "idt.h"

.marco save_pre_reg
    pushf
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %ebp
    pushl %esi
    pushl %edi
    push %fs
    push %es
    push %ds
.endm

.marco restore_pre_reg
    pop %ds
    pop %es
    pop %fs
    popl %edi
    popl %esi
    popl %ebp
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    popf
.endm

/*
 * IDT_EXCEPTION_X
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_0
IDT_EXCEPTION_0:
    pushl $0    # push exception number
    save_pre_reg
    call print_exception
    addl $4, %esp
    restore_pre_reg
    iret

/*
 * IDT_EXCEPTION_X
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_1
IDT_EXCEPTION_1:
    pushl $1    # push exception number
    save_pre_reg
    call print_exception
    addl $4, %esp
    restore_pre_reg
    iret

/*
 * IDT_EXCEPTION_X
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_2
IDT_EXCEPTION_2:
    pushl $2    # push exception number
    save_pre_reg
    call print_exception
    addl $4, %esp
    restore_pre_reg
    iret

/*
 * IDT_EXCEPTION_X
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_3
IDT_EXCEPTION_3:
    pushl $3    # push exception number
    save_pre_reg
    call print_exception
    addl $4, %esp
    restore_pre_reg
    iret

/*
 * IDT_EXCEPTION_X
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_4
IDT_EXCEPTION_4:
    pushl $4    # push exception number
    save_pre_reg
    call print_exception
    addl $4, %esp
    restore_pre_reg
    iret

/*
 * IDT_EXCEPTION_X
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_5
IDT_EXCEPTION_5:
    pushl $5    # push exception number
    save_pre_reg
    call print_exception
    addl $4, %esp
    restore_pre_reg
    iret

/*
 * IDT_EXCEPTION_X
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_6
IDT_EXCEPTION_6:
    pushl $0    # push exception number
    save_pre_reg
    call print_exception
    addl $4, %esp
    restore_pre_reg
    iret
    