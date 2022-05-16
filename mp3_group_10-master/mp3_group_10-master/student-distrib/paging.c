#include "paging.h"

/* paging_init
 * Description: Initialize paging.
 * Inputs: None
 * Outputs: None
 * Side Effects: Initialize page directory and page tables; modify control registers.
 */
void paging_init(void)
{
    /*
        what we have:
            - 1 page directory, containing only two meaningful entries:
                -- the descriptor of a PAGE TABLE: the "addr" bits is the addr of "user page table"
                -- the descriptor of a PAGE: the "addr" bits is the addr of the kernel page (4MB)
            - 1 page table (called "user page table"), containing only one meaningful entry:
                -- the descriptor of a PAGE: the page is for the video memory (4KB)
    */

    // init page directory
    init_page_dir();

    // init user page table
    init_user_page_table();

    // Enable paging.
    //reference:OSDev-->paging(https://wiki.osdev.org/Paging)
    asm volatile(
        "xorl %%eax, %%eax          \n\t"
        "movl %0, %%eax             \n\t"
        "movl %%eax, %%cr3          \n\t"

        "movl %%cr4, %%eax          \n\t"
        "orl $0x00000010, %%eax     \n\t"
        "movl %%eax, %%cr4          \n\t"

        "movl %%cr0, %%eax          \n\t"
        "orl $0x80000000, %%eax     \n\t"
        "movl %%eax, %%cr0          \n\t"

        :
        : "r"(page_dir_base)
        : "eax");
}

/* init_page_dir
 * Description: Initialize page directory.
 * Inputs: None
 * Outputs: None
 * Side Effects: Initialize page directory.
 */
void init_page_dir()
{
    // init the first entry (4KB indirection, user space, containing video memory).
    {
        page_dir_entry_u the_page_dir_entry;
        page_dir_entry_4KB_t *pde = &(the_page_dir_entry.user_page_table_desc);
        pde->present = 1; // In memory.
        pde->r_w = 1;
        pde->user_super = 1;                                        // May be accessed by all.
        pde->write_through = 0;                                     // Write back.
        pde->cache_disable = 0;                                     // Enable page caching.
        pde->accessed = 0;                                          // Have not been read during virtual address translation.
        pde->avail0 = 0;                                            // Not used.
        pde->pageSize = 0;                                          // 4KB page for user space
        pde->avail1 = 0;                                            // Not used.
        pde->page_table_base = (uint32_t)usr_page_table_base >> 12; // Bits 31-12 of address.

        // write into page_dir_base
        page_dir_base[0] = the_page_dir_entry;
    }

    // init the second entry (4MB indirection, connect 4MB kernel page to this entry).
    {
        page_dir_entry_u the_page_dir_entry;
        page_dir_entry_4MB_t *pde = &(the_page_dir_entry.kernel_page_desc);
        pde->present = 1; // In memory.
        pde->r_w = 1;
        pde->user_super = 1;    // May be accessed by all.
        pde->write_through = 0; // Write back.
        pde->cache_disable = 0; // Enable page caching.
        pde->accessed = 0;      // Have not been read during virtual address translation.
        pde->dirty = 1;         // Has been written.
        pde->pageSize = 1;      // 4MB page for kernel space
        pde->global = 0;        // Global page ignore.
        pde->avail = 0;         // Not used.
        pde->pg_attri = 0;      // Reserved and be set to 0.
        pde->addr_0 = 0;        // Bits 39-32 of address.
        pde->reserved = 0;
        pde->pg_addr = (uint32_t)KERNEL_START >> 22; // Bits 31-12 of address.
        // 0x400183
        //  write into page_dir_base
        //  page_dir_base[1] = the_page_dir_entry;
        page_dir_base[1] = the_page_dir_entry;
    }
    {
        page_dir_entry_u the_page_dir_entry;
        page_dir_entry_4KB_t *pde = &(the_page_dir_entry.user_page_table_desc);
        pde->present = 1; // In memory.
        pde->r_w = 1;
        pde->user_super = 1;                                          // May be accessed by all.
        pde->write_through = 0;                                       // Write back.
        pde->cache_disable = 0;                                       // Enable page caching.
        pde->accessed = 0;                                            // Have not been read during virtual address translation.
        pde->avail0 = 0;                                              // Not used.
        pde->pageSize = 0;                                            // 4KB page for user space
        pde->avail1 = 0;                                              // Not used.
        pde->page_table_base = (uint32_t)usrmap_page_table_base>>12; // Bits 31-12 of address.

        // write into page_dir_base
        page_dir_base[132 / 4] = the_page_dir_entry;
    }

    // Initialization for the other 1022 directory enties (4MB indirection, connect 4MB pages).
    int i;
    for (i = 2; i < NUM_PAGE_TABLE_DESC; i++)
    {
        if (i == 132 / 4)
            continue;
        page_dir_entry_u the_page_dir_entry;
        page_dir_entry_4MB_t *pde = &(the_page_dir_entry.kernel_page_desc);
        pde->present = 0; // Not in memory.
        pde->r_w = 1;
        pde->user_super = 1;    // May be accessed by all.
        pde->write_through = 0; // Write back.
        pde->cache_disable = 0; // Enable page caching.
        pde->accessed = 0;      // Have not been read during virtual address translation.
        pde->dirty = 0;         // Have not been written.
        pde->pageSize = 1;      // 4MB page for kernel space
        pde->global = 0;        // Global page ignore
        pde->avail = 0;         // Not used.
        pde->pg_attri = 0;      // Reserved and be set to 0.
        pde->addr_0 = 0;        // Bits 39-32 of address.
        pde->reserved = 0;
        pde->pg_addr = (uint32_t)i * _4M >> 22; // Bits 31-12 of address.

        // write into page_dir_base
        page_dir_base[i] = the_page_dir_entry;
    }
}

