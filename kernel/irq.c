#include "irq.h"
#include "cpu.h"

/* irqs is a bitwise OR of IRQ(x)s */
void init_irqs(int irqs) {
	char mask = inb(0x21);	
	outb(mask & ~irqs, 0x21);
}
