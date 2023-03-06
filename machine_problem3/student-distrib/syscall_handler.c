#include "paging.h"
#include "syscall_handler.h"
#include "terminal.h"
#include "fs.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"

// int usr_programs_remaining = 3;     // decrement every usr programs (also shells but not base shells)
int cur_pid = -1; // scheduler should control this!
PCB_t *cur_pcb_ptr = NULL;
// int8_t args[3][50];                 // 3 user argument buffers
// int8_t command[3][50];              // 3 command buffers
int8_t pcb_bitmap[MAX_NUM_PROCESS] = {0, 0, 0, 0, 0, 0};

// extern int32_t ece391_execute(uint8_t *command);

//! ===================================================================================

/* read_SYSCALL
 *
 * Inputs: int32_t fd,
 *         void *buf,
 *         int32_t nbytes
 * Outputs: 0
 * Side Effects: perform read system call for different file type
 * Reference: OSdev
 */
int syscall_read(int32_t fd, void *buf, int32_t nbytes)
{
    sti();
    int res;
    if (fd >= 8 || fd < 0 || fd == 1)
        return -1;

    if (cur_pcb_ptr->pcb_fds[fd].flags == FREE)
        return -1;

    file_ops_t *ops;
    ops = (file_ops_t *)cur_pcb_ptr->pcb_fds[fd].op_ptr;

    res = ops->read(&(cur_pcb_ptr->pcb_fds[fd]), buf, nbytes);
    // printf("num bytes read is: %d\n", res);
    return res;
}

/* open_SYSCALL
 *
 * Inputs: filename
 * Outputs: -1 for failure, 0 for success
 * Side Effects: perform open system call for different file type
 * Reference: OSdev
 */
int syscall_open(const uint8_t *filename)
{
    int idx;
    dentry_t tmp_dentry;

    if (filename == NULL)
        return -1;

    if (-1 == read_dentry_by_name((int8_t *)filename, &tmp_dentry))
        return -1;

    for (idx = 2; idx < 8; idx++)
    {
        if (cur_pcb_ptr->pcb_fds[idx].flags == FREE)
        {
            switch (tmp_dentry.type)
            {
            // Select open operation based on file type.
            case DIR:
                if ((dir_open((int8_t *)filename)).flags == OERROR)
                    return -1;

                cur_pcb_ptr->pcb_fds[idx].op_ptr = (uint32_t)&dir_ops;
                break;
            case USER_RTC:
                rtc_open(filename);

                cur_pcb_ptr->pcb_fds[idx].op_ptr = (uint32_t)&rtc_ops;
                break;
            case REGULAR:
                if ((file_open((int8_t *)filename)).flags == OERROR)
                    return -1;

                cur_pcb_ptr->pcb_fds[idx].op_ptr = (uint32_t)&regular_file_ops;
                break;
            default:
                break;
            }

            cur_pcb_ptr->pcb_fds[idx].inode = tmp_dentry.nr_inode;
            cur_pcb_ptr->pcb_fds[idx].file_pos = 0;
            cur_pcb_ptr->pcb_fds[idx].flags = IN_USE;

            return idx; // return fd
        }
    }

    return -1;
}

/* close_SYSCALL
 *
 * Inputs: int32_t fd,
 * Outputs: 0 for success, -1 for failure
 * Side Effects: perform close system call for different file type
 * Reference: OSdev
 */
int32_t syscall_close(int32_t fd)
{
    if (fd < 2 || fd > 7)
        return -1;

    if (cur_pcb_ptr->pcb_fds[fd].flags == FREE)
        return -1;

    cur_pcb_ptr->pcb_fds[fd].flags = FREE; // Free this file descriptor entry.
    return 0;
}

/* write_SYSCALL
 *
 * Inputs: int32_t fd,
 *         void *buf,
 *         int32_t nbytes
 * Outputs: 0 for success, -1 for failure
 * Side Effects: perform write system call for different file type
 * Reference: OSdev
 */
int32_t syscall_write(int32_t fd, void *buf, int32_t nbytes)
{
    if (fd >= 8 || fd <= 0)
        return -1;

    if (cur_pcb_ptr->pcb_fds[fd].flags == FREE)
        return -1;

    file_ops_t *ops;
    ops = (file_ops_t *)cur_pcb_ptr->pcb_fds[fd].op_ptr;
    return ops->write(&(cur_pcb_ptr->pcb_fds[fd]), buf, nbytes);
}

//! ===================================================================================
/* syscall_getargs
 *
 * Inputs: -buf
 *         -nbutes
 * Outputs: 0 for success, -1 for failure
 * Side Effects: perform getargs system call
 * Reference: OSdev
 */
