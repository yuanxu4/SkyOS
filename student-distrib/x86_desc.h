/* x86_desc.h - Defines for various x86 descriptors, descriptor tables,
 * and selectors
 * vim:ts=4 noexpandtab
 */

/*
 * History:
 * add definition for paging           - Mar 19, keyi
 *
 */

#ifndef _X86_DESC_H
#define _X86_DESC_H

#include "types.h"

/* Segment selector values */
#define KERNEL_CS 0x0010
#define KERNEL_DS 0x0018
#define USER_CS 0x0023
#define USER_DS 0x002B
#define KERNEL_TSS 0x0030
#define KERNEL_LDT 0x0038

/* Size of the task state segment (TSS) */
#define TSS_SIZE 104

/* Number of vectors in the interrupt descriptor table (IDT) */
#define NUM_VEC 256

/* about paging  */
// # of entries in PT/PD
#define PT_SIZE 1024
#define PD_SIZE 1024
// size of 4KB/4MB
#define SIZE_4KB 0x1000
#define SIZE_4MB 0x400000
// index of video memory in PT, from lib.c
#define VIDEO_MEM_INDEX 0xB8
// about the size of the video memory:
// 1B*80*25*2=4KB

#ifndef ASM

/* This structure is used to load descriptor base registers
 * like the GDTR and IDTR */
typedef struct x86_desc
{
    uint16_t padding;
    uint16_t size;
    uint32_t addr;
} x86_desc_t;

/* This is a segment descriptor.  It goes in the GDT. */
typedef struct seg_desc
{
    union
    {
        uint32_t val[2];
        struct
        {
            uint16_t seg_lim_15_00;
            uint16_t base_15_00;
            uint8_t base_23_16;
            uint32_t type : 4;
            uint32_t sys : 1;
            uint32_t dpl : 2;
            uint32_t present : 1;
            uint32_t seg_lim_19_16 : 4;
            uint32_t avail : 1;
            uint32_t reserved : 1;
            uint32_t opsize : 1;
            uint32_t granularity : 1;
            uint8_t base_31_24;
        } __attribute__((packed));
    };
} seg_desc_t;

/* TSS structure */
typedef struct __attribute__((packed)) tss_t
{
    uint16_t prev_task_link;
    uint16_t prev_task_link_pad;

    uint32_t esp0;
    uint16_t ss0;
    uint16_t ss0_pad;

    uint32_t esp1;
    uint16_t ss1;
    uint16_t ss1_pad;

    uint32_t esp2;
    uint16_t ss2;
    uint16_t ss2_pad;

    uint32_t cr3;

    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint16_t es;
    uint16_t es_pad;

    uint16_t cs;
    uint16_t cs_pad;

    uint16_t ss;
    uint16_t ss_pad;

    uint16_t ds;
    uint16_t ds_pad;

    uint16_t fs;
    uint16_t fs_pad;

    uint16_t gs;
    uint16_t gs_pad;

    uint16_t ldt_segment_selector;
    uint16_t ldt_pad;

    uint16_t debug_trap : 1;
    uint16_t io_pad : 15;
    uint16_t io_base_addr;
} tss_t;

/* structure about paging */
// intel 3.7.6

// Page-Directory Entry (4-KByte Page Table)
typedef struct __attribute__((packed)) PDE_4KB
{
    uint32_t p : 1;       // Present, 1 for present
    uint32_t r_w : 1;     // Read/Write, 0 for read only, 1 for both read and write
    uint32_t u_s : 1;     // User/Supervisor, 0 for supervisor, 1 for user
    uint32_t pwt : 1;     // Write-through
    uint32_t pcd : 1;     // Cache disabled
    uint32_t a : 1;       // Accessed
    uint32_t r : 1;       // Reserved (set to 0)
    uint32_t ps : 1;      // Page size (0 indicates 4 KBytes)
    uint32_t g : 1;       // Global page (Ignored)
    uint32_t avail : 3;   // Available for system programmer’s use
    uint32_t pt_b_a : 20; // highest 20 bits of the physical base address of 4KB PT
} PDE_4KB_t;

// Page-Directory Entry (4-MByte Page)
typedef struct __attribute__((packed)) PDE_4MB
{
    uint32_t p : 1;      // Present, 1 for present
    uint32_t r_w : 1;    // Read/Write, 0 for read only, 1 for both read and write
    uint32_t u_s : 1;    // User/Supervisor, 0 for supervisor, 1 for user
    uint32_t pwt : 1;    // Write-through
    uint32_t pcd : 1;    // Cache disabled
    uint32_t a : 1;      // Accessed
    uint32_t d : 1;      // Dirty
    uint32_t ps : 1;     // Page size (1 indicates 4 MB)
    uint32_t g : 1;      // Global page (Ignored)
    uint32_t avail : 3;  // Available for system programmer’s use
    uint32_t pat : 1;    // Page Table Attribute Index, 0, used in the Pentium III
    uint32_t r : 9;      // Reserved (set to 0)
    uint32_t p_b_a : 10; // highest 20 bits of the physical base address of 4MB Page
} PDE_4MB_t;

