
#include "file_sys.h"
#include "types.h"
#include "lib.h"

// declare array for file descriptors
file_descriptor_t file_descriptor_array[FILE_DESCRIPTOR_ARRAY_SIZE];

// Description: Initializes the file system.
// Inputs: fs_start - Pointer to the start of the file system.
// Outputs: None
// Effects: Sets up pointers to the boot block, inode, and data block.
void fileSystem_init(uint32_t* fs_start) {
    // Set the boot block pointer to the start of the file system
    boot_block_ptr = (boot_block_t*) fs_start;
    // Set the inode pointer to the location immediately after the boot block
    inode_ptr = (inode_t*) boot_block_ptr + 1;
    // Set the data block pointer to the location immediately after the inodes
    dblock_ptr = (dblock_t*) inode_ptr + boot_block_ptr->inode_count;

    int i;
    for(i = 0; i < FILE_DESCRIPTOR_ARRAY_SIZE; i++){
        file_descriptor_array[i].inode = 0;
        file_descriptor_array[i].file_position = 0;
        file_descriptor_array[i].flags = CLOSE;
    }

    // we want to open stdin and stdout on initialization
    // open stdin
    file_descriptor_array[0].flags = OPEN;
    // open stdout
    file_descriptor_array[1].flags = OPEN;
}

// Description: Reads directory entry by name.
// Inputs: fname - Name of the file, dentry - Pointer to directory entry structure.
// Outputs: Returns 0 on success, -1 on failure.
// Effects: Fills in the dentry structure with information about the file.
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    // Check if filename is NULL
    if(fname == NULL) return -1;

    // Calculate length of filename
    unsigned int file_name_length = 0;
    while(fname[file_name_length] != '\0') file_name_length++;
    
    if(fname[file_name_length - 1] == '\n') file_name_length--;

    // don't read file if name is too long
    // return -1 for fail
    if(file_name_length > MAX_NAME_LENGTH){
        return -1;
    }

    // Get number of directory entries
    unsigned int num_dentries = boot_block_ptr->dir_count;
    dentry_t* directories = boot_block_ptr->direntries;

    // Loop through all directory entries
    unsigned int i;
    for(i = 0; i < num_dentries; i++)
    {
        dentry_t curr_directory = directories[i];
        char* curr_file_name = curr_directory.filename;

        // Calculate length of current filename
        unsigned int curr_file_name_length = 0;
        while(curr_file_name_length < MAX_NAME_LENGTH && curr_file_name[curr_file_name_length] != '\0') curr_file_name_length++;

        // If lengths do not match, continue to next iteration
        if(file_name_length != curr_file_name_length) continue;

        // If filenames match, read directory entry by index
        if(strncmp((int8_t*)fname, (int8_t*)curr_file_name, file_name_length) == 0)
        {
            return read_dentry_by_index(i, dentry);
        }
    }

    // If no match found, return -1
    return -1;
}

// Description: Reads directory entry by index.
// Inputs: index - Index of the directory entry, dentry - Pointer to directory entry structure.
// Outputs: Returns 0 on success, -1 on failure.
// Effects: Fills in the dentry structure with information about the file.
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    // Check if index is valid
    if (index >= boot_block_ptr->inode_count) {
        return -1;
    }

    // Get directory entry at given index
    dentry_t curr_directory = boot_block_ptr->direntries[index];

    // Copy data from current directory entry to output directory entry
    strcpy((int8_t*) dentry->filename, (int8_t*) curr_directory.filename);
    dentry->filetype = curr_directory.filetype;
    dentry->inode_num = curr_directory.inode_num;
    memcpy(dentry->reserved, curr_directory.reserved, sizeof(curr_directory.reserved));
    
    return 0;
}

// Description: Reads data from a file.
// Inputs: inode - Inode number, offset - Offset into the file, buf - Buffer to store data, len - Number of bytes to read.
// Outputs: Returns number of bytes read.
// Effects: Reads data from a file into a buffer.
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t len) {
    // Get pointer to inode structure
    inode_t* curr_inode = inode_ptr + inode;

    // Check if inode number and offset are valid
    if (inode >= boot_block_ptr->inode_count || offset >= curr_inode->length) return 0;

    // Adjust length if it exceeds file size
    len = (len > curr_inode->length) ? curr_inode->length : len;
    
    // Adjust length if it exceeds remaining bytes in file after offset
    if (offset + len > curr_inode->length) len = curr_inode->length - offset;

    unsigned int num_bytes_read_total = 0, 
                 curr_data_block_num = offset / DATA_BLOCK_SIZE, 
                 curr_byte_index = offset % DATA_BLOCK_SIZE;

     unsigned int i;
     for (i = 0; i < len; i++) {        
         dblock_t* curr_data_block = (dblock_t*) dblock_ptr + curr_inode->data_block_num[curr_data_block_num];
         buf[i] = curr_data_block->data[curr_byte_index++];
         if (curr_byte_index >= DATA_BLOCK_SIZE) {
             curr_byte_index = 0;
             curr_data_block_num++;
         }
         num_bytes_read_total++;
     }
     return num_bytes_read_total;
}


// Description: Reads data from a file.
// Inputs: fd - File descriptor, buf - Buffer to store data, nbytes - Number of bytes to read.
// Outputs: Returns number of bytes read.
// Effects: Reads data from a file into a buffer and updates the current file position.
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    // check for valid file
    // if the file is not valid, just return 0 
    // 0 bytes read
    if(fd > MAX_FD || fd < MIN_FD){
        return 0;
    }
    // Clear the buffer
    memset(buf, '\0', nbytes);    
    // Read data from the file
    unsigned int num_bytes_read = read_data(file_descriptor_array[fd].inode, file_descriptor_array[fd].file_position, buf, nbytes);

    // update the file descriptor with the new file position
    file_descriptor_array[fd].file_position += num_bytes_read;

    // Return the number of bytes read
    return num_bytes_read;
}


