#define ASM_lib     1
#include "lib.h"

# .globl open
# open:
#    movl $0x05, %eax    # move syscall index to eax
#    movl 4(%ebp), %ebx # move fd to ebx
#    INT $0x80    # generate systemcall interupt