// Page-Table Entry (4-KByte Page)
typedef struct __attribute__((packed)) PTE_4KB
{
    uint32_t p : 1;      // Present, 1 for present
    uint32_t r_w : 1;    // Read/Write, 0 for read only, 1 for both read and write
    uint32_t u_s : 1;    // User/Supervisor, 0 for supervisor, 1 for user
    uint32_t pwt : 1;    // Write-through
    uint32_t pcd : 1;    // Cache disabled
    uint32_t a : 1;      // Accessed
    uint32_t d : 1;      // Dirty
    uint32_t pat : 1;    // Page Table Attribute Index
    uint32_t g : 1;      // Global page (Ignored)
    uint32_t avail : 3;  // Available for system programmer’s use
    uint32_t p_b_a : 20; // highest 20 bits of the physical base address of 4KB Page
} PTE_4KB_t;

// notes: here we use uint32_t since PDE, PTE are actually uint32_t
// we can cast type when necessary
// Page-Table (for 4KB)
typedef struct __attribute__((aligned(SIZE_4KB))) PT
{
    uint32_t pte[PT_SIZE];
} PT_t;

// Page-Directory
typedef struct __attribute__((aligned(SIZE_4KB))) PD
{
    uint32_t pde[PD_SIZE];
} PD_t;

/* Some external descriptors declared in .S files */
extern x86_desc_t gdt_desc;

extern uint16_t ldt_desc;
extern uint32_t ldt_size;
extern seg_desc_t ldt_desc_ptr;
extern seg_desc_t gdt_ptr;
extern uint32_t ldt;

extern uint32_t tss_size;
extern seg_desc_t tss_desc_ptr;
extern tss_t tss;

// for paging use
extern PD_t page_directory;
extern PT_t page_table;

/* Sets runtime-settable parameters in the GDT entry for the LDT */
#define SET_LDT_PARAMS(str, addr, lim)                        \
    do                                                        \
    {                                                         \
        str.base_31_24 = ((uint32_t)(addr)&0xFF000000) >> 24; \
        str.base_23_16 = ((uint32_t)(addr)&0x00FF0000) >> 16; \
        str.base_15_00 = (uint32_t)(addr)&0x0000FFFF;         \
        str.seg_lim_19_16 = ((lim)&0x000F0000) >> 16;         \
        str.seg_lim_15_00 = (lim)&0x0000FFFF;                 \
    } while (0)

/* Sets runtime parameters for the TSS */
#define SET_TSS_PARAMS(str, addr, lim)                        \
    do                                                        \
    {                                                         \
        str.base_31_24 = ((uint32_t)(addr)&0xFF000000) >> 24; \
        str.base_23_16 = ((uint32_t)(addr)&0x00FF0000) >> 16; \
        str.base_15_00 = (uint32_t)(addr)&0x0000FFFF;         \
        str.seg_lim_19_16 = ((lim)&0x000F0000) >> 16;         \
        str.seg_lim_15_00 = (lim)&0x0000FFFF;                 \
    } while (0)

/* An interrupt descriptor entry (goes into the IDT) */
typedef union idt_desc_t
{
    uint32_t val[2];
    struct
    {
        uint16_t offset_15_00;
        uint16_t seg_selector;
        uint8_t reserved4;
        uint32_t reserved3 : 1;
        uint32_t reserved2 : 1;
        uint32_t reserved1 : 1;
        uint32_t size : 1;
        uint32_t reserved0 : 1;
        uint32_t dpl : 2;
        uint32_t present : 1;
        uint16_t offset_31_16;
    } __attribute__((packed));
} idt_desc_t;

/* The IDT itself (declared in x86_desc.S */
extern idt_desc_t idt[NUM_VEC];
/* The descriptor used to load the IDTR */
extern x86_desc_t idt_desc_ptr;

/* Sets runtime parameters for an IDT entry */
#define SET_IDT_ENTRY(str, handler)                                \
    do                                                             \
    {                                                              \
        str.offset_31_16 = ((uint32_t)(handler)&0xFFFF0000) >> 16; \
        str.offset_15_00 = ((uint32_t)(handler)&0xFFFF);           \
    } while (0)

/* Load task register.  This macro takes a 16-bit index into the GDT,
 * which points to the TSS entry.  x86 then reads the GDT's TSS
 * descriptor and loads the base address specified in that descriptor
 * into the task register */
#define ltr(desc)                       \
    do                                  \
    {                                   \
        asm volatile("ltr %w0"          \
                     :                  \
                     : "r"(desc)        \
                     : "memory", "cc"); \
    } while (0)

/* Load the interrupt descriptor table (IDT).  This macro takes a 32-bit
 * address which points to a 6-byte structure.  The 6-byte structure
 * (defined as "struct x86_desc" above) contains a 2-byte size field
 * specifying the size of the IDT, and a 4-byte address field specifying
 * the base address of the IDT. */
#define lidt(desc)                \
    do                            \
    {                             \
        asm volatile("lidt (%0)"  \
                     :            \
                     : "g"(desc)  \
                     : "memory"); \
    } while (0)

/* Load the local descriptor table (LDT) register.  This macro takes a
 * 16-bit index into the GDT, which points to the LDT entry.  x86 then
 * reads the GDT's LDT descriptor and loads the base address specified
 * in that descriptor into the LDT register */
#define lldt(desc)                \
    do                            \
    {                             \
        asm volatile("lldt %%ax"  \
                     :            \
                     : "a"(desc)  \
                     : "memory"); \
    } while (0)

#endif /* ASM */

#endif /* _x86_DESC_H */
