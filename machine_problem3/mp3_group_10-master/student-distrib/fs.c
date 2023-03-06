#include "fs.h"
#include "lib.h"
#include "terminal.h"
#include "rtc.h"

dir_ops_t dir_ops;
regular_file_ops_t regular_file_ops;
rtc_ops_t rtc_ops;

boot_block_t fs_blocks[TOTAL_BLOCK];
boot_block_t boot_block;
uint32_t data_block_start;
uint32_t inode_start;

file_ops_t stdin_ops;
file_ops_t stdout_ops;

/* fs_init
 * Description: Initialize the filesystem.
 * Inputs: booting information
 * Outputs: None
 * Side Effects: Copy the filesystem to kernel. Calculate several important addresses within the filesystem.
 */
void fs_init(multiboot_info_t *boot_info) {
    uint32_t fs_start_addr = ((module_t *)(boot_info->mods_addr))->mod_start; 

    memcpy((void*)fs_blocks, (void*)fs_start_addr, BLOCK_SIZE * TOTAL_BLOCK); // Copy the filesystem from module space to kernel space.

    boot_block = fs_blocks[0]; // fill in the boot_block struct

    // calc addr
    inode_start = (uint32_t)fs_blocks + BLOCK_SIZE; // Start address of inode blocks.
    data_block_start = inode_start + BLOCK_SIZE * boot_block.boot_block_stats.num_inodes; // Start address of data blocks.

    // populate operation table
    dir_ops.read = (void*)dir_read;
    dir_ops.write = (void*)dir_write;
    dir_ops.close =(void*) dir_close;
    dir_ops.open =(void*)dir_open;

    regular_file_ops.close = (void*)file_close;
    regular_file_ops.open = (void*)file_open;
    regular_file_ops.write = (void*)file_write;
    regular_file_ops.read = (void*)file_read;

    rtc_ops.close = (void*)rtc_close;
    rtc_ops.open = (void*)rtc_open;
    rtc_ops.write = (void*)rtc_write;
    rtc_ops.read = (void*)rtc_read;

    stdin_ops.open = (void*)terminal_open;
    stdin_ops.close = (void*)terminal_close;
    stdin_ops.read = (void*)terminal_read;
    stdin_ops.write = (void*)terminal_write;

    stdout_ops.open = (void*)terminal_open;
    stdout_ops.close = (void*)terminal_close;
    stdout_ops.read = (void*)terminal_read;
    stdout_ops.write = (void*)terminal_write;
}

/* read_dentry_by_index
 * Description: Fill the dentry with the file info with index .
 * Inputs: index of boot block entries.
 *         pointer of a dentry struct.
 * Outputs: status: 0 for success; 1 for fail.
 * Side Effects: None.
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry) {
    if (index < 0 || index >= boot_block.boot_block_stats.num_dir_entries)
        return -1;
    
    dentry_t dentry_struct = boot_block.files[index];

    // fill in the filename
    strncpy(dentry->filename, dentry_struct.filename, FILENAME_MAX_CHAR);

    dentry->nr_inode = dentry_struct.nr_inode;
    dentry->type = dentry_struct.type;
    memset(dentry->reserved, 0, DIR_ENTRY_RESERVED_BYTE);

    return 0;
}

/* read_dentry_by_name
 * Description: Fill the dentry with the file info with filename.
 * Inputs: filename
 *         pointer of a dentry struct.
 * Outputs: status: 0 for success; 1 for fail.
 * Side Effects: None.
 */
