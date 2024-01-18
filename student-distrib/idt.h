#ifndef _IDT_H
#define _IDT_H

#include "kb.h"
#include "rtc.h"
#include "lib.h"
#include "idt_wrapper.h"
#include "x86_desc.h"
#include "systemcall_wrapper.h"
#include "systemcall.h"

#define EXCEPTION_HALT 111

//initialization function for our idt
void init_idt();

extern void exception_handler(uint32_t exception_id);

#endif /* _IDT_H */
