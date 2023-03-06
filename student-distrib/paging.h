#ifndef _PAGING_
#define _PAGING_

#include "types.h"

#define _4K 4096 // 2^12.
#define _8K (_4K * 2)
#define _12K (_4K + _8K)
#define _4M (_4K * 1024) // 2^22
#define _8M (_4M * 2)
#define _128M (_8M * 16)
#define _132M (_128M + _4M)
#define _256M (_128M * 2)
#define ONE_THREE_TWO_MB_BASE (_4M * 33)

#define OFFSET_12 12

#define NUM_PAGE_TABLE_DESC 1024 /* each desc described a page table */
#define NUM_PAGE_DESC 1024       /* each desc described a 4k page    */
#define VIDEO_START 0xB8000
#define VIDEO VIDEO_START
#define VIRTUAL_VIDEO_START (256 * 1024 * 1024)
#define KERNEL_START (4 * 1024 * 1024)

#ifndef ASM

//****************Indirection1: 4MB****************
typedef struct page_dir_entry_4MB
{
    uint32_t present : 1;
    uint32_t r_w : 1;
    uint32_t user_super : 1;
    uint32_t write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t pageSize : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t pg_attri : 1;
    uint32_t addr_0 : 8;
    uint32_t reserved : 1;
    uint32_t pg_addr : 10; // Align 4MB.
} page_dir_entry_4MB_t;
//****************Indirection1: 4MB****************

//****************Indirection2: 4KB****************
typedef struct page_dir_entry_4KB
{
    uint32_t present : 1;
    uint32_t r_w : 1;
    uint32_t user_super : 1;
    uint32_t write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t avail0 : 1;
    uint32_t pageSize : 1;
    uint32_t avail1 : 4;
    uint32_t page_table_base : 20; // Align 4KB.
} page_dir_entry_4KB_t;

typedef struct page_table_entry
{
    uint32_t present : 1;
    uint32_t r_w : 1;
    uint32_t usr_super : 1;
    uint32_t write_through : 1;
    uint32_t cache_disable : 1;
    uint32_t accessed : 1;
    uint32_t dirty : 1;
    uint32_t pg_attri : 1;
    uint32_t global : 1;
    uint32_t avail : 3;
    uint32_t pg_addr : 20; // 20-bit address for 4KB pages.
} page_table_entry_t;

//****************Indirection2: 4KB****************

typedef union page_dir_entry
{
    page_dir_entry_4KB_t user_page_table_desc;
    page_dir_entry_4MB_t kernel_page_desc;
} page_dir_entry_u;

page_dir_entry_u page_dir_base[NUM_PAGE_TABLE_DESC] __attribute__((aligned(_4K)));

page_table_entry_t usr_page_table_base[NUM_PAGE_DESC] __attribute__((aligned(_4K)));
page_table_entry_t usrmap_page_table_base[NUM_PAGE_DESC] __attribute__((aligned(_4K)));
// page_table_entry_t video_page_table_base[NUM_PAGE_DESC] __attribute__((aligned (_4K)));
// page_table_entry_t video_buffer_page_table_base[NUM_PAGE_DESC] __attribute__((aligned (_4K)));

//****************Helpers****************

void paging_init(void);

void init_page_dir();

void init_user_page_table();

// this is in ASM file
extern void enable_paging(page_dir_entry_u *page_dir_base);

extern void switch_usrmap(int index, int present);

#endif /* ASM */
#endif /* paging.h */