int32_t syscall_getargs(uint8_t *buf, int32_t nbytes)
{
    int i;

    if (nbytes < 0 || buf == NULL)
        return -1;

    if (cur_pcb_ptr->args[0] == '\0')
        return -1;

    for (i = 0; i < nbytes; i++)
        buf[i] = cur_pcb_ptr->args[i];

    return 0;
}

/* vidmap_SYSCALL
 *
 * Inputs: uint8_t **screen_start,
 * Outputs: 0 for success, -1 for failure
 * Side Effects: allocate new page and assign it to the program
 * Reference: OSdev
 */
int32_t syscall_vidmap(uint8_t **screen_start)
{
    if (screen_start == NULL)
        return -1;
    if ((int32_t)screen_start < USER_START || (int32_t)screen_start > (USER_END - 4))
    {
        printf("Invalid requirement of vidmap!\n");
        return -1;
    }

    if (running_term_ptr->tid != viewing_term_ptr->tid)
        switch_usrmap(running_term_ptr->vid_mem_buffer >> 12, 1);
    else
        switch_usrmap(VIDEO >> 12, 1);

    flush_tlb();
    *screen_start = (uint8_t *)(FISH_MAP);
    cur_pcb_ptr->vidmap_flag = 1;
    running_term_ptr->vidmap_flag = 1;

    return 0;
}

//! ===================================================================================
/* syscall_execute
 *
 * Inputs: -user_input
 * Outputs: 0 for success, -1 for failure
 * Side Effects: perform execute
 * Reference: OSdev
 */
int32_t syscall_execute(const uint8_t *user_input)
{

    cli();

    // declare local variables here
    int8_t first_four_bytes[4];
    dentry_t the_file_dentry;
    int8_t args[ARG_LEN];
    int8_t file_name[FNAME_MAX_LEN + 1];
    int read_result;
    int new_pid;

    int8_t *program_img_addr_virtual;
    int32_t addr_of_that_inode;
    inode_t *inode_ptr;
    int i;
    PCB_t *new_pcb_ptr;

    uint8_t *entry;
    uint32_t user_eip;

    uint32_t new_process_addr;

    /* ==================================== sanity check ==================================== */

    if (user_input == NULL)
        return -1;

    /* ==================================== parse arguments ==================================== */

    if (-1 == parse_args((int8_t *)user_input, file_name, args))
        return -1;

    /* ==================================== check ELF ==================================== */

    if (read_dentry_by_name((int8_t *)file_name, &the_file_dentry) == -1)
    {
        printf("read_by_name fails\n");
        return -1;
    }

    read_result = read_data(the_file_dentry.nr_inode, 0, first_four_bytes, 4);

    if (-1 == read_result ||
        first_four_bytes[0] != 0x7F ||
        first_four_bytes[1] != 0x45 ||
        first_four_bytes[2] != 0x4c ||
        first_four_bytes[3] != 0x46)
    {
        printf("Checking ELF fails\n");
        return -1;
    }

    /* ==================================== create new process ==================================== */
    new_pid = record_process();
    if (new_pid == -1)
    {
        printf("Reach maximal number of programs!\n");
        return 0;
    }

    /* ==================================== set up paging ==================================== */

    setup_paging_and_flush_tlb(new_pid);

    /* ==================================== load user file ==================================== */

    program_img_addr_virtual = (int8_t *)PROGRAM_VIR_ADDR; // RTDC
    addr_of_that_inode = inode_start + the_file_dentry.nr_inode * BLOCK_SIZE;
    inode_ptr = (inode_t *)(addr_of_that_inode);
    read_data(the_file_dentry.nr_inode, 0, program_img_addr_virtual, inode_ptr->length);

    /* ==================================== initiaize PCB ==================================== */
    new_pcb_ptr = get_pcb_ptr(new_pid);
    new_pcb_ptr->pid = new_pid;
    new_pcb_ptr->parent_pid = cur_pid;
    new_pcb_ptr->vidmap_flag = 0;
    init_file_table(new_pcb_ptr);

    new_process_addr = (uint32_t)new_pcb_ptr;
    new_pcb_ptr->tss_esp0 = new_process_addr + _8K - 4;

    for (i = 2; i < 8; i++)
        new_pcb_ptr->pcb_fds[i].flags = FREE;

    strncpy(new_pcb_ptr->args, args, ARG_LEN);

    /* ==================================== context switch to user ==================================== */

    // save current info
    if (cur_pid != -1)
    {
        asm volatile(
            "movl %%esp, %0 \n\t"
            "movl %%ebp, %1 \n\t"
            : "=r"(cur_pcb_ptr->esp), "=r"(cur_pcb_ptr->ebp));
    }

    // update current info
    cur_pcb_ptr = new_pcb_ptr;
    cur_pid = new_pid;

    tss.ss0 = KERNEL_DS;
    tss.esp0 = new_pcb_ptr->tss_esp0;

    // iret!
    entry = (uint8_t *)PROGRAM_VIR_ADDR + 24;
    user_eip = *(uint32_t *)entry;

    sti();
    asm volatile(
        "pushl %%eax        \n\t"
        "pushl %%ebx        \n\t"
        "pushfl             \n\t"
        "pushl %%ecx        \n\t"
        "pushl %%edx        \n\t"
        "iret               \n\t"
        :
        : "a"(USER_DS), "b"(_128M + _4M - 4), "c"(USER_CS), "d"(user_eip)
        : "cc", "memory");

    // this return will never be executed
    printf("You reach execute's return!\n");
    return 0;
}