int32_t read_dentry_by_name(const int8_t *fname, dentry_t *dentry) {
    int i;
    if(strlen(fname) > 32){
        // printf("File name too long\n");
        // printf("Length:     %d\n", strlen(fname));
        // printf("%s\n", fname);
        return -1;
    }
    // printf("%d\n", strlen(fname));
    for (i = 0; i < boot_block.boot_block_stats.num_dir_entries; i++) {
        if (!strncmp(boot_block.files[i].filename, fname, FILENAME_MAX_CHAR)) {
            // printf("%s\n", fname);
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }
    // printf("read fail!\n");

    return -1;
}

/* read_data
 * Description: read content of file with inode.
 * Inputs: inode of the file
 *         reading offset
 *         buffer
 *         the length of content to be read.
 * Outputs: positive value for the number of bytes read; -1 for fail.
 * Side Effects: content of the file is filled into the buffer.
 */
int32_t read_data(uint32_t inode, uint32_t offset, int8_t *buf, uint32_t length) {
    // attention
    // we don't check whether the inode passed corresponds to a file, just check the range
    // we have to trust the `buf` is large enough to hold the data (because we have no idea about the ptr ...)
    uint32_t i;      // idx of data node
    uint32_t end;   // end byte, truncate if needed
    uint32_t start_blk_idx, end_blk_idx;
    uint32_t data_blk_ptr;
    uint32_t start_off, end_off;

    if (inode >= MAX_FILES || inode < 0) {
        // Check if the inode is out of range 0~63
        return -1;
    }
    
    // fetch inode information given inode number
    inode_t *inode_struct = (inode_t *)(inode * BLOCK_SIZE + inode_start);
    

    if (inode_struct->length <= offset || offset < 0) {
        // Check if the start position of reading is beyond the length of the file.
        return 0;
    }

    // each data block is 4 KB (4096 b)

    start_blk_idx = offset / BLOCK_SIZE;   // index inside the inode array
    if (start_blk_idx >= MAX_FILES)
        return -1;
    start_off = offset - offset / BLOCK_SIZE * BLOCK_SIZE;

    end = (offset + length >= inode_struct->length ? inode_struct->length : offset + length);
    
    end_blk_idx = end / BLOCK_SIZE;        // index inside the inode array
    if (end_blk_idx >= MAX_FILES)
        return -1;
    end_off = end - end / BLOCK_SIZE * BLOCK_SIZE;

    // Check if a bad data block number is found within the file bounds of the given inode
    int j;
    for (j = start_blk_idx; j <= end_blk_idx; j++){
        if (inode_struct->inodes[j] >= boot_block.boot_block_stats.num_data_blocks) {
            return -1;
        }
    }
    
    // Start
    data_blk_ptr = inode_struct->inodes[start_blk_idx] * BLOCK_SIZE + data_block_start;
    if (end_blk_idx != start_blk_idx) {
        memcpy(buf, (void*)(data_blk_ptr + start_off), BLOCK_SIZE - start_off);
        buf += BLOCK_SIZE - start_off;
    } else {
        memcpy(buf, (void*)(data_blk_ptr + start_off), end_off - start_off);
    }

    for (i = start_blk_idx + 1; i < end_blk_idx; i++) {
        data_blk_ptr = inode_struct->inodes[i] * BLOCK_SIZE + data_block_start;
        memcpy(buf, (void*)data_blk_ptr, BLOCK_SIZE);
        buf += BLOCK_SIZE;
    }

    // End
    if (start_blk_idx != end_blk_idx) {
        data_blk_ptr = inode_struct->inodes[end_blk_idx] * BLOCK_SIZE + data_block_start;
        memcpy(buf, (void*)data_blk_ptr, end_off);
    }

    return end - offset;
}

/* file_open
 * Description: Open a file with filename and generating a file descriptor entry accordingly.
 * Inputs: filename
 * Outputs: file descriptor entry: whose flags == IN_USE for success; flags == OERROR for fail.
 * Side Effects: None.
 */
file_entry file_open(const int8_t *fname){
    dentry_t dentry;
    file_entry fde;
    if (read_dentry_by_name(fname, &dentry) == -1){
        fde.flags = OERROR;
        return fde;
    }
    fde.op_ptr = (uint32_t)&regular_file_ops; // Function table.
    fde.inode = dentry.nr_inode; // The inode number for this file.
    fde.file_pos = 0; // Keep track of where the user is currently reading from in the file.
    fde.flags = IN_USE;
    return fde;
}

/* dir_open
 * Description: Open a directory with directory name and generating a file descriptor entry accordingly.
 * Inputs: directory name
 * Outputs: file descriptor entry: whose flags == IN_USE for success; flags == OERROR for fail.
 * Side Effects: None.
 */
file_entry dir_open(const int8_t *dir_name){
    dentry_t dentry;
    file_entry fde;
    if (read_dentry_by_name(dir_name, &dentry) == -1){
        fde.flags = OERROR;
        printf("Fail to open the file!");
        return fde;
    }   
    fde.op_ptr = (uint32_t)&dir_ops;
    fde.inode = dentry.nr_inode;
    fde.file_pos = 0;
    fde.flags = IN_USE;
    return fde;
}

/* file_read
 * Description: read a regular file with according file descriptor entry.
 * Inputs: pointer of regular file descriptor entry
 *         buffer
 *         length of content to be read.
 * Outputs: positive value for the number of bytes read; -1 for fail.
 * Side Effects: content of the file is filled into the buffer.
 */
int32_t file_read(file_entry* fp, int8_t* buf, uint32_t length){
    // Read "length" bytes from the regular file with "inode".
    uint32_t res; // Return the number of bytes read.
    res = read_data(fp->inode, fp->file_pos, buf, length);
    if (res != -1){
        fp->file_pos += res;
        return res; // Read succeeds.
    }
    return -1; // Read fails.
}

/* dir_read
 * Description: read filename from the directory file once a time with according file descriptor entry.
 * Inputs: pointer of directory file descriptor entry
 *         buffer
 *         length of content to be read.
 * Outputs: positive value for the number of bytes read; -1 for fail.
 * Side Effects: one filename is filled into the buffer.
 */
int32_t dir_read(file_entry* fp, int8_t* buf, uint32_t length){
    // Read "num_files" files from the directory. 
    // uint32_t res; // Return the number of bytes read.
    // buf contains 80 chars
    // printf("enter dir***************\n");
    dentry_t dentry;
    uint32_t num_read;
    uint32_t index;
    // uint32_t file_size;
    uint32_t i;
    
    index = fp->file_pos;   // data block in directory is just the boot block (with some proper offset).
    if (index >= boot_block.boot_block_stats.num_dir_entries) {
        // printf("Directory read is out of range!");
        return 0;
    }
    read_dentry_by_index(index, &dentry);
    // printf("buf addr:%x\n", buf);
    memset(buf, '\0', 33);

    num_read = FILENAME_MAX_CHAR; //The length of the dir is fixed
    // strncpy(buf, "file_name: ", 11);  //The length of the string is 11
    // buf += 11;  //The length of the string is 11
    for (i = 0; i < num_read; i++) {
        if (dentry.filename[i] == '\0') {
            break;
        }
        buf[i] = dentry.filename[i];
    }
    // For CP2:S
    // j = i;
    // while (j < FILENAME_MAX_CHAR)
    //     buf[j++] = ' ';
    // buf += FILENAME_MAX_CHAR;
    // strncpy(buf, ", file_type: ", OFFSET_2);
    // buf += OFFSET_2;
    // buf[0] = '0' + dentry.type;
    // buf++;
    // strncpy(buf, ", file_size: ", OFFSET_2);
    // buf += OFFSET_2;
    // file_size = ((inode_t *)(dentry.nr_inode * BLOCK_SIZE + inode_start))->length;
    // if (file_size == 0) buf[OFFSET_1] = '0';  //
    // j = OFFSET_1;
    // while (file_size) {
    //     buf[j] = '0' + file_size % 10;  //10 is for dividing the digits of number
    //     file_size /= 10;
    //     j--;
    // }
    // while (j--)
    //     buf[j] = ' ';
    
    fp->file_pos += 1; // Increment the file position by 1 each time read a directory.
    return i;
}

/* file_write
 * Description: Not been implemented, since it is a read-only filesystem. (Write a regular file with according file descriptor entry.)
 * Inputs: pointer of regular file descriptor entry
 *         buffer
 *         length of content to be written.
 * Outputs: -1
 * Side Effects: NONE.
 */
int32_t file_write(file_entry* fp, int8_t* buf, uint32_t length) {
    return -1;
}

/* file_write
 * Description: Not been implemented, since it is a read-only filesystem. (Write a directory file with according file descriptor entry.)
 * Inputs: pointer of directory file descriptor entry
 *         buffer
 *         length of content to be written.
 * Outputs: -1
 * Side Effects: NONE.
 */
int32_t dir_write(file_entry* fp, int8_t* buf, uint32_t length) {
    return -1;
}

/* file_close
 * Description: close a regular file with according file descriptor entry and undo operations in open function.
 * Inputs: pointer of regular file descriptor entry
 * Outputs: 0
 * Side Effects: NONE.
 */
int32_t file_close(file_entry* fp) {
    return 0;
}

/* dir_close
 * Description: close a directory file with according file descriptor entry and undo operations in open function.
 * Inputs: pointer of directory file descriptor entry
 * Outputs: 0
 * Side Effects: NONE.
 */
int32_t dir_close(file_entry* fp) {
    return 0;
}
