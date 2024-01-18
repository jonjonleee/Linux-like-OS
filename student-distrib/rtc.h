// keyboard header file
#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "i8259.h"
#include "lib.h"

#define RTC_IRQ 8

//CMOS ports
#define CMOS_PORT 0x70
#define CMOS_DATA_PORT 0x71
#define STATUS_REG_A 0x8A

#define bit6 0x40

//Used to disable/enable NMI
#define NMI_MASK 0x80

//Regs
#define RegA 0xA
#define RegB 0xB
#define RegC 0xC

// initialization function for RTC
void rtc_init(void);

// handles RTC interrupt
// used in wrapper/linkage function
void rtc_irq_handler(void);

// helper functions for system call functionality
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);


// function declaration to prevent warnings
extern void test_interrupts(void);

#endif /* _RTC_H */
