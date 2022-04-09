#define ASM_lib     1
#include "lib.h"

.globl open
open:
    movl $0x05, %eax    # move syscall index to eax
    movl 4(%ebp), %ebx # move fd to ebx
    movl 8(%ebp), %ecx  # move buffer
    movl 12(%ebp), %edx # move nbyte
    INT $0x80    # generate systemcall interupt

.globl halt
halt:
    movl $0x00, %eax    # move syscall index to eax
    movl $1, %ebx
    INT $0x80    # generate systemcall interupt