// Description: Writes data to a file.
// Inputs: fd - File descriptor, buf - Buffer containing data to write, nbytes - Number of bytes to write.
// Outputs: Returns -1.
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

// Description: Opens a file.
// Inputs: filename - Name of the file to open.
// Outputs: Returns opened fd, -1 on failure.
// Effects: Sets up global variables for the opened file's inode number and initial position.
int32_t file_open(const uint8_t* filename) {
    // Read the directory entry for the file
    dentry_t curr_dentry;
    if (read_dentry_by_name(filename, &curr_dentry) == -1) return -1;

    // look for the first unused file descriptor
    int i, free_fd;
    for(i = 0; i < FILE_DESCRIPTOR_ARRAY_SIZE; i++){
        if(file_descriptor_array[i].flags == CLOSE){
            free_fd = i;
            break;
        }
    }

    // check for valid file
    // if the file is not valid, just fail (return -1)
    if(free_fd > MAX_FD || free_fd < MIN_FD){
        return -1;
    }

    // Set the file descriptor inode number for the file
    file_descriptor_array[free_fd].inode = (curr_dentry.filetype == REG_FILE_NUM) ? curr_dentry.inode_num : 0;
    // Reset the file descriptor file position
    file_descriptor_array[free_fd].file_position = 0;
    // Return success
    return free_fd;
}

// Description: Closes a file.
// Inputs: fd - File descriptor.
// Outputs: Returns 0
int32_t file_close(int32_t fd) {
    // check if valid file descriptor
    // if so, close the current file
    // user does not have access to close stdin or stdout
    // which are at fd = 0 and 1 respectively
    if(fd > MIN_FD + 1 && fd <FILE_DESCRIPTOR_ARRAY_SIZE){
        file_descriptor_array[fd].flags = CLOSE;
        return 0;
    }
    // if not a valid file, fail
    return -1;
}


// Description: Reads a directory entry.
// Inputs: fd - File descriptor, buf - Buffer to store directory entry name, nbytes - Number of bytes to read.
// Outputs: Returns length of directory entry name read.
// Effects: Reads the name of a directory entry into a buffer and updates the global directory position.
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    // check for valid file
    // if the file is not valid, just fail (return -1)
    if(fd > MAX_FD || fd < MIN_FD){
        return -1;
    }

    // Read the directory entry at the current global file position
    dentry_t curr_dentry;
    if (read_dentry_by_index(file_descriptor_array[fd].file_position++, &curr_dentry) == -1) return 0;

    // Clear the buffer
    memset(buf, '\0', nbytes);
    char* filename = curr_dentry.filename;
    unsigned int file_length = strlen((const int8_t*) filename);

    // Adjust nbytes if it exceeds the length of the filename
    nbytes = (nbytes > file_length) ? file_length : nbytes;
    if(file_length > MAX_NAME_LENGTH) file_length = MAX_NAME_LENGTH;

    // Copy the filename to the buffer
    strncpy((int8_t*) buf, (int8_t*) curr_dentry.filename, nbytes);
    ((char*)buf)[file_length] = '\0';

    // Return the length of the filename
    return strlen((const int8_t*) buf);
}

// Description: Writes a directory entry.
// Inputs: fd - File descriptor, buf - Buffer containing directory entry to write, nbytes - Number of bytes to write.
// Outputs: Returns -1
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

// Description: Opens a directory.
// Inputs: filename - Name of the directory to open.
// Outputs: Returns opened fd, -1 on failure.
// Effects: Sets up global variables for the opened directory's inode number and initial position.
int32_t dir_open(const uint8_t* filename) {
    // Check if filename is NULL
    if (filename == 0) return -1;

    // Read the directory entry for the directory
    dentry_t curr_dentry;
    if (read_dentry_by_name((uint8_t*) filename, &curr_dentry) == -1) return -1;

    // look for the first unused file descriptor
    int i, free_fd;
    for(i = 0; i < FILE_DESCRIPTOR_ARRAY_SIZE; i++){
        if(file_descriptor_array[i].flags == CLOSE){
            free_fd = i;
            break;
        }
    }

    // check for valid file
    // if the file is not valid, just fail (return -1)
    if(free_fd > MAX_FD || free_fd < MIN_FD){
        return -1;
    }

     // Set the descriptor inode number for the directory
     file_descriptor_array[free_fd].inode = (curr_dentry.filetype == 1) ? 0 : -1;

     // Reset the descriptor file position
     file_descriptor_array[free_fd].file_position = 0;

     // Return success or failure based on whether inode number is valid or not
     return (file_descriptor_array[free_fd].inode == -1) ? -1 : free_fd;
}

// Description: Closes a directory. Currently does nothing.
// Inputs: fd - File descriptor.
// Outputs: Returns 0
int32_t dir_close(int32_t fd) {
    // check if valid file descriptor
    // if so, close the current file
    // user does not have access to close stdin or stdout
    // which are at fd = 0 and 1 respectively
    if(fd > MIN_FD + 1 && fd <FILE_DESCRIPTOR_ARRAY_SIZE){
        file_descriptor_array[fd].flags = CLOSE;
        return 0;
    }
    // if not a valid file descriptor, fail
    return -1;
}
