#ifndef _SYSCALL_HANDLERS
#define _SYSCALL_HANDLERS 1

/**************************************************************
 *                    syscall_handlers.h                      *
 *                                                            *
 *                   Wrappers for syscalls                    *
 **************************************************************/


void syscall_handler0(unsigned long code);
void syscall_handler1(unsigned long code, unsigned long a1);
void syscall_handler2(unsigned long code, unsigned long a1, unsigned long a2);
void syscall_handler3(unsigned long code, unsigned long a1,
                      unsigned long a2,   unsigned long a3);
void syscall_handler4(unsigned long code, unsigned long a1, unsigned long a2,
                      unsigned long a3,   unsigned long a4);
void syscall_handler5(unsigned long code, unsigned long a1, unsigned long a2,
                      unsigned long a3,   unsigned long a4, unsigned long a5);

/* Are we handling a syscall ? */
int is_in_syscall(int syscall_code);

#endif /* syscall_handlers.h */
