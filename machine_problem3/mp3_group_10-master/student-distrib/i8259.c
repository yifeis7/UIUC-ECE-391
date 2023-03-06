/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* i8259_init
 * Description: Initialize the 8259 PIC.
 * Inputs: None
 * Outputs: None
 * Side Effects: Initialize the 8259 PIC.
 */
void i8259_init(void)
{
    master_mask = inb(MASTER_8259_DATA);
    slave_mask = inb(SLAVE_8259_DATA);
    outb(MASK_ALL, MASTER_8259_DATA);       // mask int for primary PIC
    outb(MASK_ALL, SLAVE_8259_DATA);        // mask int for secondary PIC

    // primary PIC
    outb(ICW1, MASTER_8259_PORT);               // control port (3 CTWs expected)
    outb(ICW2_MASTER, MASTER_8259_DATA);    // IRQ 0 starts from 0x20 (entry in IDT table)
    outb(ICW3_MASTER, MASTER_8259_DATA);    // IRQ 2 connects to secondary PIC
    outb(ICW4, MASTER_8259_DATA);

    // secondary PIC
    outb(ICW1, SLAVE_8259_PORT);                // control port (3 CTWs expected)
    outb(ICW2_SLAVE, SLAVE_8259_DATA);      // IRQ8 starts from 0x28 (entry in IDT table)
    outb(ICW3_SLAVE, SLAVE_8259_DATA);      // INT line connects to IRQ2 in primary PIC
    outb(ICW4, SLAVE_8259_DATA);

    outb(MASK_ALL, MASTER_8259_DATA);       // mask int for primary PIC
    outb(MASK_ALL, SLAVE_8259_DATA);        // mask int for secondary PIC
}

/* enable_irq
 * Description: Enable (unmask) the specified IRQ.
 * Inputs:
        - irq_num: the IRQ to be enabled.
 * Outputs: None
 * Side Effects: Enable (unmask) the specified IRQ.
 */
void enable_irq(uint32_t irq_num)
{
    // check whether the input is valid
    if (irq_num < IRQ_LOW || irq_num > IRQ_HIGH)
    {
        printf("Invalid irq_num!\n");
        return;
    }
    else if (irq_num >= IRQ_NUM)
    {
        slave_mask = inb(SLAVE_8259_DATA); //+1 is for changing to data port
        slave_mask &= (~(0x01 << (irq_num - IRQ_NUM)));
        outb(slave_mask, SLAVE_8259_DATA);
    }
    else
    {
        master_mask = inb(MASTER_8259_DATA); //+1 is for changing to data port
        master_mask &= (~(0x01 << irq_num));
        outb(master_mask, MASTER_8259_DATA);
    }
}

/* disable_irq
 * Description: Disable (mask) the specified IRQ.
 * Inputs:
        - irq_num: the IRQ to be diabled.
 * Outputs: None
 * Side Effects: Disable (mask) the specified IRQ.
 */
void disable_irq(uint32_t irq_num)
{
    // check whether the input is valid
    if (irq_num < IRQ_LOW || irq_num > IRQ_HIGH)
    {
        printf("Invalid irq_num!\n");
        return;
    }
    else if (irq_num >= IRQ_NUM)
    {
        slave_mask = inb(SLAVE_8259_DATA); //+1 is for changing to data port
        slave_mask |= (0x01 << (irq_num - IRQ_NUM));
        outb(slave_mask, SLAVE_8259_DATA); //+1 is for changing to data port
    }
    else
    {
        master_mask = inb(MASTER_8259_DATA); //+1 is for changing to data port
        master_mask |= (0x01 << irq_num);
        outb(master_mask, MASTER_8259_DATA); //+1 is for changing to data port
    }
}

/* send_eoi
 * Description: Send end-of-interrupt signal for the specified IRQ.
 * Inputs:
        - irq_num: the IRQ to be sent EOI.
 * Outputs: None
 * Side Effects: Send end-of-interrupt signal for the specified IRQ.
 */
void send_eoi(uint32_t irq_num)
{
    // check whether the input is valid
    if (irq_num < IRQ_LOW || irq_num > IRQ_HIGH)
    {
        printf("Invalid irq_num!\n");
        return;
    }
    else if (irq_num >= IRQ_NUM) // slave PIC
    {
        // send EOI to both slave and IRQ2
        outb((irq_num - IRQ_NUM) | EOI, SLAVE_8259_PORT);  //slave
        outb(EOI | SLAVE_IRQ_LINE, MASTER_8259_PORT);  //master
    }
    else // master PIC
    {
        outb(irq_num | EOI, MASTER_8259_PORT);
    }
}
