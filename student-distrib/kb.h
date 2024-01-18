// keyboard header file
#ifndef _KB_H
#define _KB_H

#include "types.h"
#include "i8259.h"
#include "terminal.h"
#include "systemcall.h"
#include "lib.h"

#define KEYBOARD_IRQ   1
#define TERMINAL_COUNT 3

//keyboard ports
#define KEYBOARD_PORT_DATA       0x60
#define KEYBOARD_PORT_CMD        0x64

//keyboard constants
#define ESCAPE 0x01
#define BACKSPACE 0x0E
#define CTRL 0x1D
#define CTRL_RELEASE 0x9D
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_RELEASE 0xB6
#define ALT 0x38
#define ALT_RELEASE 0xB8
#define CAPS 0x3A
#define KEYPAD_STAR 0x37

#define F1_CODE 0x3B
#define F2_CODE 0x3C
#define F3_CODE 0x3D


#define BUF_SIZE 128
#define TAB_SIZE 5
#define USER_HALT 42

uint8_t caps_flag;
uint8_t shift_flag;
uint8_t ctrl_flag;
uint8_t alt_flag;

// keyboard buffer, should allow only 127 characters to be shown on screen, 128th character saved for ENTER
// count variable used to count current index in the array

extern uint8_t kb_buffer[TERMINAL_COUNT][BUF_SIZE];
extern uint32_t count[TERMINAL_COUNT];


// initialization function for keyboard
extern void keyboard_init(void);

// handles keyboard interrupt
// used in wrapper/linkage function
extern void keyboard_irq_handler(void);

// sets all values in buffer to 0
// resets count to 0
extern void clear_buffer();

#endif /* _KB_H */
