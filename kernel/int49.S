// User entrypoint
.global int49
int49:

/* Save registers */
    push  %ds
    push  %es
    push  %fs
    push  %gs

/* call the correct function, depending on %eax (see syscall_numbers.h) */

    cmp $0x20, %eax        // more than 0 arg ?
    jae 1f

    /* 0 arg */
    pushl %eax
    call syscall_handler0
    addl $4, %esp
    jmp syscall_end_dispatch

1:
    cmp $0x40, %eax        // more than 1 arg ?
    jae 2f

    /* 1 arg */
    pushl %ebx
    pushl %eax
    call syscall_handler1
    addl $4*2, %esp
    jmp syscall_end_dispatch

2:
    cmp $0x60, %eax        // more than 2 args ?
    jae 3f
    /* 2 args */
    pushl %ecx
    pushl %ebx
    pushl %eax
    call syscall_handler2
    addl $4*3, %esp
    jmp syscall_end_dispatch

3:
    cmp $0x70, %eax        // more than 3 args ?
    jae 4f
    /* 3 args */
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    call syscall_handler3
    addl $4*4, %esp
    jmp syscall_end_dispatch

4:
    cmp $0x80, %eax        // more than 4 args ?
    jae 5f
    /* 4 args */
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    call syscall_handler4
    addl $4*5, %esp
    jmp syscall_end_dispatch

5:
    /* 5 args */
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax
    call syscall_handler5
    addl $4*6, %esp

syscall_end_dispatch:

/* Restore registers */
    pop  %gs
    pop  %fs
    pop  %es
    pop  %ds

    iret
