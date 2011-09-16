#include "floppy.h"
#include "cpu.h"
#include "irq.h"
#include "stdio.h"
#include "processor_structs.h"

extern void int38(void);

volatile int floppy_irq_recvd = 0;

void handle_int38(void)
{
    outb(0x20, 0x20);
    floppy_irq_recvd = 1;
}

void floppy_wait_for_irq(void)
{
    while(floppy_irq_recvd == 0) {
        __asm__ __volatile__ ("HLT");
    }
}

static const char * drive_types[8] = { 
    "none", 
    "360kB 5.25\"", 
    "1.2MB 5.25\"", 
    "720kB 3.5\"", 
    "1.44MB 3.5\"", 
    "2.88MB 3.5\"", 
    "unknown type", 
    "unknown type" 
}; 

void floppy_detect_drives()
{ 
    outb(0x10, 0x70); 
    unsigned drives = inb(0x71); 

    printf("FD0 : %s\n", drive_types[drives >> 4]); 
    printf("FD1 : %s\n", drive_types[drives & 0xf]); 
}

void floppy_init(void)
{
    return;
    fill_interrupt(38, KERNEL_CS, int38, 0);
    init_irqs(IRQ(6));
    floppy_detect_drives();
    /*
    outb(0x00, FDC_DOR);
    outb(0x0C, FDC_DOR);
    printf("Waiting for IRQ6...\n");
    floppy_wait_for_irq();
    printf("Got INT38\n");
    */
}

