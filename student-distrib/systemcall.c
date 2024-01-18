#include "systemcall.h"

// global variables to track the current and previous processes
int32_t new_pid = -1;
int32_t old_pid;

typedef int32_t ( *function )( );
typedef int32_t ( *read_function )( int32_t fd, void* buf, int32_t nbytes );
typedef int32_t ( *write_function )( int32_t fd, void* buf, int32_t nbytes );

// stores the current running processes
int32_t progs[MAX_OPEN_PROGS] = {0, 0, 0, 0, 0, 0};

// handles system call to halt
// includes functionality for ctrl + c (user controlled keyboard interrupts)
// Inputs: status - used to determine certain halt conditions
// Outputs: success (0), always
// Effects: resets pcb information and transitions between user/kernel spaces
int32_t halt(uint8_t status) {

    // retrieve pointer to current pcb
    pcb_t *pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));

    // current process has halted, so set the corresponding element in the progs to 0
    progs[new_pid] = 0;
    // make sure all files for the process are handled
    int i;
    for(i = 0; i < FILE_DESCRIPTOR_ARRAY_SIZE; i++){
        close(i);
    }
    // update current counters
    old_pid = new_pid;
    new_pid = pcb->parent_pid;
    terminal_array[sched_terminal].terminal_current_pid = new_pid;


    uint32_t par_ebp;
    uint32_t par_esp;
    par_ebp = pcb->parent_ebp;
    par_esp = pcb->parent_esp;
    pcb->parent_ebp = 0;
    pcb->parent_esp = 0;

    terminal_array[sched_terminal].terminal_esp = par_esp;
    terminal_array[sched_terminal].terminal_ebp = par_ebp;

    // handle special halt status cases
    // use a larger container/variable
    int32_t status_32bit = status;
    if(status_32bit == EXCEPTION_HALT){
        status_32bit = RET_EXCEPTION_HALT;
    } else if (status_32bit == USER_HALT){
        puts("\nInterrupt!!! Program interrupted by user");
    }

    // just done for visual purposes
    putc('\n');

    // make sure a program is always running, specifically shell
    if(new_pid == -1){
        execute((uint8_t*)"shell");
    }

    // sets the page table
    // maps the program to the relevant user space
    page_directory[USR_IDX].present = page_directory[USR_IDX].rw = page_directory[USR_IDX].us = page_directory[USR_IDX].ps = page_directory[USR_IDX].g = 1;
    page_directory[USR_IDX].pwt = page_directory[USR_IDX].pcd = page_directory[USR_IDX].acc = page_directory[USR_IDX].avl = page_directory[USR_IDX].avl_3 = 0;
    page_directory[USR_IDX].addy = ((uint32_t)(EIGHTMB + (new_pid * FOURMB))) >> SHIFT_12;
    flush_tlb();
    

    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHTMB - EIGHTKB * new_pid - 4;
 
    // restore the register values to the old, parent values before we jump
    asm volatile("movl %0, %%eax;"
                 "movl %1, %%ebp;" 
                 "movl %2, %%esp;" 
                 "leave;" 
                 "ret;" 
                : 
                : "r"(status_32bit), "r"(par_ebp), "r"(par_esp));

    // never reaches here
    // 01134 = Hello :)
    return 01134;
}

