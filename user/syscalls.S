#include "syscall_numbers.h"

#define REGISTER_PUSH \
            push %ebx;\
            push %ecx;\
            push %edx;\
            push %esi;\
            push %edi;

#define REGISTER_POP \
            pop %edi;\
            pop %esi;\
            pop %edx;\
            pop %ecx;\
            pop %ebx;

#define DEF_SYSCALL0(name,code) \
        .global name;\
        name:\
            push %ebp;\
            mov  %esp,%ebp;\
            REGISTER_PUSH;\
            mov  $##code, %eax;\
            int  $49;\
            REGISTER_POP;\
            leave;\
            ret;

#define DEF_SYSCALL1(name,code) \
        .global name;\
        name:\
            push %ebp;\
            mov  %esp,%ebp;\
            REGISTER_PUSH;\
            mov  $##code, %eax;\
            mov 8(%ebp), %ebx;\
            int  $49;\
            REGISTER_POP;\
            leave;\
            ret;

#define DEF_SYSCALL2(name,code) \
        .global name;\
        name:\
            push %ebp;\
            mov  %esp,%ebp;\
            REGISTER_PUSH;\
            mov  $##code, %eax;\
            mov  8(%ebp), %ebx;\
            mov 12(%ebp), %ecx;\
            int  $49;\
            REGISTER_POP;\
            leave;\
            ret;

#define DEF_SYSCALL3(name,code) \
        .global name;\
        name:\
            push %ebp;\
            mov  %esp,%ebp;\
            REGISTER_PUSH;\
            mov  $##code, %eax;\
            mov  8(%ebp), %ebx;\
            mov 12(%ebp), %ecx;\
            mov 16(%ebp), %edx;\
            int  $49;\
            REGISTER_POP;\
            leave;\
            ret;

#define DEF_SYSCALL5(name,code) \
        .global name;\
        name:\
            push %ebp;\
            mov  %esp,%ebp;\
            REGISTER_PUSH;\
            mov  $##code, %eax;\
            mov  8(%ebp), %ebx;\
            mov 12(%ebp), %ecx;\
            mov 16(%ebp), %edx;\
            mov 20(%ebp), %esi;\
            mov 24(%ebp), %edi;\
            int  $49;\
            REGISTER_POP;\
            leave;\
            ret;

DEF_SYSCALL0(current_clock,     SYSCALL_CURRENT_CLOCK)
DEF_SYSCALL0(getpid,            SYSCALL_GETPID)
DEF_SYSCALL0(getppid,           SYSCALL_GETPPID)
DEF_SYSCALL0(reboot,            SYSCALL_REBOOT)
DEF_SYSCALL0(context_ps,        SYSCALL_CONTEXT_PS)
DEF_SYSCALL0(sem_info,          SYSCALL_SEM_INFO)
DEF_SYSCALL0(clear,             SYSCALL_CLEAR)
DEF_SYSCALL0(sig_return,         SYSCALL_SIG_RETURN)

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



DEF_SYSCALL3(read,              SYSCALL_READ)
DEF_SYSCALL3(write,             SYSCALL_WRITE)
DEF_SYSCALL3(readdir,           SYSCALL_READDIR)

DEF_SYSCALL5(start,             SYSCALL_START)
