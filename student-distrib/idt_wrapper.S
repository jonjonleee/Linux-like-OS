#define ASM 1
#include "idt_wrapper.h"

.globl kb_wrapper, rtc_wrapper, pit_wrapper, exception_wrapper

// wrapper function for keyboard_irq_handler
// Input: none
// Output: none
// Effects: calls keyboard_irq_handler

kb_wrapper:
    pushal
    pushfl
    call keyboard_irq_handler
    popfl
    popal
    iret

// wrapper function for rtc_irq_handler
// Input: none
// Output: none
// Effects: calls rtc_irq_handler

rtc_wrapper:
   pushal
   pushfl
   call rtc_irq_handler
   popfl
   popal
   iret

// wrapper function for pit_irq_handler
// Input: none
// Output: none
// Effects: calls pit_irq_handler

pit_wrapper:
   pushal
   pushfl
   call pit_irq_handler
   popfl
   popal
   iret


// wrapper function for exception handlers
// Input: exception vector id
// Output: none
// Effects: squash programs if they generate exceptions
exception_wrapper:
    pushl %ebp
    movl %esp, %ebp
    pushal
    movl 8(%ebp), %eax
    pushl %eax
    call exception_handler
    ; popl %eax
    addl $4, %esp
    popal
    iret

