#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "types.h"

#define R_D_SIZE      24
#define R_B_SIZE      52
#define DIRENTRIES_SIZE     63
#define NUM_DATA_BLOCKS      1024
#define DATA_BLOCK_SIZE      4096
#define MAX_NAME_LENGTH     32
#define REG_FILE_NUM        2
#define FILE_DESCRIPTOR_ARRAY_SIZE 8
#define OPEN 1
#define CLOSE 0
#define MIN_FD 0
#define MAX_FD 7
#define NUM_DEVICES 6

#define BYTE_BITS 8
#define EXEC_LOAD_ADDRESS 0x08048000
#define PROGRAM_OFFSET 0x00048000
#define FOURMB 0x0400000
#define EIGHTMB 2*FOURMB
#define FOURKB 0x01000
#define EIGHTKB 2*FOURKB

// struct for the directory entry based on lecture and Appendix A
typedef struct dentry_t {
    char filename[MAX_NAME_LENGTH];
    unsigned int filetype;
    unsigned int inode_num;
    char reserved[R_D_SIZE];
} dentry_t;

// struct for the boot block based on lecture and Appendix A
typedef struct boot_block_t {
    unsigned int dir_count;
    unsigned int inode_count;
    unsigned int data_count;
    char reserved[R_B_SIZE];
    dentry_t direntries[DIRENTRIES_SIZE];
} boot_block_t;

// struct for the inode based on lecture and Appendix A
typedef struct inode_t {
    unsigned int length;
    unsigned int data_block_num[NUM_DATA_BLOCKS - 1];
} inode_t;

// struct for a particular data block
typedef struct dblock_t {   
    char data[DATA_BLOCK_SIZE];
} dblock_t;

typedef struct fops_table_t { 
    int32_t (*open)(const uint8_t* filename);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*close)(int32_t fd);
} fops_table_t;

// struct for file descriptor
typedef struct file_descriptor_t {
    fops_table_t * file_operation_table_ptr;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} file_descriptor_t;

extern fops_table_t fops_table[NUM_DEVICES];

// file descriptor array
extern file_descriptor_t file_descriptor_array[FILE_DESCRIPTOR_ARRAY_SIZE];

// we want to store pointers to given memory locations
// split up into boot block, inodes, and data blocks
boot_block_t * boot_block_ptr;
inode_t * inode_ptr;
dblock_t * dblock_ptr;

// initializes the file system
extern void fileSystem_init(uint32_t* fs_start);
// scans through the directory entires in the boot block to find the file name
extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
// populates the dentry parameter: file name, file type, inode number
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
// reads data from a specific inode
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// helper functions for file system calls
// read, write, open, close args are based on system call function defs
extern int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t file_open(const uint8_t* filename);
extern int32_t file_close(int32_t fd);

// helper functions for directory system calls
// read, write, open, close args are based on system call function defs
extern int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t dir_open(const uint8_t* filename);
extern int32_t dir_close(int32_t fd);

#endif /* _FILE_SYSTEM_H */
