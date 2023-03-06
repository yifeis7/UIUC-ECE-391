#ifndef SYSCALL_HANDLER_H
#define SYSCALL_HANDLER_H

#include "types.h"
#include "fs.h"

//! -----------------------------------------------------------------------------------

#define MAX_NUM_PROCESS 6
#define PROGRAM_ENTRY_POINT 24

#define NONE 0      // no process
#define RUNNABLE 1  // process in the run queue
#define EXPIRED 2   // process used up its time slice, in the expired queue
#define SUSPENDED 3 // shell under user program

#define USER_START 0x8000000
#define USER_END 0x8400000
#define FISH_MAP 0x84b8000

#define ARG_LEN 128
#define FNAME_MAX_LEN 32
#define PROGRAM_VIR_ADDR 0x08048000

//! -----------------------------------------------------------------------------------

// PCB
typedef struct PCB
{
    //int32_t terminal_id;
    uint32_t parent_pid; // shell program
    uint32_t pid;        // the pid of this program
    uint32_t vidmap_flag;  //determine whether a process has get the vidmap return value

    // regs and stack info
    uint32_t esp;
    uint32_t ebp;
    uint32_t tss_esp0;

    int8_t args[ARG_LEN];

    // scheduling info
    uint32_t counts;
    uint8_t flag; // RUNNABLE, EXPIRED, etc

    file_entry pcb_fds[8]; // keep track of files open for this process

} PCB_t;

//! -----------------------------------------------------------------------------------

extern int32_t syscall_halt(uint8_t status);
extern int32_t syscall_execute(const uint8_t *command);
extern int32_t syscall_read(int32_t fd, void *buf, int32_t nbytes);
extern int32_t syscall_write(int32_t fd, void *buf, int32_t nbytes);
extern int32_t syscall_open(const uint8_t *filename);
extern int32_t syscall_close(int32_t fd);
extern int32_t syscall_getargs(uint8_t *buf, int32_t nbytes);
extern int32_t syscall_vidmap(uint8_t **screen_start);
extern int32_t syscall_sethandler(int32_t signum, void *handler_address);
extern int32_t syscall_sigreturn(void);

extern int parse_args(const int8_t *input_command, int8_t *args, int8_t *command);
extern void setup_paging_and_flush_tlb(int pid);
extern int32_t decord_process(int32_t pid);
extern PCB_t * get_pcb_ptr(int32_t pid);
extern int32_t record_process(void);
extern void init_file_table(PCB_t *pcb);
extern void schedule(void);
extern void launch_base_shell(int32_t tid);

//! -----------------------------------------------------------------------------------

extern int usr_programs_remaining; // decrement every usr programs (also shells but not base shells)
extern int cur_pid;                // scheduler should control this!
extern int8_t args[3][50];         // 3 user argument buffers
extern int8_t command[3][50];      // 3 command buffers
extern int8_t pcb_bitmap[MAX_NUM_PROCESS];

#endif
