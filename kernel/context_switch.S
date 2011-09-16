#include "segment.h"

.global context_switch
context_switch:
    push %ebp
    mov  %esp, %ebp

    push %ebp

    mov 8(%ebp),  %eax    // %eax = &old_context
    mov   %esp, 0(%eax)   // old->esp_kernel = %esp

    mov 12(%ebp),  %eax   // %eax = &new_context
    mov 0(%eax),  %esp    // %esp = new->esp_kernel
    
    mov  4(%eax), %ebx    // %ebx = new->kernel_stack
    mov 20(%eax), %ecx    // %ecx = new->stack_size
    add %ecx, %ebx        // %ebx = new->kernel_stack + %ecx
    mov %ebx, 0x20004     // TSS(0) = new->esp_kernel
    mov $KERNEL_DS, %eax  // %eax = KERNEL_DS
    mov %eax, 0x20008     // TSS(1) = %eax

    call trt_sigs
    call garbage_collector

    pop %ebp

    leave
    ret


.global int32
int32:
    push %eax
    push %ebx
    push %ecx
    push %edx
    push %ebp
    push %esi
    push %edi
    push %ds
    push %es
    push %fs
    push %gs

    incl nb_interrupt
    mov $0x20, %al
    outb %al, $0x20
    call update_timer
    call scheduler

    pop %gs
    pop %fs
    pop %es
    pop %ds
    pop %edi
    pop %esi
    pop %ebp
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax

    iret

.global _exit
_exit:
    mov %eax, %ebx
    mov $0x21, %eax
    int $49


.global _exit_kernel
_exit_kernel:
    mov $0, %eax
    push %eax
    call exit


.global _iret
_iret:
    mov $0x4b, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    iret

.global _sigreturn
_sigreturn:
    mov %eax, %ebx
    mov $0x08, %eax
    int $49
