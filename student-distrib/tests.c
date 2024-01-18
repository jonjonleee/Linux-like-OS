#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "kb.h"
#include "terminal.h"
#include "file_sys.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

// all purpose test to check exception handlers
// change the value inside asm volatile() to respective irq vector number
// Input: None
// Output: None
// Effect: should call the correct exception handler and print the respective message to screen
int exception_test(){
	TEST_HEADER;
	asm volatile("int $5");
	return 1;
}

// calls test for system call interrupt
// Input: None
// Output: None
// Effect: calls interrupt handler for system call irq 0x80
//			prints the respective message to screen
int sys_call_test(){
	TEST_HEADER;
	asm volatile("int $0x80");
	return 1;
}


// calls test for divide by zero interrupt
// Input: None
// Output: None
// Effect: calls interrupt handler for divide by zero exception
//			prints the respective message to screen
int exception_divide_by_zero_test(){
	TEST_HEADER;
	int a = 1;
	int b = 0;
	return a/b;
}

/*
 * Function: page_correct_addy
 * Description: Tests if accessing memory at 0xB8000 causes a page fault.
 * Input: None
 * Output: Returns PASS if no page fault occurs, otherwise undefined.
 * Side Effects: Modifies the value at memory address 0xB8000.
 */
int page_correct_addy(){
    TEST_HEADER;
    volatile int *a = (int *)0xB8000;
    int b = *a;
    b++;
    printf("%d\n", *a);
    return PASS;
}

/*
 * Function: page_incorrect_addy
 * Description: Tests if accessing memory at 0xBA000 causes a page fault.
 * Input: None
 * Output: Returns FAIL if a page fault occurs, otherwise undefined.
 * Side Effects: May cause a page fault and terminate the program.
 */
int page_incorrect_addy(){
    TEST_HEADER;
    volatile int *a = (int *)0xB9000;
    int b = *a;
    b++;
    printf("%d\n", *a);
    return FAIL;
}

// for RTC testing, go to rtc.c
// within the rtc_handler uncomment test_interrupts()

/* Checkpoint 2 tests */

// Function: test_terminal_all
// Description: Tests the terminal_read and terminal_write. Asks user to input by keyboard to test terminal_read, outputs the input to test terminal_write
// Inputs: None
// Outputs: None
// Effects: Calls terminal read/write which will utilize keyboard buffer
void test_terminal_all(){
	uint8_t buffer[BUF_SIZE];
	int read_bytes, write_bytes;

	TEST_HEADER;

	terminal_open(NULL);

	while (1)
	{
		// take keyboard input
		read_bytes = terminal_read(0, (void*) buffer, BUF_SIZE);		

		// print buffer to screen
		write_bytes = terminal_write(0, (void*) buffer, read_bytes);
		
		// make sure the # of bytes for outputs are correct
		printf("\nread_bytes: %d\twrite_bytes: %d\n",read_bytes, write_bytes);
	}

	terminal_close(NULL);
	clear_buffer();
}

// Function: test_terminal_open
// Description: Tests the test_terminal_open
// Inputs: None
// Outputs: None
// Effects: Uses keyboard buffer
int test_terminal_open(){
	const uint8_t* filename = NULL; 
	if(terminal_open(filename) == 0){
		return PASS;
	}
	return FAIL;
}


// Function: test_terminal_write
// Description: Tests the terminal_write for bytes not equal to buffer size
// Inputs: None
// Outputs: None
// Effects: Uses keyboard buffer
void test_terminal_write(){
	uint8_t buffer[BUF_SIZE];
	int read_bytes, write_bytes;

	// take keyboard input
	read_bytes = terminal_read(0, (void*) buffer, BUF_SIZE);		

	// print buffer to screen
	write_bytes = terminal_write(0, (void*) buffer, read_bytes - 3);
	
	// make sure the # of bytes for outputs are correct
	printf("\nread_bytes: %d\twrite_bytes: %d\n",read_bytes, write_bytes);
	clear_buffer();
}

// Function: test_terminal_close
// Description: Tests the terminal_close
// Inputs: None
// Outputs: None
// Effects: Uses keyboard buffer
int test_terminal_close(){
	int32_t fd = 0; 
	if(terminal_close(fd) == 0){
		return PASS;
	}
	return FAIL;
}


// Function: rtc_test
// Description: Tests the rtc_read and rtc_write by changing the RTC frequency from 2Hz to 1024Hz in powers of 2 and printing '1' for each interrupt received.
// Inputs: None
// Outputs: None
// Effects: Changes RTC frequency and prints '1' for each interrupt received.
void rtc_test() {
    int32_t fd = rtc_open((uint8_t*)"RTC");
    int32_t rate = 2;
    int32_t freq;
    int32_t ret_val;

    while (rate <= 1024) {
        ret_val = rtc_write(fd, &rate, sizeof(uint32_t));
        if (ret_val == -1) {
            printf("Error: Invalid rate value.\n");
            return;
        }
		printf("\n Current rate: %d Hz. \n", rate);
        for (freq = 0; freq < rate; freq++) {
            rtc_read(fd, NULL, 0);
            printf("1");
        }

        rate *= 2;
    }
}

