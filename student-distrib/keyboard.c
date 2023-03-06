#include "keyboard.h"
#include "terminal.h"
#include "i8259.h"
#include "lib.h"
#include "syscall_handler.h"

/* special buttons */
static int left_shift_flag = 0;
static int right_shift_flag = 0;
static int ctrl_flag = 0;
static int capslock_flag = 0;
static int alt_flag = 0;

/* keyboard buffer */
// char kbd_buf[3][KBD_BUF_SIZE];   
// int num_char_in_buf[3]; // indicates the number of non-garbage characters in the keyboard buffer, which cannot be more than (KBD_BUF_SIZE-1)
// int num_char_on_screen[3]; // indicates the number of characters printed on the screen before an enter is pressed
// volatile int kbd_buf_ready[3];
// char kbd_buf[KBD_BUF_SIZE];
// int num_char_in_buf;
// int num_char_on_screen;
// volatile int kbd_buf_ready;
char *kbd_buf = NULL;
uint8_t buf_pos = 0;

/* we are using scan code set 1 */
char scan_code_set_1[][2] =
    {
        // from left to right: normal, shift + normal
        {0x0, 0x0},
        {0x0, 0x0},

        {'1', '!'},
        {'2', '@'},
        {'3', '#'},
        {'4', '$'},
        {'5', '%'},
        {'6', '^'},
        {'7', '&'},
        {'8', '*'},
        {'9', '('},
        {'0', ')'},

        {'-', '_'},
        {'=', '+'},

        {'\b', '\b'}, // Backspace
        {'\t', '\t'}, // Tab

        {'q', 'Q'},
        {'w', 'W'},
        {'e', 'E'},
        {'r', 'R'},
        {'t', 'T'},
        {'y', 'Y'},
        {'u', 'U'},
        {'i', 'I'},
        {'o', 'O'},
        {'p', 'P'},

        {'[', '{'},
        {']', '}'},

        {'\n', '\n'}, // Enter 
        {'0', '0'},   // Left ctrl 

        {'a', 'A'},
        {'s', 'S'},
        {'d', 'D'},
        {'f', 'F'},
        {'g', 'G'},
        {'h', 'H'},
        {'j', 'J'},
        {'k', 'K'},
        {'l', 'L'},

        {';', ':'},
        {'\'', '\"'},
        {'`', '~'},

        {'0', '0'}, // left shift

        {'\\', '|'},

        {'z', 'Z'},
        {'x', 'X'},
        {'c', 'C'},
        {'v', 'V'},
        {'b', 'B'},
        {'n', 'N'},
        {'m', 'M'},
        {',', '<'},
        {'.', '>'},
        {'/', '?'},

        {'0', '0'}, // right shift

        {'*', '*'}, // keypad "*" 

        {'0', '0'}, // left alt

        {' ', ' '}, // space

        {'0', '0'}, // capslock
};

/* keyboard_init
 * Description: Initialize keyboard.
 * Inputs: None
 * Outputs: None
 * Side Effects: Initialize keyboard.
 */
void keyboard_init(void)
{
    left_shift_flag = 0;  // released
    right_shift_flag = 0; // released
    ctrl_flag = 0;        // released
    capslock_flag = 0;    // released
    alt_flag = 0;         // released

    // int i;
    // for(i = 0; i < 3; i++){
    //     num_char_in_buf[i] = 0;
    //     num_char_on_screen[i] = 0;
    //     kbd_buf_ready[i] = 0;

    // }
    // num_char_in_buf = 0;
    // num_char_on_screen = 0;
    // kbd_buf_ready = 0;

    enable_irq(KEYBOARD_IRQ);
}

/* keyboard_int_handler
 * Description: Read from keyboard input and display to screen.
 * Inputs: None
 * Outputs: None
 * Side Effects: Send command to keyboard to read from it; clear and reset the interrupt enabling flag; print a character onto screen; change "kbd_buf_ready".
 */