// Handles system call to execute
// Inputs: command - the command to execute
// Outputs: returns fail (-1) or 0 on success
// Effects: creates a new process and executes the command
int32_t execute(const uint8_t* command) {
    // Print a newline character and clear the buffer
    putc('\n');
    clear_buffer();

    // Save the current stack pointer (esp) and base pointer (ebp)
    uint32_t parent_esp, parent_ebp;
    asm volatile("movl %%esp, %[parent_esp];"
                 "movl %%ebp, %[parent_ebp];"
                : [parent_esp] "=m" (parent_esp), [parent_ebp] "=m" (parent_ebp)
                :
                : "memory");

    // Initialize directory entry and buffer
    dentry_t dentry;
    uint8_t buf[DATA_BLOCK_SIZE];

    // Parse the command to get the file name
    int i = 0, j = 0;
    uint8_t cur_file[MAX_NAME_LENGTH];

    while(command[i] == ' ') i++;

    while(command[i] != ' ' && command[i] != '\0') {
        if(j >= MAX_NAME_LENGTH) return 0;
        cur_file[j++] = command[i++];
    }

    cur_file[j] = '\0';

    // Check for errors and read the directory entry and data
    if(new_pid >= MAX_OPEN_PROGS - 1 || command == NULL || command == '\0' || strlen((int8_t*)command) > BUF_SIZE ||
    read_dentry_by_name((uint8_t*)cur_file, &dentry) == -1 ||
    read_data(dentry.inode_num, 0, buf, RAND_BUF_SIZE) == -1 ||
    (buf[0] != ASCII_DEL) || (buf[1] != ASCII_E) || (buf[2] != ASCII_L) || (buf[3] != ASCII_F)) return -1;

    // Update the current process ID
    old_pid = terminal_array[current_terminal].terminal_current_pid;
    for(i = 0; i < MAX_OPEN_PROGS; i++) {
        if(progs[i] == CLOSE) {
            progs[i] = OPEN;
            new_pid = i;
            break;
        }
    }
    if(i > MAX_OPEN_PROGS - 1) { return -1; }

    terminal_array[current_terminal].terminal_current_pid = new_pid;

    // Initialize the process control block (PCB) for the new process
    pcb_t *pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));
    memset(pcb->arg_buff, '\0', sizeof(pcb->arg_buff));
    strcpy((int8_t*)pcb->arg_buff, (int8_t*)command);

    // Calculate the entry point of the new process
    int eip = 0;
    for(i = 0; i < EIP_BYTES; i++) eip |= buf[EIP_OFFSET + i] << (BYTE_BITS * i);

    // Map the new process into the page directory
    page_directory[USR_IDX].present = page_directory[USR_IDX].rw = page_directory[USR_IDX].us = page_directory[USR_IDX].ps = page_directory[USR_IDX].g = 1;
    page_directory[USR_IDX].pwt = page_directory[USR_IDX].pcd = page_directory[USR_IDX].acc = page_directory[USR_IDX].avl = page_directory[USR_IDX].avl_3 = 0;
    page_directory[USR_IDX].addy = ((uint32_t)(EIGHTMB + (new_pid * FOURMB))) >> SHIFT_12;
    flush_tlb();

    // Read the file data into memory
    uint32_t file_size = (uint32_t)(((inode_t*)(inode_ptr + dentry.inode_num))->length);
    read_data((uint32_t)dentry.inode_num, 0, (uint8_t*)EXEC_LOAD_ADDRESS, file_size);

    if(new_pid < 3){
        old_pid = -1;
    }

    // Update the PCB with the new process information
    pcb->parent_pid = old_pid;
    pcb->pid = new_pid;
    pcb->parent_ebp = parent_ebp;
    pcb->parent_esp = parent_esp;

    // Initialize the file descriptors for the new process
    for(i = 0; i < 2; i++) {
        pcb->systemcall_fd_array[i].file_operation_table_ptr = get_fops_table(TERMINAL_INDEX);
        pcb->systemcall_fd_array[i].inode = -1;
        pcb->systemcall_fd_array[i].file_position = 0;
        pcb->systemcall_fd_array[i].flags = 1;
    }

    for(i = 2; i < FILE_DESCRIPTOR_ARRAY_SIZE; i++) pcb->systemcall_fd_array[i].flags = 0;

    // Update the task state segment (TSS) for the new process
    pcb->esp = tss.esp0; 
    pcb->ss = tss.ss0;   

    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHTMB - EIGHTKB * new_pid - EIP_BYTES;

    pcb->program_ebp = tss.esp0;
    pcb->program_esp = tss.esp0;

    // Prepare for context switch to the new process
    asm volatile ("leal HALT_RET, %%eax;"
                :
                :
                : "eax", "memory");
    asm volatile("pushl %[USR_DS];"
                 "pushl %[BOT];"
                 "pushfl;"
                 "popl %%ebx;"
                 "orl %[IF_EN], %%ebx;"
                 "pushl %%ebx;"
                 "pushl %[USR_CS];"
                 "pushl %[ip];" 
                : 
                : [USR_DS] "i" (USER_DS), [BOT] "i" (KEY_MEM), [IF_EN] "i" (FLAG_MASK), [USR_CS] "i" (USER_CS), [ip] "r" (eip) 
                : "ebx");
    
    // Perform the context switch
    asm volatile("iret");
    asm volatile("HALT_RET:");

    return 0;
}

