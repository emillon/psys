#include "segment.h"

#define DEF_INT(id) \
.global int##id;\
int##id:\
	pusha;\
	pushl %ds;\
	pushl %es;\
	movw  $KERNEL_DS, %ax;\
	movw  %ax, %ds;\
	movw  %ax, %es;\
	cld;\
	call handle_int##id;\
	popl %es;\
	popl %ds;\
	popa;\
	iret;

// put DEF_INTs here
DEF_INT(33)
DEF_INT(38)

.global pfault
pfault:
    pusha
    call handle_pfault
    popa
    iret
