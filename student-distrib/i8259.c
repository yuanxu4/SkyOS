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
 */
void i8259_init(void) {
    

    /* SPIN_LOCK_IRQSAVE???? */

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
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint8_t value;
    uint8_t value_master;
    if (irq_num < 8){
        value = inb(MASTER_8259_DATA)&(~(1<<irq_num));
        outb(value, MASTER_8259_DATA);
    }else{
        irq_num-=8;
        value = inb(SLAVE_8259_DATA)&(~(1<<irq_num));
        outb(value, SLAVE_8259_DATA);
        value_master = inb(MASTER_8259_DATA)&(~(0x02));
        outb(value_master, MASTER_8259_DATA);
    }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint8_t value;
    uint8_t value_master;
    if (irq_num < 8){
        value = inb(MASTER_8259_DATA)|(1<<irq_num);
        outb(value, MASTER_8259_DATA);
    }else{
        irq_num-=8;
        value = inb(SLAVE_8259_DATA)|(1<<irq_num);
        outb(value, SLAVE_8259_DATA);
        if (value == 0xFF){
            value_master = inb(MASTER_8259_DATA)|0x02;
            outb(value_master, MASTER_8259_DATA);
        }
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    uint8_t value;
    
    if (irq_num >= 8){
        value = irq_num | EOI;
        outb(value,SLAVE_8259_DATA);
        outb( 2 | EOI, MASTER_8259_DATA);
    }else{
        value = irq_num | EOI;
        outb(value,MASTER_8259_DATA);
    }
    
}
