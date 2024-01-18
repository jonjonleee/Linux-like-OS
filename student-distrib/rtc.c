// implementation of RTC
// most code from https://wiki.osdev.org/RTC

#include "rtc.h"


volatile int rtc_interrupt_received;

// initialization function for RTC
// sends interrupt request to PIC
// Inputs: none
// Outputs: none
// Effects: initializes rtc
void rtc_init(void){
    // link RTC to PIC
    enable_irq(RTC_IRQ);

    // select register B, and disable NMI
    outb(NMI_MASK | RegB, CMOS_PORT);

    // read the current value of register B
    char prev = inb(CMOS_DATA_PORT);

    // set the index again (a read will reset the index to register D)
    outb(NMI_MASK | RegB, CMOS_PORT);

    // write the previous value ORed with 0x40. This turns on bit 6 of register B
    outb(prev | bit6, CMOS_DATA_PORT);


    // sets it to 2Hz
    outb(STATUS_REG_A, CMOS_PORT);		// set index to register A, disable NMI
    prev=inb(CMOS_DATA_PORT);	// get initial value of register A
    outb(STATUS_REG_A, CMOS_PORT);		// reset index to A
    outb((prev & 0xF0) | 0x0F, CMOS_DATA_PORT); //write only our rate to A. Note, rate is the bottom 4 bits.         
    enable_irq(RTC_IRQ);
}


// handles interrupt for keyboard
// interacts with linkage/wrapper function
// Inputs: none
// Outputs: none
// Effects: for CP1, uncomment test_interrupts() for testing RTC
void rtc_irq_handler(void){
    // stop interrupts while handling this interrupt
    cli();

    // select register C
    outb(RegC, CMOS_PORT);
    // get rid of contents
    inb(CMOS_DATA_PORT);

    // comment/uncomment below for RTC testing
    // test_interrupts();

    send_eoi(RTC_IRQ);
    rtc_interrupt_received = 1;

    // enable interrupts at this point
    sti();
    return;
}

// Function: rtc_open
// Description: Initializes RTC frequency to 2Hz.
// Inputs: filename
// Outputs: Returns 0 on success, -1 if filename is NULL.
// Effects: Sets RTC frequency to 2Hz and enables RTC IRQ.
int32_t rtc_open(const uint8_t* filename){
    if(filename == NULL){
        return -1;
    }   
    outb(STATUS_REG_A, CMOS_PORT);		// set index to register A, disable NMI
    char prev=inb(CMOS_DATA_PORT);	// get initial value of register A
    outb(STATUS_REG_A, CMOS_PORT);		// reset index to A
    outb((prev & 0xF0) | 0x0F, CMOS_DATA_PORT); //write only our rate to A. Note, rate is the bottom 4 bits. 0x0F and 0xF0 is for masking        
    enable_irq(RTC_IRQ);                   
    return 0;
}

// Function: rtc_close
// Inputs: fd
// Outputs: Returns 0 as it currently does nothing.
// Effects: None unless RTC is virtualized.
int32_t rtc_close(int32_t fd){
    return 0;
}

// Function: rtc_read
// Inputs: fd, buf, nbytes
// Outputs: Returns 0 after an RTC interrupt is received.
// Effects: This function blocks the execution until an RTC interrupt is received.
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    // Wait until an RTC interrupt is received
    while(!rtc_interrupt_received);
    // Reset the interrupt flag
    rtc_interrupt_received = 0;
    // Return success
    return 0;
}

// Function: rtc_write
// Description: Changes the frequency of RTC interrupts.
// Inputs: fd, buf, nbytes
// Outputs: Returns 0 on success, -1 on failure.
// Effects: Changes the interrupt rate of the RTC.
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    // Check for null pointer and correct buffer size
    if (buf == NULL || nbytes != sizeof(uint32_t)) {
        return -1;
    }
    uint32_t rate = *(uint32_t*)buf;
    // Check if rate is a power of 2
    int exponent_2 = 0;
    int index = 0;
    int i;
    for (i = 0; i < 32; i++){ // counting set bits and finding the index of the highest set bit
        if ( (rate >> i) & 1){
            exponent_2++;
            index = i;
        }
    }
    index--; 
    // Check if rate is within valid range and is a power of 2
    if (rate < 2 || rate > 1024 || exponent_2 != 1){
        return -1;
    }
    // Array of possible frequency values corresponding to rates from 2 Hz to 1024 Hz
    char freq_values[10] = {0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06}; //, 0x06
    char freq = freq_values[index];    // Get the correct value to set RS bits in Register A

    cli();    // Block interrupts when writing to RTC
    outb(STATUS_REG_A, CMOS_PORT);    // Select register A and disable NMI
    char prev = inb(CMOS_DATA_PORT) & 0xF0;    // Get the contents of register A and clear the bottom 4 bits
    outb(STATUS_REG_A, CMOS_PORT);    // Select register A again
    outb(prev | freq, CMOS_DATA_PORT);    // Write the new frequency value to the bottom 4 bits of register A
    sti();    // Allow interrupts again
    return 0;
}

