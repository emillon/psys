#include "syscall_handlers.h"
#include "syscall_numbers.h"
#include "sys.h"
#include "timer.h"
#include "syscalls.h"
#include "semaphore.h"
#include "signals.h"

#define SYSCALL_INIT(name,code) [code]= (void*) name,
#define DEF_SYSCALL0 SYSCALL_INIT
#define DEF_SYSCALL1 SYSCALL_INIT
#define DEF_SYSCALL2 SYSCALL_INIT
#define DEF_SYSCALL3 SYSCALL_INIT
#define DEF_SYSCALL5 SYSCALL_INIT

static unsigned long (*syscalls_0[])(void) ={
    DEF_SYSCALL0(current_clock,     SYSCALL_CURRENT_CLOCK)
    DEF_SYSCALL0(getpid,            SYSCALL_GETPID)
    DEF_SYSCALL0(getppid,           SYSCALL_GETPPID)
    DEF_SYSCALL0(reboot,            SYSCALL_REBOOT)
    DEF_SYSCALL0(context_ps,        SYSCALL_CONTEXT_PS)
    DEF_SYSCALL0(sem_info,          SYSCALL_SEM_INFO)
    DEF_SYSCALL0(clear,             SYSCALL_CLEAR)
    DEF_SYSCALL0(sig_return,        SYSCALL_SIG_RETURN) 
};

static unsigned long (*syscalls_1[])(unsigned long) ={
    DEF_SYSCALL1(exit,              SYSCALL_EXIT)
    DEF_SYSCALL1(kill,              SYSCALL_KILL)
    DEF_SYSCALL1(wait_clock,        SYSCALL_WAIT_CLOCK)
    DEF_SYSCALL1(getprio,           SYSCALL_GETPRIO)
    DEF_SYSCALL1(wait,              SYSCALL_WAIT)
    DEF_SYSCALL1(try_wait,          SYSCALL_TRY_WAIT)
    DEF_SYSCALL1(signal,            SYSCALL_SIGNAL)
    DEF_SYSCALL1(scount,            SYSCALL_SCOUNT)
    DEF_SYSCALL1(sdelete,           SYSCALL_SDELETE)
    DEF_SYSCALL1(cons_echo,         SYSCALL_CONS_ECHO)
    DEF_SYSCALL1(screate,           SYSCALL_SCREATE)
    DEF_SYSCALL1(usleep,            SYSCALL_USLEEP)
    DEF_SYSCALL1(open,              SYSCALL_OPEN)
    DEF_SYSCALL1(close,             SYSCALL_CLOSE)
    DEF_SYSCALL1(chdir,             SYSCALL_CHDIR)
    DEF_SYSCALL1(set_stdout,        SYSCALL_SET_STDOUT)
};

static unsigned long (*syscalls_2[])(unsigned long, unsigned long) ={
    DEF_SYSCALL2(waitpid,           SYSCALL_WAITPID)
    DEF_SYSCALL2(chprio,            SYSCALL_CHPRIO)
    DEF_SYSCALL2(signaln,           SYSCALL_SIGNALN)
    DEF_SYSCALL2(cons_read,         SYSCALL_CONS_READ)
    DEF_SYSCALL2(cons_write,        SYSCALL_CONS_WRITE)
    DEF_SYSCALL2(clock_settings,    SYSCALL_CLOCK_SETTINGS)
    DEF_SYSCALL2(cons_ioctl,        SYSCALL_CONS_IOCTL)
    DEF_SYSCALL2(sreset,            SYSCALL_SRESET)
    DEF_SYSCALL2(buzzer_beep,       SYSCALL_BUZZER_BEEP)
    DEF_SYSCALL2(fstat,             SYSCALL_FSTAT)
    DEF_SYSCALL2(signal_send,       SYSCALL_SIGNAL_SEND)
    DEF_SYSCALL2(sigaction,         SYSCALL_SIGACTION)
    DEF_SYSCALL2(getcwd,            SYSCALL_GETCWD)
};

static unsigned long (*syscalls_3[])(unsigned long, unsigned long,
                                                    unsigned long) ={
    DEF_SYSCALL3(read,              SYSCALL_READ)
    DEF_SYSCALL3(write,             SYSCALL_WRITE)
    DEF_SYSCALL3(readdir,           SYSCALL_READDIR)
};

static unsigned long (*syscalls_5[])(unsigned long, unsigned long,
unsigned long,unsigned long,unsigned long) ={
    DEF_SYSCALL5(start,             SYSCALL_START)
};

static int in_syscall = 0;

void syscall_handler0(unsigned long code)
{
    in_syscall=code;
    syscalls_0[code]();
    in_syscall=0xff;
}

void syscall_handler1(unsigned long code, unsigned long arg1)
{
    in_syscall=code;
    syscalls_1[code](arg1);
    in_syscall=0xff;
}

void syscall_handler2(unsigned long code, unsigned long arg1,
                      unsigned long arg2)
{
    in_syscall=code;
    syscalls_2[code](arg1, arg2);
    in_syscall=0xff;
}

void syscall_handler3(unsigned long code, unsigned long arg1,
                      unsigned long arg2, unsigned long arg3)
{
    in_syscall=code;
    syscalls_3[code](arg1, arg2, arg3);
    in_syscall=0xff;
}

void syscall_handler4(unsigned long code, unsigned long arg1,
                      unsigned long arg2, unsigned long arg3,
                      unsigned long arg4)
{
    /* nothing */
    (void) code;
    (void) arg1;
    (void) arg2;
    (void) arg3;
    (void) arg4;
}

void syscall_handler5(unsigned long code, unsigned long arg1,
                      unsigned long arg2, unsigned long arg3,
                      unsigned long arg4, unsigned long arg5)
{
    in_syscall=code;
    syscalls_5[code](arg1, arg2, arg3, arg4, arg5);
    in_syscall=0xff;
}

int is_in_syscall(int code)
{
    return in_syscall==code;
}
