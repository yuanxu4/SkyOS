# x86_desc.S - Set up x86 segment descriptors, descriptor tables
# vim:ts=4 noexpandtab

# history
# add gdt_desc_ptr -- ky
/*
 * ECE 391 SP 2022
 * History:
 * add gdt_desc_ptr           - Mar 17, keyi
 * init paging      - Mar 19, keyi
 * 
 */


#define ASM     1
#include "x86_desc.h"

.text

.globl ldt_size, tss_size
.globl gdt_desc, ldt_desc, tss_desc
.globl tss, tss_desc_ptr, ldt, ldt_desc_ptr
# add gdt_desc_ptr
.globl gdt_ptr, gdt_desc_ptr
.globl idt_desc_ptr, idt

# for paging
.globl page_directory
.globl page_table

.align 4


tss_size:
    .long tss_bottom - tss - 1

ldt_size:
    .long ldt_bottom - ldt - 1

    .word 0 # Padding
ldt_desc:
    .word KERNEL_LDT
    .long ldt

    .align 4
tss:
_tss:
    .rept 104
    .byte 0
    .endr
tss_bottom:

# add GDT descriptor pointer, used in boot.S
# the same as idt_desc_ptr
.align 4
    .word 0 # Padding
gdt_desc_ptr:
    .word gdt_bottom - gdt - 1 # limit, size of gdt - 1, 2B
    .long gdt # base, addr of the first entry in gdt, 4B

    .align  16
gdt:
_gdt:

    # First GDT entry cannot be used
    .quad 0

    # NULL entry
    .quad 0

    # Segmentation will not be used
    # CS and DS both are 0-4GB r/w segments
    #
    # The layout is (from Intel IA-32 reference manual):
    #  31        24 23  22  21  20  19   16 15  14 13 12  11   8 7          0
    # |----------------------------------------------------------------------|
    # |            |   | D |   | A |  Seg  |   |  D  |   |      |            |
    # | Base 31:24 | G | / | 0 | V | Limit | P |  P  | S | Type | Base 23:16 |
    # |            |   | B |   | L | 19:16 |   |  L  |   |      |            |
    # |----------------------------------------------------------------------|
    #
    # |----------------------------------------------------------------------|
    # |                                    |                                 |
    # | Base 15:0                          | Segment Limit 15:0              |
    # |                                    |                                 |
    # |----------------------------------------------------------------------|

gdt_ptr:
    # Set up an entry for kernel CS
    .quad 0x00CF9A000000FFFF

    # Set up an entry for kernel DS
    .quad 0x00CF92000000FFFF

    # Set up an entry for user CS
    .quad 0x00CFFA000000FFFF

    # Set up an entry for user DS
    .quad 0x00CFF2000000FFFF

    # Set up an entry for TSS
tss_desc_ptr:
    .quad 0

    # Set up one LDT
ldt_desc_ptr:
    .quad 0

gdt_bottom:

    .align 16
ldt:
    .rept 4
    .quad 0
    .endr
ldt_bottom:

.align 4
    .word 0 # Padding
idt_desc_ptr:
    .word idt_bottom - idt - 1
    .long idt


    .align  16
idt:
_idt:
    .rept NUM_VEC
    .quad 0
    .endr

idt_bottom:

.align 4096 # align 4KB
# see mp3 doc
page_directory:
    # 0-4MB, for page table of 4KB pages
    .long   0x00000003  # init base addr(will be set when enable), set bit 0&1, present and R/W, bit7=0 for 4KB
    # 4-8MB, for 4MB kernel page, virtual = physical
    .long   0x00400083  # set base addr, set bit 0&1&7 set, present, R/W and PS for 4MB
    .long   0x00800083
    .long   0x00C00083
    .long   0x01000083
    .long   0x01400083
    .long   0x01800083
    .long   0x01C00083
    # .rept   8 - 2
    # .long   0
    # .endr
    # 32- 36MB for terminal
    .long   0x02000083
    # 8MB-4GB, not presented
    .rept PD_SIZE - 9
    .long 0 # just set 0 for not presented
    .endr

page_table:
    # Fill index 0-0xB7
    .rept 0xA0   # modex
    .long 0
    .endr

    # video memory, modex
    .long   0x000A0003
    .long   0x000A1003
    .long   0x000A2003
    .long   0x000A3003
    .long   0x000A4003
    .long   0x000A5003
    .long   0x000A6003
    .long   0x000A7003
    .long   0x000A8003
    .long   0x000A9003
    .long   0x000AA003
    .long   0x000AB003
    .long   0x000AC003
    .long   0x000AD003
    .long   0x000AE003
    .long   0x000AF003
    .long   0x000B0003
    .long   0x000B1003
    .long   0x000B2003
    .long   0x000B3003
    .long   0x000B4003
    .long   0x000B5003
    .long   0x000B6003
    .long   0x000B7003
    .long   0x000B8003
    .long   0x000B9003
    .long   0x000BA003
    .long   0x000BB003
    .long   0x000BC003
    .long   0x000BD003
    .long   0x000BE003
    .long   0x000BF003
    .long   0x000C0003


    # Fill the rest to 1024
    .rept PT_SIZE - 0xA0 - 33   # modex
    .long 0
    .endr

        

    # may use .macro to set addr for every entry
