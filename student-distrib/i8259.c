/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)
#define PIC_EOI 0x20
#define MASTER_MASK_PIC		 0xFB
#define SLAVE_MASK_PIC		 0xFF
#define MASTER_IRQS 8

// from OSDEV https://wiki.osdev.org/PIC
/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */
#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */
#define M_to_S_port 2           // slave connected to master at port 2 (IRQ2)

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

// This function is used to initialize the 8259 Programmable Interrupt Controller (PIC).
// Inputs: none
// Outputs: none
// Effects: Initializes the PIC.
void i8259_init(void) {

    // The master and slave masks are set to the initial values.
    master_mask = MASTER_MASK_PIC;  
    slave_mask = SLAVE_MASK_PIC;

    // The Initialization Command Word 1 (ICW1) is sent to the command ports of both the master and slave PICs.
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    // The Initialization Command Word 2 (ICW2) is sent to the data ports of both the master and slave PICs.
    outb(ICW2_MASTER, MASTER_PORT);
    outb(ICW2_SLAVE, SLAVE_PORT);

    // The Initialization Command Word 3 (ICW3) is sent to the data ports of both the master and slave PICs.
    outb(ICW3_MASTER,MASTER_PORT);
    outb(ICW3_SLAVE,SLAVE_PORT);

    // The Initialization Command Word 4 (ICW4) is sent to the data ports of both the master and slave PICs.
    outb(ICW4,MASTER_PORT);
    outb(ICW4,SLAVE_PORT);

    // The masks are sent to the data ports of both the master and slave PICs.
    outb(master_mask, MASTER_PORT); 
    outb(slave_mask, SLAVE_PORT);  
}



// This function is used to enable (or unmask) a specific interrupt request (IRQ) line.
// Inputs: irq_num - The number of the IRQ line to be enabled.
// Outputs: none
// Effects: Enables the specified IRQ line.
void enable_irq(uint32_t irq_num) {
    uint16_t port; // The I/O port to which the command will be sent.
    uint8_t value; // The value to be sent to the port.

    // If the IRQ number is less than 8, it means that the interrupt was sent by the master PIC.
    if(irq_num < MASTER_IRQS) {
        port = PIC1_DATA; // Master PIC's data port
    } else {
        // If the IRQ number is 8 or more, it means that the interrupt was sent by the slave PIC.
        port = PIC2_DATA; // Slave PIC's data port
        irq_num -= MASTER_IRQS;
    }
    // Read the current value of the port, clear the bit corresponding to the IRQ line, and write it back to the port.
    value = inb(port) & ~(1 << irq_num);
    outb(value, port);   
}


// This function is used to disable (or mask) a specific interrupt request (IRQ) line.
// Inputs: irq_num - The number of the IRQ line to be disabled.
// Outputs: none
// Effects: Disables the specified IRQ line.
void disable_irq(uint32_t irq_num) {
    uint16_t port; // The I/O port to which the command will be sent.
    uint8_t value; // The value to be sent to the port.

    // If the IRQ number is less than 8, it means that the interrupt was sent by the master PIC.
    if(irq_num < MASTER_IRQS) {
        port = PIC1_DATA; // Master PIC's data port
    } else {
        // If the IRQ number is 8 or more, it means that the interrupt was sent by the slave PIC.
        port = PIC2_DATA; // Slave PIC's data port
        irq_num -= MASTER_IRQS;
    }
    // Read the current value of the port, set the bit corresponding to the IRQ line, and write it back to the port.
    value = inb(port) | (1 << irq_num);
    outb(value, port);   
}



// This function is used to send an end-of-interrupt (EOI) signal to the Programmable Interrupt Controller (PIC).
// The PIC is a device used to combine several sources of interrupt onto one or more CPU lines.
// Inputs: irq_num - The number of the interrupt request (IRQ) line to which the EOI signal will be sent.
// Outputs: none
// Effects: Sends an EOI signal to the specified IRQ line.
void send_eoi(uint32_t irq_num) {
    // If the IRQ number is 8 or more, it means that the interrupt was sent by the slave PIC.
    // The slave PIC is connected to the master PIC's IRQ2 line.
    if(irq_num >= MASTER_IRQS){
        irq_num -= MASTER_IRQS;
        // Send EOI to slave PIC
        outb(EOI | irq_num, PIC2_COMMAND);
        // Send EOI to master PIC's IRQ2 line
        outb(EOI | M_to_S_port, PIC1_COMMAND);
    }
    else{
        // If the IRQ number is less than 8, it means that the interrupt was sent by the master PIC.
        // Send EOI to master PIC
        outb(EOI | irq_num, PIC1_COMMAND);
    }
}
