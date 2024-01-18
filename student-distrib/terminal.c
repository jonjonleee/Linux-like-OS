#include "terminal.h"

volatile uint32_t read_flag = 0;
uint8_t terminal_buffer[BUF_SIZE];
int32_t current_terminal, sched_terminal;

/* int32_t terminal_open( const uint8_t* filename )
 *   Inputs: uint8_t* filename - not used
 *   Return Value: None
 *   Effects: None */
int32_t terminal_open( const uint8_t* filename ) {
    return 0;
}

/* int32_t terminal_close( int32_t fd )
 *   Inputs: int32_t fd - not used
 *   Return Value: None
 *   Effects: Clears keyboard buffer
 *            Doesn't do anything else */
int32_t terminal_close( int32_t fd ) {
    clear_buffer();
    return -1;
}

/* extern int32_t terminal_read( int32_t fd, void* buf, int32_t nbytes )
 *   Inputs: int32_t fd - not used
             void* buf  - buffer to read from
             int32_t nbytes - number of bytes to read from buffer
 *   Return Value: Number of bytes successfully read
 *   Effects: Copies keyboard buffer into terminal buffer
 *            It will need functionality from kb.h and kb.c */
int32_t terminal_read( int32_t fd, void* buf, int32_t nbytes ){
    // just ensure the read_flag wasn't set improperly before
    read_flag = 0;

    // used to count how many bytes read so far
    int ret_count = 0;

    // wait for keyboard to detect an ENTER ('\n')
    // then proceed with read
    while(!read_flag){}

    // reset read flag
    read_flag = 0;
    // check if there's nothing to read from
    if(buf == NULL) {return 0;}

    // have to typecast from void*
    uint8_t * temp_buf = buf;
    

    //initialize the buffer before we read from keyboard
    int i;
    for(i = 0; i < BUF_SIZE; i++){
        temp_buf[i] = 0;
    }
    for(i = 0; i < count[current_terminal]; i++){
        temp_buf[i] = kb_buffer[current_terminal][i];
        ret_count++;
        // break if we there is an ENTER
        if(kb_buffer[current_terminal][i] == '\n'){ break; }
    }
    // clear keyboard buffer for next use
    clear_buffer();
    return ret_count;
}

/* extern int32_t terminal_write( int32_t fd, const void* buf, int32_t nbytes )
 *   Inputs: int32_t fd - not used
             void* buf  - buffer to write to screen
             int32_t nbytes - number of bytes to print
 *   Return Value: Number of bytes successfully printed
                   Will return -1 if it wasn't able to print
 *   Effects: Prints the elements stored in buffer onto the terminal
 *            It will need functionality from kb.h and kb.c */
int32_t terminal_write( int32_t fd, const void* buf, int32_t nbytes ){
    // don't try to write if buf is NULL poiner
    if(buf == NULL){ return -1; }

    // uncomment to limit terminal write to only 128 characters
    // if(nbytes > BUF_SIZE){
    //     nbytes = BUF_SIZE;
    // }
    // used to count how many bytes read so far

    int ret_count = 0;

    // have to typecast from void*
    const uint8_t * temp_buf = buf;

    // print chars to screen
    int i;
    for(i = 0; i < nbytes; i++){
        putc(temp_buf[i]);
        ret_count++;
    }
    return ret_count;
}

// initialize our three terminal structs
// loads basic values for our terminal values
// Inputs: none
// Outputs: none
void term_init(){
    int i;
    for(i = 0; i < TERMINAL_COUNT; i++){
        terminal_array[i].terminal_current_pid = -1;
        terminal_array[i].on_off_flag = terminal_array[i].terminal_esp = terminal_array[i].terminal_ebp = 0;
    }
    current_terminal = 0;
    sched_terminal = 0;
}

// opens a particular terminal with given id
// Inputs: terminal_id - terminal to open
// Outputs: none
void open_terminal(int32_t terminal_id){
    // check for valid terminal_id
    // we don't need to do anything if the terminal is already open
    if(terminal_id >= TERMINAL_COUNT || terminal_id < 0 || terminal_id == current_terminal){
        return;
    }

    // store current screen of terminal
    memcpy((void*)(VIDEO_PAGES) + current_terminal * VID_MEM, (void*) VIDEO_START, VID_MEM);
    // change terminal to display
    current_terminal = terminal_id;
    // restore screen of correct terminal
    memcpy((void*)(VIDEO_START), (void*)(VIDEO_PAGES) + current_terminal * VID_MEM, VID_MEM);

    // set cursor to the correct position in current terminal
    update_cursor(screen_x[current_terminal], screen_y[current_terminal]);
}
