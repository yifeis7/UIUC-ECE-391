#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"

// which IRQ the keyboard is connected to
#define KEYBOARD_IRQ 1

// scan codes
#define NUMBER_START_IDX            0x02
#define NUMBER_END_IDX              0x0B

#define Q_IDX                       0x10
#define P_IDX                       0x19
#define A_IDX                       0x1E
#define L_IDX                       0x26
#define Z_IDX                       0x2C
#define M_IDX                       0x32

#define LEFT_CTRL_PRESSED_IDX       0x1D
#define RIGHT_CTRL_PRESSED_IDX      0xED
#define CTRL_RELEASED_IDX           0x9D

#define LEFT_SHIFT_PRESSED_IDX      0x2A
#define LEFT_SHIFT_RELEASED_IDX     0xAA

#define RIGHT_SHIFT_PRESSED_IDX     0x36
#define RIGHT_SHIFT_RELEASED_IDX    0xB6

#define ALT_PRESSED_IDX             0x38
#define ALT_RELEASED_IDX            0xB8

#define SPACE_PRESSED_IDX           0x39
#define SPACE_RELEASED_IDX          0xB9

#define CAPSLOCK_PRESSED_IDX        0x3A
#define CAPSLOCK_RELEASED_IDX       0XBA

#define BACKSPACE_PRESSED_IDX       0x0E
#define BACKSPACE_RELEASED_IDX      0x8E

#define ENTER_PRESSED_IDX           0x1C
#define ENTER_RELEASED_IDX          0x9C

#define TAB_PRESSED_IDX             0x0F
#define TAB_RELEASED_IDX            0x8F

#ifndef _F
#define _F
#define F1_PRESSED_IDX              0x3B
#define F1_RELEASED_IDX             0xBB

#define F2_PRESSED_IDX              0x3C
#define F2_RELEASED_IDX             0XBC

#define F3_PRESSED_IDX              0x3D
#define F3_RELEASED_IDX             0xBD
#endif

// keyboard ports
#define KBD_DATA_PORT   0x60
#define KBD_STATUS_PORT 0x64

#define KBD_READ        0x0
#define KBD_WRITE       0x1

// others
#define SCAN_CODE_SET_SIZE  59
#define KBD_BUF_SIZE        128

/* =========================== function declarations =========================== */

extern char *kbd_buf;
extern uint8_t buf_pos;

// keyboard initialization
void keyboard_init(void);
// read a character from keyboard and put it
void keyboard_int_handler(void);
// helper function
int check_and_handle_special_button(unsigned char input_code);
// read keyboard buffer
int read_from_kbd_buf_to_buf(int32_t fd, void *buf, int32_t nbytes);

/* scan code set */
extern char scan_code_set_1[SCAN_CODE_SET_SIZE][2];

/* keyboard buffer */
// extern char kbd_buf[3][KBD_BUF_SIZE];
// extern int num_char_in_buf[3]; // indicates the number of non-garbage characters in the keyboard buffer, which cannot be more than (KBD_BUF_SIZE-1)
// extern int num_char_on_screen[3]; // indicates the number of characters printed on the screen before an enter is pressed
// extern volatile int kbd_buf_ready[3];

#endif
