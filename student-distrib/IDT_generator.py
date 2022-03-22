f = open("idt_ref.S", "w")
f.write("#define ASM     1\n")
f.write('''#include "idt.h"\n\n\n''')

# write exception handler
exception_num = [0,1,2,3,4,5,6,7,9,15,16,18,19,20,28,31]

for i in exception_num:
    f.write('''/*
 * IDT_EXCEPTION_''')
    f.write(str(i))
    f.write("\n")
    f.write(''' * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number
 * return: none
 */\n''')
    f.write(".globl IDT_EXCEPTION_")
    f.write(str(i))
    f.write("\n")
    f.write("IDT_EXCEPTION_")
    f.write(str(i))
    f.write(":\n")
    f.write("    # save_pre_reg\n")
    f.write('''    pushf
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %ebp
    pushl %esi
    pushl %edi
    push %fs
    push %es
    push %ds\n''')
    f.write("    pushl $")
    f.write(str(i))
    f.write("\n")
    f.write("    call print_exception\n")
    f.write("    addl $4, %esp\n")
    f.write("    # restore_pre_reg\n")
    f.write('''    pop %ds
    pop %es
    pop %fs
    popl %edi
    popl %esi
    popl %ebp
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    popf\n''')
    f.write("    iret\n\n\n")

# write exception with handler
exception_with_errcode = [8,10,11,12,13,14,17,21,29,30]

for i in exception_with_errcode:
    f.write('''/*
 * IDT_EXCEPTION_''')
    f.write(str(i))
    f.write("\n")
    f.write(''' * description: exception handler used to handle diferent exception
 * input: none
 * output: the exception number, err_code
 * return: none
 */\n''')
    f.write(".globl IDT_EXCEPTION_")
    f.write(str(i))
    f.write("\n")
    f.write("IDT_EXCEPTION_")
    f.write(str(i))
    f.write(":\n")
    f.write("    # save_pre_reg\n")
    f.write('''    pushf
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %ebp
    pushl %esi
    pushl %edi
    push %fs
    push %es
    push %ds\n''')
    f.write("    pushl $")
    f.write(str(i))
    f.write("\n")
    f.write("    call print_exception\n")
    f.write("    addl $4, %esp\n")
    f.write("    # restore_pre_reg\n")
    f.write('''    pop %ds
    pop %es
    pop %fs
    popl %edi
    popl %esi
    popl %ebp
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    popf\n''')
    f.write("    addl $4, %esp\n")
    f.write("    iret\n\n\n")

# write exception with handler
interupt = [33,40]
fun = ["keyboard_handler", "rtc_interrupt_handler"]

for i in interupt:
    f.write('''/*
 * IDT_INTERUPT_''')
    f.write(str(i))
    f.write("\n")
    f.write(''' * description: interupt handler used to handle diferent exception
 * input: none
 * output: the interupt reaction
 * return: none
 */\n''')
    f.write(".globl IDT_INTERUPT_")
    f.write(str(i))
    f.write("\n")
    f.write("IDT_INTERUPT_")
    f.write(str(i))
    f.write(":\n")
    f.write("    # save_pre_reg\n")
    f.write('''    pushf
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %ebp
    pushl %esi
    pushl %edi
    push %fs
    push %es
    push %ds\n''')
   #  f.write("    pushl $")
   #  f.write(str(i))
   #  f.write("\n")
    f.write("    call ")
    f.write(fun[interupt.index(i)])
    f.write("\n")
   #  f.write("    addl $4, %esp\n")
    f.write("    # restore_pre_reg\n")
    f.write('''    pop %ds
    pop %es
    pop %fs
    popl %edi
    popl %esi
    popl %ebp
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    popf\n''')
    f.write("    iret\n\n\n")

# syscall
f.write('''/*
* IDT_SYSCALL''')
f.write("\n")
f.write(''' * description: systemcall handler used to handle diferent exception
* input: none
* output: the systemcall reaction
* return: none
*/\n''')
f.write(".globl IDT_SYSCALL")
f.write("\n")
f.write("IDT_SYSCALL")
f.write(":\n")
f.write("    # save_pre_reg\n")
f.write('''    pushf
   pushl %eax
   pushl %ebx
   pushl %ecx
   pushl %edx
   pushl %ebp
   pushl %esi
   pushl %edi
   push %fs
   push %es
   push %ds\n''')
f.write("    call print_syscall\n")
f.write("    # restore_pre_reg\n")
f.write('''    pop %ds
   pop %es
   pop %fs
   popl %edi
   popl %esi
   popl %ebp
   popl %edx
   popl %ecx
   popl %ebx
   popl %eax
   popf\n''')
#f.write("    addl $4, %esp\n")
f.write("    iret\n\n\n")

f.close()