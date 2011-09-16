#ifndef _SYS_H
#define _SYS_H 1
#include "context.h"
#include "semaphore.h"

void exit(int retval);
void _exit();
void _exit_kernel();
void _iret();

int getprio(int pid);
int chprio(int pid, int newprio);

int getpid();
int getppid();

int waitpid(int pid, int *retvalp);

int kill(int pid);

int start(int (*ptfunc)(void *), unsigned long ssize, int prio, const char *
name, void * arg);

unsigned long current_clock(void);


/*
 * Check if a pointer is not dangerous :
 * If we are calling a syscall from userspace,
 * only allow userspace pointers. (replace it by NULL)
 */
void* check_pointer(const void* p, int code);
#endif /* sys.h */
