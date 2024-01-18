#ifndef _SCHEDULE_H
#define _SCHEDULE_H
#include "lib.h"

#define COUNT 23862 //  23862 will give us 20ms or 50Hz
#define CHANNEL_0_DATA_PORT 0x40
#define COMMAND_REG 0x43
#define CHANNEL_MODE 0x36 // 0011 0110 selects channel 0 and mode 3 for square wave generator

// initializing programmable interval timer
void PIT_init( void );
// handling programmable interval timer interrupts for scheduling
void pit_handler( void );

#endif