// RTC test for open
// Description: This function will print 1 indefinetely at the rate of 2Hz
// Input: none
// Iutput: none
void rtc_test2(){
    int32_t fd = rtc_open((uint8_t*)"RTC");
    // Opening sets the frequency to 2Hz
    while(1){
        rtc_read(fd, NULL, 0);
        // Print '1' for each interrupt received
        printf("1");    
    }
}

// RTC test for close
// Description: This function will return PASS when rtc_close performs successfully
// Input: none
// Output: PASS or FAIL
int rtc_test3(){
    TEST_HEADER;
    int32_t fd = rtc_open((uint8_t*)"RTC");
    int close = rtc_close(fd);
    // successfully closing returns 0;
    if(close == 0){
        return PASS;
    } else {return FAIL;}
}

// helper function to print strings
// Input: buf - buffer to print to screen
// Output: none
// Effects: prints to screen
void print_str(uint8_t* buf) {
    unsigned int i;
    for (i = 0; i < strlen((const int8_t*) buf); i++) {
        putc(buf[i]);
    }
    return;
}

// Test case for opening large file with very long name
// Input: none
// Output: returns pass (1) or fail (-1)
int test_print_large_text () {
    uint8_t test[] = "verylargetextwithverylongname.txt";
	// make buffer really large so it stores everything inside file
    uint8_t buf[DATA_BLOCK_SIZE*3];
    uint32_t nbytes = DATA_BLOCK_SIZE*3;
    uint32_t bytes_read = 0;

    uint32_t fd = file_open((const uint8_t*) test);
    if (fd == -1) {
        return FAIL;
    }

    bytes_read = file_read(fd, buf, nbytes);
    print_str(buf);

    file_close(fd);
    return PASS;

}

// Test case for opening small file
// Input: none
// Output: returns pass (1) or fail (-1)
int test_print_frame0_text () {
    uint8_t test[] = "frame0.txt";
	// make buffer really large so it stores everything inside file
    uint8_t buf[DATA_BLOCK_SIZE*3];
    uint32_t nbytes = DATA_BLOCK_SIZE*3;
    uint32_t bytes_read = 0;

    uint32_t fd = file_open((const uint8_t*) test);
    if (fd == -1) {
        return FAIL;
    }

    bytes_read = file_read(fd, buf, nbytes);
    print_str(buf);

    file_close(fd);
    return PASS;

}

// Function: test_print_executable
// Description: returns PASS if "shell" was successfully opened, read, and printed
// Inputs: None
// Outputs: None
// Effects: opens a file with the name of "shell" and prints its content on the screen
int test_print_executable () {
	TEST_HEADER;
    uint8_t test[] = "shell"; // load the executable file

    // init variables to be used
    uint8_t buf[DATA_BLOCK_SIZE*3];
    uint32_t nbytes = 128;
    uint32_t nbytes_read = 0;
    uint32_t fd = file_open((const uint8_t*) test);

    // attempt to open the file, return fail if unsuccessful
    if (fd == -1) {
        return FAIL;
    }
    // store the number of bytes read
    nbytes_read = file_read(fd, buf, nbytes);

    // while there are bytes to read
    while (nbytes_read > 0) {
        // printf("%d", nbytes_read);
        // write the contents of the buf to the terminal
        terminal_write(0, buf, nbytes_read);
        // read bytes agian
        nbytes_read = file_read(fd, buf, nbytes);
    }
	putc('\n');
    // close the file
    file_close(fd);
    return PASS;

}

// directory read
// performs function as ls
// Input: none
// Output: returns PASS
// Effects: file directory prints to screen
void test_directory_read () {  
    uint8_t buf[MAX_NAME_LENGTH];
	// only directory in our file system is called '.'
    char dir[] = ".";
    int32_t test_fd = 0;

	// open directory
    clear_buffer();
    dir_open((const uint8_t*) dir);

	dentry_t cur_dentry;
    inode_t cur_inode;  

	// print out each file in directory one at a time
    while(dir_read(test_fd, buf, MAX_NAME_LENGTH) > 0) {
        printf("file_name: ");
        terminal_write(0, buf, MAX_NAME_LENGTH);
        read_dentry_by_name(buf, &cur_dentry);
        cur_inode = *((inode_t*)(inode_ptr + (cur_dentry.inode_num)));
        printf(",  file_type: %d, file_size: %d", cur_dentry.filetype, cur_inode.length);
        printf("\n");
    }

    dir_close(test_fd);
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	clear();
	clear_buffer();
	// checkpoint 1 tests________________________________________________________________________________

	//exception_test();
	//sys_call_test();
	//exception_divide_by_zero_test();
	//page_correct_addy();
	//page_incorrect_addy();

	//checkpoint 2 tests________________________________________________________________________________
	// test_terminal_all();
	// TEST_OUTPUT("terminal_open", test_terminal_open());
	// test_terminal_write();
	// TEST_OUTPUT("terminal_close", test_terminal_close());
	//rtc_test();
	// rtc_test2();
	//TEST_OUTPUT("rtc_test3", rtc_test3());
	// TEST_OUTPUT("test_print_large_text", test_print_large_text());
	// TEST_OUTPUT("test_print_frame0_text", test_print_frame0_text());
	// TEST_OUTPUT("test_print_executable", test_print_executable());
	// test_directory_read();
}
