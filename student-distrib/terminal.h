// terminal header file
#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "kb.h"
#include "systemcall.h"

#define BUF_SIZE 128
#define VID_MEM 0x1000
#define TERMINAL_COUNT 3
#define VIDEO_START    0xB8000
#define VIDEO_PAGES    0xBA000

// check if keyboard is ready for read
// this should usually be low
// then set to high specifically after there is an ENTER ('\n') detected
extern volatile uint32_t read_flag;

extern uint8_t terminal_buffer[BUF_SIZE];

// support for system calls
// doesn't do anything
extern int32_t terminal_open( const uint8_t* filename );

// doesn't do anything
extern int32_t terminal_close( int32_t fd );

// read from keyboard
extern int32_t terminal_read( int32_t fd, void* buf, int32_t nbytes );

// display the content from buffer to screen
extern int32_t terminal_write( int32_t fd, const void* buf, int32_t nbytes );

typedef struct terminal_t {
    uint8_t terminal_vidmem_buffer[BUF_SIZE];
    int32_t terminal_current_pid;
    uint32_t terminal_esp;
    uint32_t terminal_ebp;
    int32_t on_off_flag;
} terminal_t;

// array of our three open terminals
terminal_t terminal_array[TERMINAL_COUNT];
int32_t current_display;
int32_t current_scheduled_process;

extern int32_t current_terminal;
extern int32_t sched_terminal;

// initialize our three terminal structs
extern void term_init();

// open terminal with given id
extern void open_terminal(int32_t terminal_id);

#endif /* _TERMINAL_H */

