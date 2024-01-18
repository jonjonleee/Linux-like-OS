#ifndef _PAGING_H
#define _PAGING_H

// Define the number of pages and the size of each page
#define ENTRIES     1024
#define SHIFT_12   12
#define SHIFT_22   22

// Define the start addresses for video memory, kernel, and user
#define VIDEO_START    0xB8000
#define VIDEO_PAGES    0xBA000
#define TERMINAL_PAGES 3
#define KERNEL_START   0x400000

// info for structures from https://wiki.osdev.org/Paging
// Define the structure for a page directory entry
typedef struct __attribute__((packed)) page_directory_entry_t {
    unsigned int present : 1;   // Page present in memory
    unsigned int rw : 1;   // Read-write flag
    unsigned int us : 1;   // User-supervisor flag
    unsigned int pwt : 1;   // Page-level write-through flag
    unsigned int pcd : 1;   // Page-level cache disable flag
    unsigned int acc : 1;   // Accessed flag
    unsigned int avl : 1;   // Available for software use
    unsigned int ps : 1;    // Page size flag
    unsigned int g : 1;   // Global page flag
    unsigned int avl_3 : 3;   // Available for software use
    unsigned int addy : 20;  // Physical address of the page frame in memory
} page_directory_entry_t;

// Define the structure for a page table entry
typedef struct __attribute__((packed)) page_table_entry_t {
    unsigned int present : 1;    // Page present in memory
    unsigned int rw : 1;    // Read-write flag
    unsigned int us : 1;    // User-supervisor flag
    unsigned int pwt : 1;    // Page-level write-through flag
    unsigned int pcd : 1;    // Page-level cache disable flag
    unsigned int acc : 1;    // Accessed flag
    unsigned int dirty : 1;     // Dirty flag (has the page been written to)
    unsigned int pat : 1;    // Page attribute table index (set to zero)
    unsigned int g : 1;    // Global page flag (ignored)
    unsigned int avl_3 : 3;    // Available for software use 
    unsigned int addy : 20;   // Physical address of the page frame in memory 
} page_table_entry_t;

// Declare the page directory and page tables, aligning them to a boundary of 4096 bytes (the size of a page)
page_directory_entry_t page_directory[ENTRIES] __attribute__((aligned(4096)));
page_table_entry_t page_table[ENTRIES] __attribute__((aligned(4096))); 
page_table_entry_t vid_table[ENTRIES] __attribute__((aligned(4096))); 

// Function prototypes for initializing paging, loading the page directory, enabling paging, and flushing the TLB.
extern void page_init();
extern void enabling(unsigned int *page_directory);
extern void flush_tlb();

#endif /* PAGING_H */

