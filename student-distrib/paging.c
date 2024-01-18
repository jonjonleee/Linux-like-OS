#include "lib.h"
#include "paging.h"
#include "types.h"


/*
 * Function: Initializes the page directory and page table for a paging system.
 * No input parameters.
 * This function does not return a value.
 * Side effect: Modifies global variables page_directory and page_table. Changes the state of the memory by enabling paging.
 */
void page_init() {    
    unsigned int i; // Variable to iterate through the pages
    
    // Loop through all the pages
    for(i = 0; i < ENTRIES; i++)
    {
        // Initialize each entry in the page directory
        page_directory[i].present = 0; // Page is not present in memory
        page_directory[i].rw = 1; // Page is read/write
        page_directory[i].us = 0; // Page is kernel mode
        page_directory[i].pwt = 0; // Write through caching is disabled
        page_directory[i].pcd = 0; // Cache is enabled
        page_directory[i].acc = 0; // Page has not been accessed since last refresh
        page_directory[i].avl = 0; // For software use, set to 0
        page_directory[i].ps = 1; // Pages are 4MB in size
        page_directory[i].g = 1; // Page level cache disable bit
        page_directory[i].avl_3 = 0; // For software use, set to 0

        // Initialize each entry in the page table
        page_table[i].present = 0; // Page is not present in memory
        page_table[i].rw = 1; // Page is read/write
        page_table[i].us = 0; // Page is kernel mode
        page_table[i].pwt = 0; // Write through caching is disabled
        page_table[i].pcd = 0; // Cache is enabled
        page_table[i].acc = 0; // Page has not been accessed since last refresh
        page_table[i].dirty = 0; // Page has not been written to since last refresh
        page_table[i].pat = 0; // Page attribute table index (set to zero)
        page_table[i].g = 0; // Global flag (ignored)
        page_table[i].avl_3 = 0; // For software use, set to 0
        page_table[i].addy = i; // Address of the physical frame

        if (i << SHIFT_12 == VIDEO_START) {
            page_table[i].present = 1; // If this is the video memory address, mark it as present in memory
        }

        if (i == 0) {
            page_directory[i].present         = 1; // The first entry of the directory is present in memory
            page_directory[i].ps       = 0; // Pages are not large pages (4KB)
            page_directory[i].addy = ( (uint32_t) page_table ) >> SHIFT_12;
            /* The address of the physical frame is the address of the 
               corresponding entry in the table shifted right by 
               12 bits */
        } 
       
        else if (i == 1) {
            page_directory[i].present         = 1;
            /* The second entry of the directory is present in memory and 
               its address is the kernel start address shifted right by 
               12 bits */
            page_directory[i].addy = ( (uint32_t) KERNEL_START ) >> SHIFT_12;
        } 
    }
    
    // setup pages for terminal video memory
    for(i = VIDEO_PAGES >> SHIFT_12; i < ((VIDEO_PAGES >> SHIFT_12) + TERMINAL_PAGES); i++){
        page_table[i].present = 1; // Page is present in memory
        page_table[i].rw = 1; // Page is read/write
        page_table[i].us = 1; // Page is user mode
        page_table[i].pwt = 0; // Write through caching is disabled
        page_table[i].pcd = 0; // Cache is enabled
        page_table[i].acc = 0; // Page has not been accessed since last refresh
        page_table[i].dirty = 0; // Page has not been written to since last refresh
        page_table[i].pat = 0; // Page attribute table index (set to zero)
        page_table[i].g = 0; // Global flag (ignored)
        page_table[i].avl_3 = 0; // For software use, set to 0
        page_table[i].addy = i; // Address of the physical frame
    }
    enabling((unsigned int*) page_directory); // Load the directory into CR3 register
}

/*
 * Function: enabling
 * Description: Loads the page directory into the CR3 register and enables paging by setting the PG and PE bits of the CR0 register and PSE (4 MiB pages) by setting PSE bit of CR4 register.
 * Input: Pointer to the page directory.
 * Output: None
 * Side Effects: Modifies the CR0, CR3, and CR4 registers.
 */
void enabling(unsigned int *page_directory) {
    // Load the page directory into the CR3 register
    asm volatile 
    (
        "mov %0, %%eax    ;"
        "mov %%eax, %%cr3 ;"
        :
        : "r"(page_directory)
        : "%eax"
    );
    // Enable PSE (4 MiB pages)
    asm volatile 
    (
        "mov %%cr4, %%eax           ;" 
        "or $0x00000010, %%eax      ;"  
        "mov %%eax, %%cr4           ;"           
        :
        : 
        : "%eax"
    );
    // Enable paging by setting the PG and PE bits of the CR0 register
    asm volatile
    (
        "mov %%cr0, %%eax           ;"    
        "or $0x80000001, %%eax      ;"  
        "mov %%eax, %%cr0           ;" 
        :
        : 
        : "%eax"
    );    
}


// TLB stands for Translation Lookaside Buffer
// Whenever we move between pages (namely between kernel and user)
// We must flush TLB
// Algorithm found on OSDev
// https://wiki.osdev.org/TLB 
// Inputs: None
// Outputs: None
// Effects: Clears TLB
void flush_tlb()
{
    asm volatile( "movl %%cr3, %%eax;" 
                  "movl %%eax, %%cr3;"
                : // no outputs
                : // no inputs
                : "memory", "cc"  // clobbered
                );
}