/* syscall_halt
 *
 * Inputs: -user_input
 * Outputs: 0 for success, -1 for failure
 * Side Effects: perform halt
 * Reference: OSdev
 */
int32_t syscall_halt(uint8_t status)
{
    cli();

    // declare local variables here
    int i;
    PCB_t *parent_pcb_ptr;
    uint16_t retval;

    /* ==================================== deallocate pid ==================================== */

    decord_process(cur_pid);

    /* ==================================== restore parent data ==================================== */

    if (cur_pcb_ptr->parent_pid == -1)
    {
        cur_pid = -1;
        syscall_execute((uint8_t *)"shell");
        printf("This should not be printed!\n");
    }

    parent_pcb_ptr = get_pcb_ptr(cur_pcb_ptr->parent_pid);

    tss.ss0 = KERNEL_DS;
    tss.esp0 = parent_pcb_ptr->tss_esp0;

    /* ==================================== restore parent paging ==================================== */

    setup_paging_and_flush_tlb(cur_pcb_ptr->parent_pid);

    /* ==================================== close any relevant FDs ==================================== */

    for (i = 0; i < 8; i++)
        syscall_close(i);

    /* ==================================== switch user mapping ==================================== */

    if (cur_pcb_ptr->vidmap_flag)
    {
        cur_pcb_ptr->vidmap_flag = 0;
        running_term_ptr->vidmap_flag = 0;
        if (running_term_ptr->tid == viewing_term_ptr->tid)
            switch_usrmap(VIDEO >> 12, 0); // map to current terminal, close page
        else
            switch_usrmap(running_term_ptr->vid_mem_buffer >> 12, 0); // map to buffer, close page
    }

    /* ==================================== modify "current" info ==================================== */

    cur_pid = cur_pcb_ptr->parent_pid;
    cur_pcb_ptr = parent_pcb_ptr;

    /* ==================================== jump to execute return ==================================== */

    retval = (status == 255) ? 256 : (uint16_t)status;

    asm volatile(
        "movl %0, %%esp     \n\t"
        "movl %1, %%ebp     \n\t"
        "movl $0, %%eax     \n\t"
        "movw %2, %%ax      \n\t"
        "leave              \n\t"
        "ret                \n\t"
        :
        : "r"(parent_pcb_ptr->esp), "r"(parent_pcb_ptr->ebp), "r"(retval)
        : "esp", "ebp", "eax");

    return 0;
}

//! ===================================================================================

/* syscall_sethandler
 *
 * Inputs: signum, handler address
 * Outputs: 0 for success, -1 for failure
 * Side Effects: perform execute
 * Reference: OSdev
 */
int32_t syscall_sethandler(int32_t signum, void *handler_address)
{
    return -1;
}

/* syscall_sigreturn
 *
 * Inputs: None
 * Outputs: 0 for success, -1 for failure
 * Side Effects: perform execute
 * Reference: OSdev
 */
int32_t syscall_sigreturn(void)
{
    return -1;
}

// above: 10 syscalls
//! ===================================================================================
// below: helpers

/* get_pcb_ptr
 *
 * Inputs: -user_input
 * Outputs: 0 for success, -1 for failure
 * Side Effects: perform execute
 * Reference: OSdev
 */
PCB_t *get_pcb_ptr(int32_t pid)
{
    return (PCB_t *)(_8M - (pid + 1) * _8K);
}

// scan the bitmap for pcb (6 elements in total)
/*
 * record_process
 * Description: allocate a new pid
 *  Inputs: None
 *  Outputs: None
 * Side Effects: None.
 */
int32_t record_process(void)
{
    int i;
    int new_pid = -1;
    for (i = 0; i < 6; i++)
    {
        if (0 == pcb_bitmap[i])
        {
            new_pid = i;
            pcb_bitmap[i] = 1;
            break;
        }
    }

    return new_pid;
}

/*
 * decord_process
 * Description: deallocate a new pid
 *  Inputs: pid
 *  Outputs: None
 * Side Effects: None.
 */
int32_t decord_process(int32_t pid)
{
    if (pcb_bitmap[pid] == 0)
        return -1;

    pcb_bitmap[pid] = 0;
    return 0;
}