// handles system call to read
// Inputs: fd - file to read from
//         buf - buffer to read from
//         nbytes - number of bytes to read from buffer
// Outputs: returns fail (-1) or the bytes read from buffer
// Effects: calls file system functions
int32_t read( int32_t fd, void* buf, int32_t nbytes )
{
    // retrieve pointer to current pcb
    pcb_t * pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));

    // make sure the given fd is valid
    if(fd > MAX_FD || fd < MIN_FD || fd == 1 || buf == NULL || pcb->systemcall_fd_array[fd].flags == 0) { return -1; }

    // synchronize the global file descriptor array with the current program's array
    int i;
    for(i = 0; i < FILE_DESCRIPTOR_ARRAY_SIZE; i++) {
        file_descriptor_array[i] = pcb->systemcall_fd_array[i];
    }

    // try to call read function
    int retval = (pcb->systemcall_fd_array[fd].file_operation_table_ptr->read)(fd, buf, nbytes);

    // restore the old file array information
    // system's and program's file information will be different, especially after calling a read function
    // we have to make sure the changes are stored for the current program (since the call to read only updates the system's file information)
    for(i = 0; i < FILE_DESCRIPTOR_ARRAY_SIZE; i++) {
        pcb->systemcall_fd_array[i] = file_descriptor_array[i];
    }

    return retval; 
}


// handles system call to write
// Inputs: fd - file to write with
//         buf - buffer to write with
//         nbytes - number of bytes to write from buffer
// Outputs: returns fail (-1) or the bytes written from buffer
// Effects: calls file system functions
int32_t write( int32_t fd, const void* buf, int32_t nbytes )
{
    // retrieve pointer to current pcb
    pcb_t * pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));

    // make sure the given fd is valid
    if(fd > MAX_FD || fd <= MIN_FD || buf == NULL || pcb->systemcall_fd_array[fd].flags == 0) { return -1; }

    // try to call write function
    return (pcb->systemcall_fd_array[ fd ].file_operation_table_ptr->write)( fd, buf, nbytes ); 
}

// handles system call to open files
// Inputs: filename - name of file to open
// Outputs: returns fail (-1) or the index in file descritpor array of the file
// Effects: calls file system functions
int32_t open( const uint8_t* filename )
{
    dentry_t cur_dentry;
    // try to retrieve directory entry of file
    if( read_dentry_by_name( filename, &cur_dentry ) == -1 ) { return -1; }

    // retrieve pointer to current pcb
    pcb_t * pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));

    // try to get a free index in file descriptor array
    int i;
    for( i = 0; i < FILE_DESCRIPTOR_ARRAY_SIZE; i++ )
    {
        if( pcb->systemcall_fd_array[i].flags == CLOSE ) { break; }
    }
    // if there was no space in file descriptor array, fail 
    if( i >= FILE_DESCRIPTOR_ARRAY_SIZE ) { return -1; }

    // set members of our current pcb
    pcb->systemcall_fd_array[i].inode = cur_dentry.inode_num;
    pcb->systemcall_fd_array[i].file_operation_table_ptr = get_fops_table(cur_dentry.filetype);
    // set file to open
    pcb->systemcall_fd_array[i].flags = 1;
    pcb->systemcall_fd_array[i].file_position = 0;

    // try to open file, if cannot open, fail
    if((pcb->systemcall_fd_array[i].file_operation_table_ptr->open)(filename) == -1){ return -1; }

    return ( i );
}


// handles system call close
// Inputs: fd - file descriptor to close
// Outputs: return success (0) or fail (-1)
// Effects: accesses and change svalues in systemcall_fd_array
int32_t close (int32_t fd) {
    if (fd < (MIN_FD + 2) || fd > MAX_FD) {  // check if the fd is within bounds, cannot close stdin or stdout, so MIN_FD+2 is our minimum index
        return -1;
    }
    // retrieve pointer to current pcb
    pcb_t * pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));
    
    // check if file is currently closed
    if (pcb->systemcall_fd_array[fd].flags == 0) { return -1; }
    // if not closed, close the file
    pcb -> systemcall_fd_array[fd].flags = CLOSE;
    return (pcb->systemcall_fd_array[ fd ].file_operation_table_ptr->close)(fd);
}