void keyboard_int_handler(void)
{
    cli();
    // int terminal_index = ((PCB_t *)get_PCB_addr(cur_pid))->terminal_id;
    // int terminal_index = current_term_id;
    // int terminal_index = current_term_id;
    outb(KBD_READ, KBD_STATUS_PORT);
    unsigned char input_code = inb(KBD_DATA_PORT);

    if (check_and_handle_special_button(input_code))
    {
        send_eoi(KEYBOARD_IRQ);
        sti();
        return;
    }
    else
    {
        // only normal keys and "enter" go here; otherwise, bad input
        if (0x02 <= input_code && input_code <= SCAN_CODE_SET_SIZE - 1)
        {
            if (ctrl_flag && scan_code_set_1[input_code][0] == 'l')
            {
                // Ctrl + l: clear the screen and put the cursor at the top
                clear();
                ctrl_flag = 0;
            }
            else
            {
                char what_to_put;
                int should_modify;

                // 26 letters
                if ((Q_IDX <= input_code && input_code <= P_IDX) ||
                    (A_IDX <= input_code && input_code <= L_IDX) ||
                    (Z_IDX <= input_code && input_code <= M_IDX))
                {
                    should_modify = (left_shift_flag || right_shift_flag) ^ capslock_flag;
                    what_to_put = scan_code_set_1[input_code][should_modify];
                }
                // numbers, operators, punctuations, space
                else
                {
                    should_modify = left_shift_flag || right_shift_flag;
                    what_to_put = scan_code_set_1[input_code][should_modify];
                }

                change_vidmem_mapping(viewing_term_ptr->tid);
                nb_putc(what_to_put);
                calib_cursor();
                change_vidmem_mapping(running_term_ptr->tid);

                // num_char_on_screen[terminal_index]++;

                if (buf_pos <= KBD_BUF_SIZE - 2) // the last character in the buffer should be reserved for '\n'
                {
                    kbd_buf[buf_pos] = what_to_put;
                    if (what_to_put == '\n')
                    {
                        // num_char_in_buf[terminal_index] = 0;
                        // num_char_on_screen[terminal_index] = 0;
                        // kbd_buf_ready[terminal_index] = 1;
                        buf_pos = 0;
                        viewing_term_ptr->kbd_buf_ready = 1;
                    }
                    else
                    {
                        // num_char_in_buf[terminal_index]++;
                        buf_pos++;
                    }
                }
                else if ((buf_pos == KBD_BUF_SIZE - 1) && (what_to_put == '\n'))    // the buffer is filled up, with the last character being '\n'
                {
                    // kbd_buf[terminal_index][num_char_in_buf[terminal_index]] = '\n';
                    // num_char_in_buf[terminal_index] = 0;
                    // num_char_on_screen[terminal_index] = 0;
                    // kbd_buf_ready[terminal_index] = 1;
                    kbd_buf[buf_pos] = '\n';
                    buf_pos = 0;
                    viewing_term_ptr->kbd_buf_ready = 1;
                }
            }
        }

        send_eoi(KEYBOARD_IRQ); // when a release button is pressed, send EOI directly
        sti();
        return;
    }
}

/*
 * read_from_kbd_buf_to_buf
 * Description: read certain number of characters from keyboard buffer, and store them into buf.
 *  Inputs:
 *      - fd: not used yet
 *      - buf: buffer to read characters into
 *      - nbytes: size to read
 *  Outputs: # of byte read
 * Side Effects: change "kbd_buf_ready".
 */
