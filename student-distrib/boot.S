# boot.S - start point for the kernel after GRUB gives us control
# vim:ts=4 noexpandtab

# history
# load gdt -- ky
/*
 * ECE 391 SP 2022
 * History:
 * load gdt           - Mar 17, keyi
 * add enable_paging function     - Mar 19, keyi
 * 
 */


#define ASM     1

#include "multiboot.h"
#include "x86_desc.h"

.text

    # Multiboot header (required for GRUB to boot us)
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

# Entrypoint to the kernel
.globl start, _start

.align 4
start:
_start:
    # Make sure interrupts are off
    cli
    jmp     continue

continue:
    # Load the GDT
    lgdt gdt_desc_ptr

    # Load CS with the new descriptor value
    ljmp    $KERNEL_CS, $keep_going

keep_going:
    # Set up ESP so we can have an initial stack
    movl    $0x800000, %esp

    # Set up the rest of the segment selector registers
    movw    $KERNEL_DS, %cx
    movw    %cx, %ss
    movw    %cx, %ds
    movw    %cx, %es
    movw    %cx, %fs
    movw    %cx, %gs

    # Push the parameters that entry() expects (see kernel.c):
    # eax = multiboot magic
    # ebx = address of multiboot info struct
    pushl   %ebx
    pushl   %eax

    # Jump to the C entrypoint to the kernel.
    call    entry

    # We'll never get back here, but we put in a hlt anyway.
halt:
    hlt
    jmp     halt

.globl enable_paging


/*
 * enable_paging
 * enable paging: set page directory, used in kernel.c when booting
 * Interface: 
 *  Inputs: none
 *  Outputs: none
 * Side effect: enable paging
 * Registers: Clobbers eax, ecx, see cmt for usage
 */
enable_paging:
# C calling convention
    pushl %ebp
    movl %esp, %ebp
# set the page table base addr for 1st PDE (0-4MB)
    movl page_directory, %eax # the 1st entry in PD, 0x00000003
    movl $page_table, %ecx # the addr of PT, (aligned 4KB) 0xXXXXX000
    orl %eax, %ecx
    movl %ecx, page_directory
# set regs
    # cr3: bit 31-12, Page-Directory Base; bit 4,3, PCD,PWT, both 0; other ignored,0
    movl $page_directory, %eax # the addr of PD, (aligned 4KB) 0xXXXXX000
    # andl $0xFFFFFFE7, %eax cr3
    movl %eax, %cr3

    # cr4: bit 4: PSE, set
    movl    %cr4, %eax
    orl     $0x00000010, %eax
    movl    %eax, %cr4

    # cr0: bit 31, pg(enable paging); bit 0, pe (protection enable). both set
    movl    %cr0, %eax
    orl     $0x80000001, %eax
    movl    %eax, %cr0   

    leave
    ret