// not necessary for checkpoint 3.3, but implemented to run tests
// Inputs: buf - buffer that will contains the arguments
//         nbytes - number of bytes to copy to buf
// Outputs: returns success (0) or fail (-1)
// Effects: clobbers anything preexisting in buf
int32_t getargs( uint8_t* buf, int32_t nbytes )
{
    // check for valid inputs
    if (buf == NULL || nbytes < 1){ return -1; }

    pcb_t * pcb = (pcb_t *)(EIGHTMB - EIGHTKB * (new_pid + 1));

    uint8_t command_buffer[BUF_SIZE];
    strcpy((int8_t*)command_buffer, (int8_t*)pcb->arg_buff);

    int i = 0;
    // flags for spaces in between "words" in the command
    int flag1 = 0;
    int flag2 = 0;
    while((command_buffer[i] != '\0')){
        if (command_buffer[i] != ' '){
            flag1 = 1;
        }
        if((command_buffer[i] == ' ') && (flag1 == 1)){
            flag2 = 1;
        }
        if((command_buffer[i] != ' ') && (flag2 == 1)){
            break;
        }
        i++;
    }
    // by this point i should store the start of args
    uint8_t args[BUF_SIZE];    
    memset( args, '\0', BUF_SIZE );

    int arg_offset = i;
    while (command_buffer[i] != '\0'){
        args[i - arg_offset] = command_buffer[i];
        i++;
    }

    // fail if args are too long or no args were found
    if (!(strlen((int8_t*)args) <= nbytes) || (strlen((int8_t*)args) < 1)){
        return -1;
    }
    memcpy(buf, args, nbytes);

    // if memcpy was successful, return success
    return 0; 
}   


// allocates a 4KB page in virtual memory for video memory
// sets screen_start to the start of this memory
// Inputs: screen_start - pointer to set later on
// Outputs: returns success (0) or fails (-1)
// Effects: accesses paging information and changes *screen_start
int32_t vidmap (uint8_t** screen_start){
    // check valid location
    // should be within user space
    if(screen_start == NULL || (uint32_t)screen_start < USER_START || (uint32_t)screen_start >= KEY_MEM){
        return -1;
    }
    // initialize new page directory entry for video memory
    page_directory[VID_IDX].present = page_directory[VID_IDX].rw = page_directory[VID_IDX].us = 1;
    // size of the page will be 4KB, not 4MB
    page_directory[VID_IDX].ps = 0;
    page_directory[VID_IDX].addy = ((uint32_t)vid_table >> SHIFT_12);

    // set up page table entry at correct location
    vid_table[0].present = vid_table[0].rw = vid_table[0].us = 1;
    vid_table[0].addy = VIDEO_START >> SHIFT_12;
    

    // set location of user video memory
    *screen_start = (uint8_t *)(USER_VID_MEM);

    // new page loaded, so we have to flush the tlb
    flush_tlb();

    return 0;
}

// returns failure since we don't have extra credit implemented yet
int32_t set_handler (int32_t signum, void* handler_address){
    return -1;
}

// returns failure since we don't have extra credit implemented yet
int32_t sigreturn (void){
    return -1;
}

// Initializes our fops table
// Inputs: none
// Outputs: none
// Effects: sets all file operation functions for different types of files in fops_table
void init_fops_tables() {
    fops_table[RTC_INDEX].open = rtc_open;
    fops_table[RTC_INDEX].read = rtc_read;
    fops_table[RTC_INDEX].write = rtc_write;
    fops_table[RTC_INDEX].close = rtc_close;

    fops_table[DIR_INDEX].open = dir_open;
    fops_table[DIR_INDEX].read = dir_read;
    fops_table[DIR_INDEX].write = dir_write;
    fops_table[DIR_INDEX].close = dir_close;

    fops_table[FILE_INDEX].open = file_open;
    fops_table[FILE_INDEX].read = file_read;
    fops_table[FILE_INDEX].write = file_write;
    fops_table[FILE_INDEX].close = file_close;

    fops_table[TERMINAL_INDEX].open = terminal_open;
    fops_table[TERMINAL_INDEX].read = terminal_read;
    fops_table[TERMINAL_INDEX].write = terminal_write;
    fops_table[TERMINAL_INDEX].close = terminal_close;
}

// Get the file operations table for a specific device
// Inputs: device_index - index for the type of device/file
// Outputs: file operations table corresponding to given index
// Effects: accesses fops_table
fops_table_t* get_fops_table(int device_index) {
    if (device_index < 0 || device_index >= NUM_DEVICES) {
        return NULL; // invalid
    }
    
    return &fops_table[device_index];
}
