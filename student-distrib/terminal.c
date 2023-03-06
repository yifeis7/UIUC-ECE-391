#include "terminal.h"
#include "lib.h"
#include "syscall_handler.h"
#include "paging.h"
#include "i8259.h"
#include "keyboard.h"
#include "x86_desc.h"

// terminal_t terminal_array[3];
// volatile int current_term_id = 0;
// int8_t pcb_record_for_each_terminal[3][5];
static terminal_t term_arr[3];

terminal_t *running_term_ptr = &(term_arr[0]);
terminal_t *viewing_term_ptr = &(term_arr[0]);

// extern int32_t ece391_execute(uint8_t *command);

/*
 * terminal_init
 * Description: Initialize the terminal.
 *  Inputs: None
 *  Outputs: None
 * Side Effects: None.
 */
void terminal_init(void)
{
    // int i;
    // // int j;
    // for (i = 0; i < 6; i++)
    //     pcb_bitmap[i] = NONE;
    // // set_paging_multi_terminal();
    // for (i = 0; i < 3; i++) {
    //     terminal_array[i].valid = 1;  //check whether there is base shell running in the terminal
    //     // for(j = 0; j < 5; j++){
    //     //     pcb_record_for_each_terminal[i][j] = -1;
    //     // }
    //     // terminal_array[i].num_char_in_buf = 0;
    //     // terminal_array[i].num_char_on_screen = 0;
    //     // memset(terminal_array[i].kbd_buf, 0, 128);
    // }

    // this call will create PCB0 for base shell, write screen content on VIDEO_MEMORY
    // write global variables (i.e. cursor info, keyboard buffer, all flags, etc)

    int i;
    for (i = 0; i < 3; i++)
    {
        term_arr[i].tid = i;
        term_arr[i].pid = -1;
        term_arr[i].buf_pos = 0;
        term_arr[i].kbd_buf_ready = 0;
        term_arr[i].cursor_x = 0;
        term_arr[i].cursor_y = 0;
        term_arr[i].vidmap_flag = 0;
    }

    flush_tlb();

    kbd_buf = term_arr[0].kbd_buf;
    buf_pos = 0;
    screen_x_ptr = &term_arr[0].cursor_x;
    screen_y_ptr = &term_arr[0].cursor_y;

    term_arr[0].vid_mem_buffer = VIDEO_START + _4K;
    term_arr[1].vid_mem_buffer = VIDEO_START + _8K;
    term_arr[2].vid_mem_buffer = VIDEO_START + _12K;

    term_arr[0].next_term_to_run = &(term_arr[1]);
    term_arr[1].next_term_to_run = &(term_arr[2]);
    term_arr[2].next_term_to_run = &(term_arr[0]);
}

/*
 * terminal_open
 * Description: Open the terminal.
 *  Inputs:
 *      - filename: file name
 *  Outputs: 0
 * Side Effects: None.
 */
int terminal_open(const uint8_t *filename)
{
    // do nothing
    return 0;
}

/*
 * terminal_close
 * Description: Close the terminal.
 *  Inputs:
 *      - filename: file name
 *  Outputs: 0
 * Side Effects: None.
 */
int terminal_close(const uint8_t *filename)
{
    // do nothing
    return 0;
}

/*
 * terminal_read
 * Description: Reads FROM the keyboard buffer into buf, return number of bytes read.
 *  Inputs:
 *      - fd: not used yet
 *      - buf: buffer to read char into
 *      - nbytes: size to read
 *  Outputs: # of byte read
 * Side Effects: change "kbd_buf_ready".
 */
int terminal_read(int32_t fd, void *buf, int32_t nbytes)
{
    // sanity check
    if (buf == NULL || nbytes <= 0)
    {
        // printf("terminal read fails!\n");
        return -1;
    }
    // please go to "keyboard.c"
    return read_from_kbd_buf_to_buf(fd, buf, nbytes);
}

/*
 * terminal_write
 * Description: Writes TO the screen from buf, return number of bytes written or -1.
 *  Inputs:
 *      - fd: not used yet
 *      - buf: buffer to write char into
 *      - nbytes: size to write
 *  Outputs: # of byte written
 * Side Effects: None.
 */
int terminal_write(int32_t fd, void *buf, int32_t nbytes)
{
    // sanity check
    if (buf == NULL || nbytes <= 0)
    {
        // printf("terminal write fail!\n");
        return -1;
    }

    int i;
    for (i = 0; i < nbytes; i++)
        nb_putc(((char *)buf)[i]);

    // return the number of bytes written
    return i;
}
/*
 * switch_terminal
 * Description: switch the terminal
 *  Inputs:
 *      -target_tid
 *  Outputs: none
 * Side Effects: None.
 */
void switch_terminal(int target_tid)
{
    if (target_tid == viewing_term_ptr->tid)
        return;

    terminal_t *target_term_ptr = &(term_arr[target_tid]);

    // save things
    viewing_term_ptr->buf_pos = buf_pos;

    // restore things
    kbd_buf = target_term_ptr->kbd_buf;
    buf_pos = target_term_ptr->buf_pos;

    update_screen_xy_ptr(&(target_term_ptr->cursor_x), &(target_term_ptr->cursor_y));

    // switch video memory content
    change_vidmem_mapping(viewing_term_ptr->tid);
    memcpy((void *)(viewing_term_ptr->vid_mem_buffer), (void *)VIDEO_START, _4K);
    memcpy((void *)VIDEO_START, (void *)(target_term_ptr->vid_mem_buffer), _4K);
    viewing_term_ptr = &(term_arr[target_tid]);
    change_vidmem_mapping(running_term_ptr->tid);
}

/*
 * change_vidmem_mapping
 * Description: change the video memory mapping
 *  Inputs:
 *      - tid: terminal id
 *  Outputs: None
 * Side Effects: None.
 */
void change_vidmem_mapping(int tid)
{
    int32_t temp_flag;

    screen_x_ptr = &term_arr[tid].cursor_x;
    screen_y_ptr = &term_arr[tid].cursor_y;
    temp_flag = (tid != viewing_term_ptr->tid);
    switch_fish_paging(tid, temp_flag);
    flush_tlb();
}

/*
 * switch_fish_paging
 * Description: switch the paging of the video memory
 *  Inputs:
 *      -tid
 *      -temp_flag
 *  Outputs: none
 * Side Effects: None.
 */
void switch_fish_paging(int32_t tid, int32_t temp_flag)
{
    int32_t target_place;

    target_place = VIDEO_START >> 12;

    if (temp_flag)
    {
        usr_page_table_base[target_place].pg_addr = term_arr[tid].vid_mem_buffer >> 12;
        usrmap_page_table_base[target_place].present = term_arr[tid].vidmap_flag;
        usrmap_page_table_base[target_place].pg_addr = term_arr[tid].vid_mem_buffer >> 12;
    }
    else
    {
        usr_page_table_base[target_place].pg_addr = target_place;
        usrmap_page_table_base[target_place].present = term_arr[tid].vidmap_flag;
        usrmap_page_table_base[target_place].pg_addr = target_place;
    }
}
