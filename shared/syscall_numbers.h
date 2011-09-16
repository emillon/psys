#ifndef _SYSCALL_NUMBERS_H
#define _SYSCALL_NUMBERS_H 1

/* 0 arg : 0x00-0x1f */
#define SYSCALL_CLOCK           0x00
#define SYSCALL_GETPID          0x01
#define SYSCALL_GETPPID         0x02
#define SYSCALL_CURRENT_CLOCK   0x03
#define SYSCALL_REBOOT          0x04
#define SYSCALL_CONTEXT_PS      0x05
#define SYSCALL_SEM_INFO        0x06
#define SYSCALL_CLEAR           0x07
#define SYSCALL_SIG_RETURN      0x08
/* 1 arg : 0x20-0x3f*/
#define SYSCALL_CONS_ECHO       0x20
#define SYSCALL_EXIT            0x21
#define SYSCALL_KILL            0x22
#define SYSCALL_SCOUNT          0x23
#define SYSCALL_SCREATE         0x24
#define SYSCALL_SDELETE         0x25
#define SYSCALL_GETPRIO         0x26
#define SYSCALL_SIGNAL          0x27
#define SYSCALL_WAIT_CLOCK      0x28
#define SYSCALL_WAIT            0x29
#define SYSCALL_USLEEP          0x2a
#define SYSCALL_OPEN            0x2b
#define SYSCALL_CLOSE           0x2c
#define SYSCALL_TRY_WAIT        0x2d
#define SYSCALL_CHDIR           0x2e
#define SYSCALL_SET_STDOUT      0x2f
/* 2 args : 0x40-0x5f*/
#define SYSCALL_CHPRIO          0x40
#define SYSCALL_CLOCK_SETTINGS  0x41
#define SYSCALL_CONS_READ       0x42
#define SYSCALL_CONS_WRITE      0x43
#define SYSCALL_SIGNALN         0x44
#define SYSCALL_SRESET          0x45
#define SYSCALL_WAITPID         0x46
#define SYSCALL_CONS_IOCTL      0x47
#define SYSCALL_BUZZER_BEEP     0x48
#define SYSCALL_FSTAT           0x49
#define SYSCALL_GETCWD          0x4a
#define SYSCALL_SIGNAL_SEND     0x4b
#define SYSCALL_SIGACTION       0x4c
/* 3 args : 0x60-0x7f */
#define SYSCALL_READ            0x60
#define SYSCALL_WRITE           0x61
#define SYSCALL_READDIR         0x62
/* 4 args : 0x80-0x9f */
/* nothing */
/* 5 args : 0xa0-0xbf */
#define SYSCALL_START           0xa0
/* 6 args : 0xc0-0xdf */
/* nothing */

#endif /* syscall_numbers.h */
