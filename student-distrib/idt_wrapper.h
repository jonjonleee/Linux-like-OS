#ifndef _IDT_WRAPPER_H
#define _IDT_WRAPPER_H
#ifndef ASM
#include "x86_desc.h"

// wrapper function for keyboard_irq_handler
extern void kb_wrapper();

// wrapper function for rtc_irq_handler
extern void rtc_wrapper();

// wrapper function for pit_irq_handler
extern void pit_wrapper();

// common assembly wrapper for exceptions raised
extern void exception_wrapper(uint32_t exception_id);

#endif
#endif
