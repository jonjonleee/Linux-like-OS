#include "schedule.h" 
#include "i8259.h"
#include "terminal.h"
#include "systemcall.h"
#include "paging.h"
#include "file_sys.h"

int temp_terminal = 0;

// Description: Initializes the PIT to schedule a timer interrupt every 20 milliseconds
// Inputs: None
// Outputs: None
// Got inspiration from https://wiki.osdev.org/Programmable_Interval_Timer
void PIT_init(){
    // cli(); // disable interrupts
    outb(CHANNEL_MODE, COMMAND_REG); // select mode and command
    outb(COUNT&0xFF, CHANNEL_0_DATA_PORT);        // send Low byte
    outb((COUNT&0xFF00)>>8, CHANNEL_0_DATA_PORT);    // send High byte
    enable_irq(0);
    return;
}

// Function: pit_irq_handler
// Description: Handles PIT(programmable interval timer) interrupts for scheduling.
// Inputs: None
// Outputs: None
void pit_irq_handler() {
    cli();
    send_eoi(0);

    // If the current terminal is not active, initialize it and execute a shell
    if (terminal_array[temp_terminal].on_off_flag == 0) {
        terminal_array[temp_terminal].on_off_flag = 1;
        int prev_terminal = current_terminal;
        open_terminal(temp_terminal);     

        execute( (uint8_t*)"shell" );

        open_terminal(prev_terminal);

        sti();

        return;
    }  
    // Find the next terminal to schedule
    // 3 is the number of terminals
    temp_terminal = (temp_terminal + 1) % 3;

    // If the current terminal does not have an active process, return
    if(terminal_array[temp_terminal].terminal_current_pid < 3){
        sti(); // Enable interrupts
        return;
    }

    // Get the PCB (Process Control Block) of the current and next process
    sched_terminal = temp_terminal;
    new_pid = terminal_array[sched_terminal].terminal_current_pid;

    pcb_t *pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));
    pcb_t *nxt_pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));

    // If the next terminal has an active process, switch the paging to its page directory
    // There are terminals at pid's 0,1,2
    // Check if there are any none-base shell processes funning
    if(terminal_array[sched_terminal].terminal_current_pid > 2){
        nxt_pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (terminal_array[sched_terminal].terminal_current_pid + 1));
        page_directory[USR_IDX].present = page_directory[USR_IDX].rw = page_directory[USR_IDX].us = page_directory[USR_IDX].ps = page_directory[USR_IDX].g = 1;
        page_directory[USR_IDX].pwt = page_directory[USR_IDX].pcd = page_directory[USR_IDX].acc = page_directory[USR_IDX].avl = page_directory[USR_IDX].avl_3 = 0;
        page_directory[USR_IDX].addy = ((uint32_t)(EIGHTMB + (nxt_pcb->pid * FOURMB))) >> SHIFT_12;
        flush_tlb();
    }
    
    // Set up the video memory paging for the next terminal
    vid_table[0].present = vid_table[0].rw = vid_table[0].us = 1;
    if(sched_terminal == current_terminal){
        vid_table[0].addy = VIDEO_START >> SHIFT_12;
    } else {
        vid_table[0].addy = (VIDEO_PAGES >> SHIFT_12) + sched_terminal;
    }

    // Update the Task State Segment (TSS) for the next process
    tss.ss0 = KERNEL_DS;
    // subtract 4 for a memfence for safety
    tss.esp0 = EIGHTMB - EIGHTKB * new_pid - 4;

    // Save the current process's stack pointer and base pointer
	asm volatile(
        "movl   %%ebp, %0   ;"
        "movl   %%esp, %1   ;"
        :"=r"(pcb->program_ebp), "=r"(pcb->program_esp)
    );

    // Load the next process's stack pointer and base pointer
    asm volatile(
        "movl   %0, %%esp   ;"
        "movl   %1, %%ebp   ;"
        : :"r"(nxt_pcb->program_esp), "r"(nxt_pcb->program_ebp) 
    );

    sti();
    return;
}
