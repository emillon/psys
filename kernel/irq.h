#ifndef _IRQ_H
#define _IRQ_H 1

/* 8254A interrupt controller registers */
/* Master */
#define PIC_M_COMMAND 0x0020
#define PIC_M_DATA    0x0021
/* Slave */
#define PIC_S_COMMAND 0x00A0 
#define PIC_S_DATA    0x00A1

#define IRQ(x) (1<<x)

void init_irqs(int irqs);

#endif /* irq.h */
