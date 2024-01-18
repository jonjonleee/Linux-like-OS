#include "idt.h"
#include "idt_wrapper.h"


#define SYS_CALL 0x80
#define PIT 0x20
#define KEYBOARD 0x21
#define RTC 0x28
#define PROGRAM_DEAD 256

// exception handlers, from 0-19 as well as one for system calls
// each one is passed into an IDT entry
// they are not called outside of interrupt/exception handling
void exception_0 () {
    exception_wrapper(0);
}

void exception_1 () {
    exception_wrapper(1);
}

void exception_2 () {
    exception_wrapper(2);
}

void exception_3 () {
    exception_wrapper(3);
}

void exception_4 () {
    exception_wrapper(4);
}

void exception_5 () {
    exception_wrapper(5);
}

void exception_6 () {
    exception_wrapper(6);
}

void exception_7 () {
    exception_wrapper(7);
}

void exception_8 () {
    exception_wrapper(8);
}

void exception_9 () {
    exception_wrapper(9);
}

void exception_10 () {
    exception_wrapper(10);
}

void exception_11 () {
    exception_wrapper(11);
}

void exception_12 () {
    exception_wrapper(12);
}


void exception_13 () {
    exception_wrapper(13);
}

void exception_14 () {
    exception_wrapper(14);
}

void exception_15 () {
    exception_wrapper(15);
}

void exception_16(){
    exception_wrapper(16);
}
void exception_17(){
    exception_wrapper(17);
}
void exception_18(){
    exception_wrapper(18);
}
void exception_19(){
    exception_wrapper(19);
}

void system_call_interrupt(){
    systemcall_wrapper();
}

//initializatoin function for our idt
// Inputs: none
// Outputs: none
// Effects: sets all of our idt entries and loads IDTR
void init_idt() {
    int i;
    // IDT has 256 entries
    for (i = 0; i < NUM_VEC; i++) {
        // set default values
        idt[i].seg_selector = KERNEL_CS;
        // Present bit must be set (1) for the descriptor to be valid
        idt[i].present = 1;
        // All reserved0 is a 0
        idt[i].reserved0 = 0;

        // If first 32 or system call, trap gate; otherwise, interrupt gate
        // Trap Gate: 0b1111
        // Interrupt Gate: 0b1110
        idt[i].size = 1;
        // All reserved1 is a 1
        idt[i].reserved1 = 1;
        // All reserved2 is a 1
        idt[i].reserved2 = 1;

        idt[i].reserved3 = ((i < 32) || (i == SYS_CALL));
        // All reserved4 is a 0
        idt[i].reserved4 = 0;
        // CPU previlege levels. 3 for system calls and 0 for kernel mode
        if (i == SYS_CALL) {
            idt[i].dpl = 3;
        } else {
            idt[i].dpl = 0;
        }
    }
    // SET_IDT_ENTRY
    // Each idt[i] represents the ith entry in the IDT,
    // which correspond directly to the ith exception/interrupt
    SET_IDT_ENTRY(idt[0], exception_0);
    SET_IDT_ENTRY(idt[1], exception_1);
    SET_IDT_ENTRY(idt[2], exception_2);
    SET_IDT_ENTRY(idt[3], exception_3);
    SET_IDT_ENTRY(idt[4], exception_4);
    SET_IDT_ENTRY(idt[5], exception_5);
    SET_IDT_ENTRY(idt[6], exception_6);
    SET_IDT_ENTRY(idt[7], exception_7);
    SET_IDT_ENTRY(idt[8], exception_8);
    SET_IDT_ENTRY(idt[9], exception_9);
    SET_IDT_ENTRY(idt[10], exception_10);
    SET_IDT_ENTRY(idt[11], exception_11);
    SET_IDT_ENTRY(idt[12], exception_12);
    SET_IDT_ENTRY(idt[13], exception_13);
    SET_IDT_ENTRY(idt[14], exception_14);
    SET_IDT_ENTRY(idt[15], exception_15);
    SET_IDT_ENTRY(idt[16], exception_16);
    SET_IDT_ENTRY(idt[17], exception_17);
    SET_IDT_ENTRY(idt[18], exception_18);
    SET_IDT_ENTRY(idt[19], exception_19);
    // No interrupt functions for vectors 0x14 - 0x1F
    for(i = 0x14; i <= 0x1F; i++){
        SET_IDT_ENTRY(idt[i], NULL);
    }
    SET_IDT_ENTRY(idt[SYS_CALL], systemcall_wrapper);
    SET_IDT_ENTRY(idt[KEYBOARD], kb_wrapper);
    SET_IDT_ENTRY(idt[RTC], rtc_wrapper);
    SET_IDT_ENTRY(idt[PIT], pit_wrapper);
    lidt(idt_desc_ptr);
    return;
}



// Common exception handler
// Inputs: exception vector id
// Outputs: none
// Effects: prints which exception was raised and terminates the program by calling halt
void exception_handler(uint32_t exception_id) {
    if (exception_id == 0) {
        printf(" Divide Error Exception ");
    } else if (exception_id == 1) {
        printf(" Debug Exception ");
    } else if (exception_id == 2) {
        printf(" NMI Interrupt ");
    } else if (exception_id == 3) {
        printf(" Breakpoint ");
    } else if (exception_id == 4) {
        printf(" Overflow ");
    } else if (exception_id == 5) {
        printf(" BOUND Range Exceeded ");
    } else if (exception_id == 6) {
        printf(" Invalid Opcode (Undefined Opcode) ");
    } else if (exception_id == 7) {
        printf(" Device Not Available (No Math Coprocessor) ");
    } else if (exception_id == 8) {
        printf(" Double Fault ");
    } else if (exception_id == 9) {
        printf(" Coprocessor Segment Overrun(reserved) ");
    } else if (exception_id == 10) {
        printf(" Invalid TSS ");
    } else if (exception_id == 11) {
        printf(" Segment Not Present ");
    } else if (exception_id == 12) {
        printf(" Stack-Segment Fault ");
    } else if (exception_id == 13) {
        printf(" General Protection ");
    } else if (exception_id == 14) {
        printf(" Page Fault ");
    } else if (exception_id == 15) {
        printf(" Assertion Error ");
    } else if (exception_id == 16) {
        printf(" x87 FPU Floating-Point Error (Math Fault) ");
    } else if (exception_id == 17) {
        printf(" Alignment Check ");
    } else if (exception_id == 18) {
        printf(" Machine Check ");
    } else if (exception_id == 19) {
        printf(" SIMD Floating-Point Exception ");
    } else if (exception_id == SYS_CALL) {
        printf(" System Call ");
    } else {
        return;
        // the exception id passed is not in [0,19] nor a system call
    }

    // pass 111 as the parameter -> means program was terminated by exception
    halt(EXCEPTION_HALT);
}

