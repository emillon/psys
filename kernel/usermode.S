#include "segment.h"

.global enter_usermode
// 4(%esp) : initial esp
enter_usermode:
    cli
    mov $USER_DS, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov 4(%esp), %eax
    pushl $USER_DS      // SS
    pushl %eax          // ESP
    pushl $0x202        // EFLAGS
    pushl $USER_CS      // CS
    pushl $0x1000000    // EIP
    iret                //  ^____ will pop all those

.global proc_syscall
proc_syscall:
    mov $10, %eax
    int $49
    ret