/* init_user_page_table
 * Description: Initialize page table for user space.
 * Inputs: None
 * Outputs: None
 * Side Effects: Initialize page table for user space.
 */
void init_user_page_table()
{
    // 0~4M.
    int i;
    for (i = 0; i < NUM_PAGE_DESC; i++)
    {
        // initialze page_desc_t for each page
        page_table_entry_t the_page_table_entry;

        // B8000 - BC000
        if ((i == (VIDEO_START >> 12)) || (i == ((VIDEO_START >> 12) + 1)) || (i == ((VIDEO_START >> 12) + 2)) || (i == ((VIDEO_START >> 12) + 3)))
        {
            the_page_table_entry.present = 1;
        }
        else
        {
            the_page_table_entry.present = 0;
        }

        the_page_table_entry.r_w = 1;
        the_page_table_entry.usr_super = 1;     // May be accessed by all.
        the_page_table_entry.write_through = 0; // Write back.
        the_page_table_entry.cache_disable = 0; // Enable page caching.
        the_page_table_entry.accessed = 0;      // Have not been read during virtual address translation.
        the_page_table_entry.dirty = 0;
        the_page_table_entry.pg_attri = 0;          // Reserved and be set to 0.
        the_page_table_entry.global = 0;            // Global page ignore.
        the_page_table_entry.avail = 0;             // Not used.
        the_page_table_entry.pg_addr = (uint32_t)i; // 20-bit address for 4KB pages.

        // write into usr_page_table_base
        usr_page_table_base[i] = the_page_table_entry;
    }

    for (i = 0; i < NUM_PAGE_DESC; i++)
    {
        // initialze page_desc_t for each page
        page_table_entry_t the_page_table_entry;

        the_page_table_entry.present = 0;
        the_page_table_entry.pg_addr = (uint32_t)i; // 20-bit address for 4KB pages.
        the_page_table_entry.r_w = 1;
        the_page_table_entry.usr_super = 1;     // May be accessed by all.
        the_page_table_entry.write_through = 0; // Write back.
        the_page_table_entry.cache_disable = 0; // Enable page caching.
        the_page_table_entry.accessed = 0;      // Have not been read during virtual address translation.
        the_page_table_entry.dirty = 0;
        the_page_table_entry.pg_attri = 0; // Reserved and be set to 0.
        the_page_table_entry.global = 0;   // Global page ignore.
        the_page_table_entry.avail = 0;    // Not used.
        if (i == (VIDEO_START >> 12))
        {
            the_page_table_entry.present = 1;
        }
        // write into usr_page_table_base
        usrmap_page_table_base[i] = the_page_table_entry;
    }
}

/* switch_usrmap
 * Description: Change the mapping of the chunk of memory that the user get
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 */
void switch_usrmap(int index, int present)
{
    // initialze page_desc_t for each page
    page_table_entry_t the_page_table_entry;

    // B8000 - BC000
    the_page_table_entry.present = present;

    the_page_table_entry.r_w = 1;
    the_page_table_entry.usr_super = 1;     // May be accessed by all.
    the_page_table_entry.write_through = 0; // Write back.
    the_page_table_entry.cache_disable = 0; // Enable page caching.
    the_page_table_entry.accessed = 0;      // Have not been read during virtual address translation.
    the_page_table_entry.dirty = 0;
    the_page_table_entry.pg_attri = 0;              // Reserved and be set to 0.
    the_page_table_entry.global = present;                // Global page ignore.
    the_page_table_entry.avail = 0;                 // Not used.
    the_page_table_entry.pg_addr = (uint32_t)index; // 20-bit address for 4KB pages.

    // write into usr_page_table_base
    usrmap_page_table_base[VIDEO_START >> 12] = the_page_table_entry;
}


