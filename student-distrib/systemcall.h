#ifndef _SYSTEMCALL_H
#define _SYSTEMCALL_H

#include "lib.h"
#include "file_sys.h"
#include "paging.h"
#include "x86_desc.h"
#include "systemcall_wrapper.h"
#include "kb.h"
#include "tests.h"
#include "types.h"
#include "rtc.h"

#define NUM_DEVICES 6
#define RTC_INDEX 0
#define DIR_INDEX 1
#define FILE_INDEX 2
#define TERMINAL_INDEX 3
#define FILE_DESCRIPTOR_ARRAY_SIZE 8
#define MAX_OPEN_PROGS 6
#define EXCEPTION_HALT 111
#define RET_EXCEPTION_HALT 256
#define BUF_SIZE 128
#define USR_IDX 32
#define VID_IDX 34

// according to mp3 doc
// "The EIP you need to jump to is the entry point from bytes 24-27 of the executable that you have just loaded"
#define EIP_OFFSET 24
#define EIP_BYTES 4

#define FLAG_MASK 0x0200  
#define MEM_FENCE 4
#define USER_START 0x08000000
#define KEY_MEM 0x08400000 - MEM_FENCE    
#define USER_VID_MEM 0x08800000

// choose sufficiently large buffer size
// exact size doesn't really matter
#define RAND_BUF_SIZE 42

// define ASCII codes
#define ASCII_DEL 0x7F
#define ASCII_E 0x45
#define ASCII_L 0x4C
#define ASCII_F 0x46


// struct for pcb
typedef struct pcb_t{
    // information for current process/task
    int32_t pid;
    uint32_t program_esp;
    uint32_t program_ebp;

    // parent values for context switching
    // will be used on execute and halt to know where to go
    int32_t parent_pid;
    uint32_t parent_esp;
    uint32_t parent_ebp;
    
    // each program has its own set of files
    file_descriptor_t systemcall_fd_array[FILE_DESCRIPTOR_ARRAY_SIZE];
    
    // privilege info from TSS
    uint32_t esp;
    uint32_t ss;

    // arg buffer for get_args
    uint8_t arg_buff[BUF_SIZE];
} pcb_t;

extern int32_t curr_pid;
extern int32_t new_pid;

// functions needed for 3.3
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);

// Function to get the file operations table for a specific device
extern fops_table_t* get_fops_table(int device_index);
extern void init_fops_tables();
fops_table_t fops_table[NUM_DEVICES];
#endif

