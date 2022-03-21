#define ASM     1

#include "idt.h"


/*
 * IDT_EXCEPTION_0
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_0
IDT_EXCEPTION_0:
    # save_pre_reg
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
    pushl $0
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_1
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_1
IDT_EXCEPTION_1:
    # save_pre_reg
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
    pushl $1
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_2
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_2
IDT_EXCEPTION_2:
    # save_pre_reg
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
    pushl $2
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_3
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_3
IDT_EXCEPTION_3:
    # save_pre_reg
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
    pushl $3
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_4
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_4
IDT_EXCEPTION_4:
    # save_pre_reg
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
    pushl $4
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_5
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_5
IDT_EXCEPTION_5:
    # save_pre_reg
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
    pushl $5
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_6
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_6
IDT_EXCEPTION_6:
    # save_pre_reg
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
    pushl $6
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_7
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_7
IDT_EXCEPTION_7:
    # save_pre_reg
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
    pushl $7
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_9
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_9
IDT_EXCEPTION_9:
    # save_pre_reg
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
    pushl $9
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_15
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_15
IDT_EXCEPTION_15:
    # save_pre_reg
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
    pushl $15
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_16
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_16
IDT_EXCEPTION_16:
    # save_pre_reg
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
    pushl $16
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_18
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_18
IDT_EXCEPTION_18:
    # save_pre_reg
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
    pushl $18
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_19
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_19
IDT_EXCEPTION_19:
    # save_pre_reg
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
    pushl $19
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_20
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_20
IDT_EXCEPTION_20:
    # save_pre_reg
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
    pushl $20
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_28
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_28
IDT_EXCEPTION_28:
    # save_pre_reg
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
    pushl $28
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_31
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */
.globl IDT_EXCEPTION_31
IDT_EXCEPTION_31:
    # save_pre_reg
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
    pushl $31
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    iret


/*
 * IDT_EXCEPTION_8
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_8
IDT_EXCEPTION_8:
    # save_pre_reg
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
    pushl $8
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_10
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_10
IDT_EXCEPTION_10:
    # save_pre_reg
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
    pushl $10
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_11
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_11
IDT_EXCEPTION_11:
    # save_pre_reg
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
    pushl $11
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_12
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_12
IDT_EXCEPTION_12:
    # save_pre_reg
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
    pushl $12
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_13
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_13
IDT_EXCEPTION_13:
    # save_pre_reg
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
    pushl $13
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_14
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_14
IDT_EXCEPTION_14:
    # save_pre_reg
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
    pushl $14
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_17
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_17
IDT_EXCEPTION_17:
    # save_pre_reg
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
    pushl $17
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_21
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_21
IDT_EXCEPTION_21:
    # save_pre_reg
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
    pushl $21
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_29
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_29
IDT_EXCEPTION_29:
    # save_pre_reg
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
    pushl $29
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_EXCEPTION_30
 * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */
.globl IDT_EXCEPTION_30
IDT_EXCEPTION_30:
    # save_pre_reg
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
    pushl $30
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_INTERUPT_33
 * description: interupt handler used to handle diferent exception
 * input: none
 * output: the interupt reaction
 * return: none
 */
.globl IDT_INTERUPT_33
IDT_INTERUPT_33:
    # save_pre_reg
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
    pushl $33
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
 * IDT_INTERUPT_40
 * description: interupt handler used to handle diferent exception
 * input: none
 * output: the interupt reaction
 * return: none
 */
.globl IDT_INTERUPT_40
IDT_INTERUPT_40:
    # save_pre_reg
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
    pushl $40
    call print_exception
    addl $4, %esp
    # restore_pre_reg
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
    addl $4, %esp
    iret


/*
* IDT_SYSCALL
 * description: systemcall handler used to handle diferent exception
* input: none
* output: the systemcall reaction
* return: none
*/
.globl IDT_SYSCALL
IDT_SYSCALL:
    # save_pre_reg
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
    call print_syscall
    # restore_pre_reg
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
    iret


