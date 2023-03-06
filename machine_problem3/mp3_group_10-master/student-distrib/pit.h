#ifndef _PIT_H
#define _PIT_H

#define PIT_IRQ 0
#define _100HZ 11932

/*
    I/O port     Usage
    0x40         Channel 0 data port (read/write)
    0x41         Channel 1 data port (read/write)
    0x42         Channel 2 data port (read/write)
    0x43         Mode/Command register (write only, a read is ignored)
*/
#define PIT_CHNL_0_PORT 0x40
#define PIT_CHNL_1_PORT 0x41
#define PIT_CHNL_2_PORT 0x42
#define PIT_CMD_REG     0x43    // Mode/Command register

void pit_init(void);
void pit_int_handler(void);

#endif
