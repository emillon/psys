#include "start.h"
#include "syscalls.h"
#include "shell.h"
#include "stdio.h"

extern int test_proc(void*p);
void sys_info(void)
{
    context_ps();
    sem_info();
}

extern int testreg(void*arg);

/* Usermode entry point */
void user_start(void)
{
#if 1 // a changer en 1 pour les tests, 0 pour lancer les shells
    start(test_proc, STACK_SIZE, 128, "Tests", (void*) 0);
#else
    set_stdout("/dev/vc/1");
    printf("@ /dev/vc/1\n");
    start(shell_start, STACK_SIZE, 3, "Shell (1)", (void*) 0);
    set_stdout("/dev/vc/2");
    printf("@ /dev/vc/2\n");
    start(shell_start, STACK_SIZE, 3, "Shell (2)", (void*) 0);
    set_stdout("/dev/vc/3");
    printf("@ /dev/vc/3\n");
    start(shell_start, STACK_SIZE, 3, "Shell (3)", (void*) 0);
    set_stdout("/dev/vc/0"); // last so that kernel writes to vc0
    printf("@ /dev/vc/0\n");
    start(shell_start, STACK_SIZE, 3, "Shell (0)", (void*) 0);
#endif
    for(;;);
}

