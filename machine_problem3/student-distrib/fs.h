#ifndef FS_H
#define FS_H

#include "types.h"
#include "multiboot.h"

#define BLOCK_SIZE (1024 * 4)
#define MAX_FILES 63
#define FILENAME_MAX_CHAR 32    /* NOTE: since filename not necessarily includes a terminal EOS, we may truncate characters that exceeds the limit */
#define DIR_ENTRY_RESERVED_BYTE 24
#define BOOT_STAT_RESERVED_BYTE 52
#define MAX_INODES_PER_FILE 1023
#define MAX_FILE_SIZE (1023 * 4 * 1024)   /* 4MB - 4KB */
#define TOTAL_BLOCK (1 + 64 + 59)
#define SCREEN_WIDTH 80
#define OFFSET_1 7 //used for formatted terminal output

#define OFFSET_2 13 //used for formatted terminal output

/* file type const */
#define USER_RTC 0
#define DIR 1
#define REGULAR 2

/* file descriptor table entry flag */
#define OERROR 0
#define IN_USE 1
#define FREE 2

typedef struct fs_stats {
    uint32_t num_dir_entries;   /* should in range 0 - 63 */
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    int8_t reserved[BOOT_STAT_RESERVED_BYTE];
}fs_stats_t;

typedef struct dir_entry {
    int8_t filename[FILENAME_MAX_CHAR];
    uint32_t type;
    uint32_t nr_inode;   /* valid ONLY regular file */
    int8_t reserved[DIR_ENTRY_RESERVED_BYTE];
}dentry_t;

typedef struct boot_block {
    fs_stats_t boot_block_stats;
    dentry_t files[MAX_FILES];
}boot_block_t;

typedef struct inode {
    uint32_t length;   /* file size in bytes */
    uint32_t inodes[MAX_INODES_PER_FILE];  /* data blocks (check validity before read!) */
}inode_t;

typedef struct file_entry {
    uint32_t op_ptr;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flags;
}file_entry;

typedef struct regular_file_ops {
    int32_t (*read)(file_entry* fp, void* buf, int32_t nbytes);
    int32_t (*write)(file_entry* fp, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* fname);
    int32_t (*close)(file_entry* fp);
}regular_file_ops_t;

typedef struct dir_ops {
    int32_t (*read)(file_entry* fp, void* buf, int32_t nbytes);
    int32_t (*write)(file_entry* fp, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* fname);
    int32_t (*close)(file_entry* fp);
}dir_ops_t;

typedef struct rtc_ops {
    int32_t (*read)(file_entry* fp, void* buf, int32_t nbytes);
    int32_t (*write)(file_entry* fp, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* fname);
    int32_t (*close)(file_entry* fp);
}rtc_ops_t;

typedef struct file_ops {
    int32_t (*read)(file_entry* fp, void* buf, int32_t nbytes);
    int32_t (*write)(file_entry* fp, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* fname);
    int32_t (*close)(file_entry* fp);
} file_ops_t;

/* global variables */
extern boot_block_t boot_block;
// extern boot_block_t *fs_start_addr;
extern dir_ops_t dir_ops;
extern regular_file_ops_t regular_file_ops;
extern rtc_ops_t rtc_ops;
extern boot_block_t fs_blocks[TOTAL_BLOCK];
extern uint32_t data_block_start;
extern uint32_t inode_start;
extern file_ops_t stdin_ops;
extern file_ops_t stdout_ops;

/* -1 on failure, non-existing file. 0 on success*/
int32_t read_dentry_by_name(const int8_t *fname, dentry_t *dentry);

/* -1 on failure, invalid index. 0 on success*/
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);

/* return number of bytes read and placed in the buffer, 0 means EOF */
int32_t read_data(uint32_t inode, uint32_t offset, int8_t *buf, uint32_t length);

/* file operations */

/* init file temp structures, return 0 */
file_entry file_open(const int8_t *fname);

int32_t file_close(file_entry* fp);

int32_t file_write(file_entry* fp, int8_t* buf, uint32_t length);

int32_t file_read(file_entry* fp, int8_t* buf, uint32_t length);

/* dir operations */

file_entry dir_open(const int8_t *dir_name);

int32_t dir_close(file_entry* fp);

int32_t dir_write(file_entry* fp, int8_t* buf, uint32_t length);

int32_t dir_read(file_entry* fp, int8_t* buf, uint32_t length);

/* init the file system */
void fs_init(multiboot_info_t *boot_info);

#endif  /* fs.h */

