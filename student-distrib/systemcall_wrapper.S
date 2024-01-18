#define ASM 1
#include "systemcall_wrapper.h"

# Description:  Check if the syscall value is valid, and call the jump table.
# inputs: none
# outputs: none
# effect: handles system call using the jump table

.globl systemcall_wrapper

systemcall_wrapper:
    pushl %ebp
    pushl %esi # push callee-saved registers
    pushl %edi
    
    cmpl $10, %eax # see if eax is greater than 10, if so jump to done
    ja error_done
    testl %eax, %eax # see if eax is less than or equal to 0, if so jump to done
    jle error_done
    addl $-1, %eax # decrement eax by 1 in order to make it zero indexed just like the jump table

    pushl %edx
    pushl %ecx # push caller-saved registers
    pushl %ebx

    call *jump_table(, %eax, 4) # call jumptable

    sti
    popl %ebx
    popl %ecx # pop caller saved registers
    popl %edx

    popl %edi # pop callee saved registers
    popl %esi
    popl %ebp
    iret

    error_done: # jumps here if eax is invalid

    popl %edi
    popl %esi
    popl %ebp
    movl $-1, %eax
    iret

jump_table: # jump table for system call functions
    .long halt, execute, read, write, open, close, getargs , vidmap , set_handler, sigreturn

