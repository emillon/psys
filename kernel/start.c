#include "debugger.h"
#include "console.h"
#include "processor_structs.h"
#include "cpu.h"
#include "mem.h"
#include "user_stack_mem.h"
#include "stdio.h"
#include "context.h"
#include "timer.h"
#include "sys.h"
#include "semaphore.h"
#include "floppy.h"
#include "irq.h"
#include "signals.h"

extern void int49(void);
extern void pfault(void);

#define INIT(X) X##_init();

void handle_pfault(void)
{
    int pid = getpid();;
    printf("Exception catched");
    printf(" (pid = %d)\n", pid);
    kill(pid);
    scheduler();
}

void kernel_start(void)
{
    
    fill_interrupt(49, KERNEL_CS, int49, 3); // Usermode syscalls

    /* Protection faults etc */
    fill_interrupt(0x0d, KERNEL_CS, pfault, 0);
    fill_interrupt(0x0e, KERNEL_CS, pfault, 0);

    call_debugger();

    INIT(console);
    INIT(sem);
    //INIT(floppy);
    INIT(vfs);
    INIT(context);
    INIT(timer);
 
    sti();
    
    for(;;);
    return;
}

