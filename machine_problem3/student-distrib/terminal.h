#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "keyboard.h"
#include "syscall_handler.h"

#define TERM_NUM 3

/* 3 terminal video buffers */
#define VIDEO_START_TMN_1 (_128M + _4M)
#define VIDEO_START_TMN_2 (_128M + _4M + _4K)
#define VIDEO_START_TMN_3 (_128M + _4M + 2 * _4K)

/* always assign this value to vidmap_syscall, always points to the video memory when assigned */
#define FISH_VIDEO        (_128M + _4M + 3 * _4K)

/**
    * Write into the screen buffer:
    *     1. before terminal switching
    *     2. when the ACTIVE running process is not shown on the screen
    * 
    * Mapping:
    *     terminal one hidden vidmem: physical 0xC0000 - 0xC1000
    *     terminal one hidden vidmem: physical 0xC1000 - 0xC2000
    *     terminal one hidden vidmem: physical 0xC2000 - 0xC3000
    * 
    * modify `vidmap` system call, allocating different virtual address for different termial!
    *  
    */
// the unit of scheduling
typedef struct terminal_t {
    int tid;
    int pid;

    char kbd_buf[ARG_LEN];
    int buf_pos;
    volatile int kbd_buf_ready;

    int cursor_x;
    int cursor_y;

    uint32_t vid_mem_buffer;
    uint32_t vidmap_flag;

    struct terminal_t *next_term_to_run;
} terminal_t;

/* =========================== function declarations =========================== */

void terminal_init(void);
int terminal_open(const uint8_t* filename);
int terminal_close(const uint8_t* filename);
int terminal_read(int32_t fd, void* buf, int32_t nbytes);
int terminal_write(int32_t fd, void* buf, int32_t nbytes);
extern int32_t check_terminal_baseshell_pid(int32_t terminal_index);

void switch_terminal(int target_tid);

terminal_t *terminal_addr(int term_id);
extern int32_t term_id_2_video_start(int32_t term_id);

extern volatile int current_term_id;
extern terminal_t terminal_array[3];
extern int8_t pcb_record_for_each_terminal[3][5];  //record the pcb info for each terminal

extern terminal_t *running_term_ptr;
extern terminal_t *viewing_term_ptr;

extern void init_shell();
extern void copy_shell_img(int shell_pid);
extern void set_up_user_context(int shell_pid);
extern void init_shell_PCB(PCB_t *pcb, int term_id);
extern void change_vidmem_mapping(int tid);
extern void switch_fish_paging(int32_t tid, int32_t temp_flag);

#endif
