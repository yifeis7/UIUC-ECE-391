#include "pit.h"
#include "i8259.h"
#include "syscall_handler.h"
#include "terminal.h"
#include "lib.h"

/*
 * pit_init
 * Description: initialize the pit
 *  Inputs: None
 *  Outputs: None
 * Side Effects: None.
 */
void pit_init(void)
{
    // set Mode/Command register
    /*
    INFO about the Mode/Command register (* means we select it)
        Bits         Usage
        6 and 7      Select channel :
                        0 0 = Channel 0 (*)
                        0 1 = Channel 1
                        1 0 = Channel 2
                        1 1 = Read-back command (8254 only)

                        where:
                        - Channel 0 is connected directly to IRQ0,
                            so it is best to use it only for purposes that should generate interrupts.
                        - Channel 1 is unusable, and may not even exist.
                        - Channel 2 is connected to the PC speaker, but can be used for other purposes without producing audible speaker tones.

        4 and 5      Access mode :
                        0 0 = Latch count value command
                        0 1 = Access mode: lobyte only
                        1 0 = Access mode: hibyte only
                        1 1 = Access mode: lobyte/hibyte (*)

                        where:
                        - "lobyte only": only the lowest 8 bits of the counter value is read or written to/from the data port.
                        - "hibyte only": only the highest 8 bits of the counter value is read or written.
                        - "lobyte/hibyte": 16 bits are always transferred as a pair,
                            with the lowest 8 bits followed by the highest 8 bits
                                (both 8 bit transfers are to the same IO port, sequentially – a word transfer will not work).

        1 to 3       Operating mode :
                        0 0 0 = Mode 0 (interrupt on terminal count)
                        0 0 1 = Mode 1 (hardware re-triggerable one-shot)
                        0 1 0 = Mode 2 (rate generator)
                        0 1 1 = Mode 3 (square wave generator) (*)
                        1 0 0 = Mode 4 (software triggered strobe)
                        1 0 1 = Mode 5 (hardware triggered strobe)
                        1 1 0 = Mode 2 (rate generator, same as 010b)
                        1 1 1 = Mode 3 (square wave generator, same as 011b)

        0            BCD/Binary mode:
                        0 = 16-bit binary
                        1 = four-digit BCD (*)

        Therefore, we are going to send 0011 0111 (0x37) to PIT Mode/Command register
    */
    // init_shell();
    outb(0x37, PIT_CMD_REG); // 0x43

    outb((uint8_t)_100HZ, PIT_CHNL_0_PORT);        // 0x40
    outb((uint8_t)(_100HZ >> 8), PIT_CHNL_0_PORT); // 0x41

    enable_irq(PIT_IRQ);
}

/*
 * pit_int_handler
 * Description: pit interrupts
 *  Inputs:
 *  Outputs: None
 * Side Effects: None.
 */
void pit_int_handler(void)
{
    cli();
    send_eoi(PIT_IRQ);
    change_vidmem_mapping(running_term_ptr->next_term_to_run->tid);
    schedule();
    sti();
}