int read_from_kbd_buf_to_buf(int32_t fd, void *buf, int32_t nbytes)
{
    int i;
    int nbytes_read;
    char read_char;
    // int terminal_index = ((PCB_t *)get_PCB_addr(cur_pid))->terminal_id;
    // int terminal_index = current_term_id;

    // sanity check
    if (buf == NULL || nbytes <= 0)
        return -1;
    // memset((void *)buf, 0, 1024);
    // wait until user presses "enter"
    while (!(running_term_ptr->kbd_buf_ready))
    {
    }

    // start reading from keyboard buffer
    nbytes_read = 0;
    for (i = 0; i < nbytes; i++)
    {
        // if we are gonna read more than 128 characters, we only read the first 128 characters
        if (i > KBD_BUF_SIZE - 1)
            break;

        read_char = kbd_buf[i];

        // store the character into buf
        ((char *)buf)[i] = read_char;
        nbytes_read++;

        // if we reach '\n', we should stop (there is always a '\n' in the keyboard buffer)
        if (read_char == '\n')
            break;
    }
    // memset((void *)kbd_buf[terminal_index], 0, 128);
    
    // printf("The buffer return is %s\n", (char *)buf);
    // printf("Number of bytes read is %d\n", nbytes_read);

    // kbd_buf_ready[terminal_index] = 0;
    running_term_ptr->kbd_buf_ready = 0;
    return nbytes_read;
}


/* check_and_handle_special_button
 * Description: check if input code is any of the special key; if so, flip the corresponding flag or do the correponding job.

 *** special keys include:
    - CTRL
    - shift
    - capslock
    - backspace
    - ALT (TODO): reserved for switching terminal
    - TAB (TODO): reserved for auto-complete

 * Inputs:
        - input_code: the scan code read from keyboard
 * Outputs:
        - whether a special button is pressed: 1 means yes, 0 means no
 * Side Effects: Send command to keyboard to read from it; clear and reset the interrupt enabling flag; print a character onto screen.
 */
int check_and_handle_special_button(unsigned char input_code)
{
    switch (input_code)
    {
    case LEFT_CTRL_PRESSED_IDX:
        ctrl_flag = 1;
        return 1;

    case RIGHT_CTRL_PRESSED_IDX:
        ctrl_flag = 1;
        return 1;

    case CTRL_RELEASED_IDX:
        ctrl_flag = 0;
        return 1;

    case LEFT_SHIFT_PRESSED_IDX:
        left_shift_flag = 1;
        return 1;

    case LEFT_SHIFT_RELEASED_IDX:
        left_shift_flag = 0;
        return 1;

    case RIGHT_SHIFT_PRESSED_IDX:
        right_shift_flag = 1;
        return 1;

    case RIGHT_SHIFT_RELEASED_IDX:
        right_shift_flag = 0;
        return 1;

    case CAPSLOCK_PRESSED_IDX:
        capslock_flag = 1 - capslock_flag;
        return 1;

    case CAPSLOCK_RELEASED_IDX:
        return 1;

    case BACKSPACE_PRESSED_IDX:{
        if (buf_pos > 0)
        {
            // nb_putc('\b);
            change_vidmem_mapping(viewing_term_ptr->tid);
            nb_putc('\b');
            calib_cursor();
            change_vidmem_mapping(running_term_ptr->tid);

            // if (num_char_on_screen[terminal_index] <= KBD_BUF_SIZE - 1){
            //     num_char_in_buf[terminal_index]--;
            // }
            // num_char_on_screen[terminal_index]--;
            buf_pos--;
        }
        return 1;
    }

    case BACKSPACE_RELEASED_IDX:
        return 1;

    case ALT_PRESSED_IDX:
        alt_flag = 1;
        return 1;

    case ALT_RELEASED_IDX:
        alt_flag = 0;
        return 1;

    case TAB_PRESSED_IDX:
        return 1;

    case TAB_RELEASED_IDX:
        return 1;

    case F1_PRESSED_IDX:
        if (alt_flag)
            switch_terminal(0);
        return 1;
    
    case F1_RELEASED_IDX:
        return 1;

    case F2_PRESSED_IDX:
        if (alt_flag)
            switch_terminal(1);
        return 1;

    case F2_RELEASED_IDX:
        return 1;

    case F3_PRESSED_IDX:
        if (alt_flag)
            switch_terminal(2);
        return 1;

    case F3_RELEASED_IDX:
        return 1;

    default:
        return 0;
    }
}

