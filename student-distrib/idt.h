#ifndef IDT_H
#define IDT_H

#ifndef ASM

/*** include lib ***/
#include "types.h"
#include "asmlink.h"

/*** default number ***/
#define IDT_BY_INTEL_START 0x00 // interupt defined by intel 0x00 - 0x1F
#define IDT_BY_INTEL_END 0x1F
#define IDT_BY_PIC_START 0x20 // interupt defined by PIC 0x20 - 0x2F
#define IDT_BY_PIC_END 0x2F
#define IDT_BY_SYSCALL_START 0x80 // interupt defined by system call 0x80

#define IDT_BY_SB16 0x25
#define IDT_BY_PIT 0x20
#define IDT_BY_KEYBOARD 0x21 // interupt by keyboard
#define IDT_BY_RTC 0x28      // interupt by RTC

/*** define function ***/
void idt_init(); // idt initialize function

/** exception handlers **/
extern void IDT_EXCEPTION_0();
extern void IDT_EXCEPTION_1();
extern void IDT_EXCEPTION_2();
extern void IDT_EXCEPTION_3();
extern void IDT_EXCEPTION_4();
extern void IDT_EXCEPTION_5();
extern void IDT_EXCEPTION_6();
extern void IDT_EXCEPTION_7();
extern void IDT_EXCEPTION_8();
extern void IDT_EXCEPTION_9();
extern void IDT_EXCEPTION_10();
extern void IDT_EXCEPTION_11();
extern void IDT_EXCEPTION_12();
extern void IDT_EXCEPTION_13();
extern void IDT_EXCEPTION_14();
extern void IDT_EXCEPTION_15();
extern void IDT_EXCEPTION_16();
extern void IDT_EXCEPTION_17();
extern void IDT_EXCEPTION_18();
extern void IDT_EXCEPTION_19();
extern void IDT_EXCEPTION_20();
extern void IDT_EXCEPTION_21();
extern void IDT_EXCEPTION_28();
extern void IDT_EXCEPTION_29();
extern void IDT_EXCEPTION_30();
extern void IDT_EXCEPTION_31();

/** interupt handlers **/
extern void IDT_INTERUPT_21();   // PIT
extern void IDT_INTERUPT_26(); // sb16
extern void IDT_INTERUPT_33(); // keyboard
extern void IDT_INTERUPT_40(); // RTC


/** syscall handlers **/
extern void IDT_SYSCALL();

/** cp1 identify exception **/
void print_exception(uint32_t exception_num);

/** cp1 indentify syscall **/
void print_syscall(uint32_t syscall_num);

/** cp2 syscall_err **/
void syscall_err(uint32_t invalid_call);

asmlinkage int32_t system_open(uint8_t* filename);

asmlinkage int32_t system_close(int32_t fd);

asmlinkage int32_t system_write(int32_t fd, const void *buf, int32_t nbytes);

asmlinkage int32_t system_read(int32_t fd, void *buf, int32_t nbytes);

asmlinkage int32_t system_halt (uint8_t status);


#endif

#endif