/* parse_args
 *
 * Inputs: input_command
 *         file_name
 *         command
 * Outputs: none
 * Side Effects: parse the argument
 * Reference: OSdev
 */
int parse_args(const int8_t *user_input, int8_t *parsed_fname, int8_t *parsed_args)
{
    if (NULL == user_input)
        return -1;

    int flag;
    int length;
    int i;
    memset(parsed_fname, 0, 50);
    memset(parsed_args, 0, 50);
    while (*user_input == ' ')
        user_input++;

    flag = 0;
    length = strlen((const int8_t *)user_input);
    for (i = 0; i < length; i++)
    {
        if (!flag)
        {
            if (*user_input != ' ')
                parsed_fname[i] = *(user_input++); // read next char
            else
                flag = 1;
        }
        else
        {
            while (*user_input == ' ')
                user_input++;
            *parsed_args = *user_input++;
            parsed_args++;
        }
    }

    return 0;
}

/* setup_paging_and_flush_tlb
 *
 * Inputs: physical_addr
 * Outputs: none
 * Side Effects: set up the paging for current process and flush tlb
 * Reference: OSdev
 */
void setup_paging_and_flush_tlb(int pid)
{
    int32_t physical_addr = pid + 2;
    page_dir_entry_u the_page_dir_entry;
    page_dir_entry_4MB_t *pde = &(the_page_dir_entry.kernel_page_desc);
    pde->present = 1; // in memory.
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
    pde->pg_addr = (uint32_t)physical_addr; // Bits 31-12 of address.

    // write into page_dir_base
    page_dir_base[128 / 4] = the_page_dir_entry;

    flush_tlb();
}

/*
 * init_file_table
 * Description: initialize the file table of PCB
 *  Inputs: PCB pointer
 *  Outputs: None
 * Side Effects: None.
 */
void init_file_table(PCB_t *pcb)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        switch (i)
        {
        case 0:
        {
            pcb->pcb_fds[i].op_ptr = (uint32_t)&stdin_ops;
            pcb->pcb_fds[i].inode = -1;   // does not matter
            pcb->pcb_fds[i].file_pos = 0; // does not matter
            pcb->pcb_fds[i].flags = IN_USE;
            break;
        }
        case 1:
        {
            pcb->pcb_fds[i].op_ptr = (uint32_t)&stdout_ops;
            pcb->pcb_fds[i].inode = -1;     // does not matter
            pcb->pcb_fds[i].file_pos = 0;   // does not matter
            pcb->pcb_fds[i].flags = IN_USE; // does not matter
            break;
        }

        default:
        {
            pcb->pcb_fds[i].flags = FREE; // does not matter
            // this struct will be initialized in open syscalls
            break;
        }
        }
    }
}

//! ===================================================================================

/*
 * schedule
 * Description: switch to the next process
 *  Inputs: None
 *  Outputs: None
 * Side Effects: None.
 */
void schedule(void)
{
    // declare local variables here
    terminal_t *next_terminal;
    int next_pid;
    PCB_t *next_pcb_ptr;

    //================== save "current" info =======================

    running_term_ptr->pid = cur_pid;

    if (cur_pid != -1)
    {
        cur_pcb_ptr->tss_esp0 = tss.esp0;
        asm volatile(
            "movl %%esp, %0     \n\t"
            "movl %%ebp, %1     \n\t"
            : "=r"(cur_pcb_ptr->esp), "=r"(cur_pcb_ptr->ebp));
    }

    //================== update "current" info =======================
    next_terminal = running_term_ptr->next_term_to_run;
    next_pid = next_terminal->pid;
    next_pcb_ptr = get_pcb_ptr(next_pid);
    running_term_ptr = next_terminal;

    cur_pcb_ptr = next_pcb_ptr;
    cur_pid = next_pid;

    //================== if there is no process in the new terminal, then start base shell =======================
    if (next_pid == -1)
    {
        launch_base_shell(next_terminal->tid);
        printf("This should not be printed!\n");
    }

    //================== set up new process paging =======================

    setup_paging_and_flush_tlb(next_pid);

    //================== restore next program's reg info =======================
    tss.ss0 = KERNEL_DS;
    tss.esp0 = next_pcb_ptr->tss_esp0;

    asm volatile(
        "movl %0, %%esp     \n\t"
        "movl %1, %%ebp     \n\t"
        :
        : "r"(next_pcb_ptr->esp), "r"(next_pcb_ptr->ebp));
    return;
}

/*
 * launch_base_shell
 * Description: launch a base shell
 *  Inputs: terminal id
 *  Outputs: None
 * Side Effects: None.
 */
void launch_base_shell(int32_t tid)
{
    printf("========>>> TERMINAL ID: %d                        \n", tid);
    syscall_execute((uint8_t *)"shell");
}
