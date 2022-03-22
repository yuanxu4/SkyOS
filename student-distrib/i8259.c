/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */


/* Initialize the 8259 PIC 
 * Inputs: None
 * Outputs: None
 * Side Effects: init i8259
 *  first mask all interrupts then use four 
 * initialization control words
 * https://wiki.osdev.org/8259_PIC
 */
void i8259_init(void) {
    /* init two masks */
    master_mask = 0xFF;
    slave_mask = 0xFF;

    /* SPIN_LOCK_IRQSAVE???? */
    cli();
    /* mask all of PICs */
    outb(0XFF, MASTER_8259_DATA);   // 0xff mask all bits of interrupt
    outb(0xFF, SLAVE_8259_DATA);
    /* init master PIC */
    outb(ICW1, MASTER_8259_COMMAND);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER,MASTER_8259_DATA);
    outb(ICW4,MASTER_8259_DATA);
    /* init slave PIC */
    outb(ICW1, SLAVE_8259_COMMAND);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4,SLAVE_8259_DATA);

    /* restore mask of all IRQ */
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);
    sti();
}

/* Enable (unmask) the specified IRQ */
/* 
 * Inputs: irq_num -- interrupt request number
 * Outputs: None
 * Side Effects: unmask specified IRQ
 * https://wiki.osdev.org/8259_PIC
 */
void enable_irq(uint32_t irq_num) {
    uint8_t value;
    uint8_t value_master;
    /* less than 8 means irq of master PIC */
    if (irq_num < 8){
        value = inb(MASTER_8259_DATA)&(~(0x01<<irq_num)); //0x01 bit mask, active low
        outb(value, MASTER_8259_DATA);
    }else{
        /* get corresponding pin number */
        irq_num-=8;
        value = inb(SLAVE_8259_DATA)&(~(0x01<<irq_num)); //0x01 bit mask
        outb(value, SLAVE_8259_DATA);
        /* unmask the IR2(0100) in master PIC*/
        value_master = inb(MASTER_8259_DATA)&(~(0x04));
        outb(value_master, MASTER_8259_DATA);
    }
}

/* Disable (mask) the specified IRQ */
/* 
 * Inputs: irq_num -- interrupt request number
 * Outputs: None
 * Side Effects: mask specified IRQ
 * https://wiki.osdev.org/8259_PIC
 */
void disable_irq(uint32_t irq_num) {
    uint8_t value;
    uint8_t value_master;
    /* irq of Master PIC*/
    if (irq_num < 8){
        value = inb(MASTER_8259_DATA)|(0x01<<irq_num);//0x01 bit mask
        outb(value, MASTER_8259_DATA);
    }else{
        /* irq of Slave PIC */
        irq_num-=8;
        value = inb(SLAVE_8259_DATA)|(0x01<<irq_num);//0x01 bit mask
        outb(value, SLAVE_8259_DATA);
        /* if all slave PIC masked mask master's IR2 */
        if (value == 0xFF){
            /* mask master IRQ */
            value_master = inb(MASTER_8259_DATA)|0x04;
            outb(value_master, MASTER_8259_DATA);
        }
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
/* 
 * Inputs: irq_num -- interrupt request number
 * Outputs: None
 * Side Effects: send EOI signal to specified IRQ
 * https://wiki.osdev.org/8259_PIC
 */
void send_eoi(uint32_t irq_num) {
    uint8_t value;
    uint8_t value_master;
    /* irq of Slave PIC*/
    if (irq_num >= 8){
        /* irq of Slave PIC */
        irq_num-=8;
        value = irq_num | EOI;
        /* send EOI to master PIC and also mask slave port */
        value_master = 2 | EOI;
        outb(value,SLAVE_8259_COMMAND);
        outb(value_master, MASTER_8259_COMMAND);
    }else{
        value = irq_num | EOI;
        outb(value,MASTER_8259_COMMAND);
    }
    
}
