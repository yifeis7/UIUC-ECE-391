#include "types.h"
#include "fs.h"

#ifndef _RTC_H
#define _RTC_H

#define RTC_IRQ 8
#define RTC_PORT 0x70       //RTC I/O port (Port 0x70 is used to specify an index or "register number", and to disable NMI)
#define RTC_DATA 0x71
#define CMOS_PORT 0x71      //CMOS I/O port
#define SEL_B_DIS_NMI 0x8B  //(0x80| 0x0B) (NMI_mask|RTC_status_register_B)
#define SEL_A_DIS_NMI 0x8A  //(0x80| 0x0A) (NMI_mask|RTC_status_register_A)
#define RTC_REG_C 0x0C      //RTC register C
//==========checkpoint2==========
#define NUM_BYTE 4  //required number of bytes
#define LOW_FREQUENCY 2 //lower bound of the rtc frequency
#define HIGH_FREQUENCY 1024 //high bound of the rtc frequency
#define HIGH_RATE 16  //16 is the upper bound of the low four bits of the register A. The low 4 bits of the register A is the divider value
// #define TEST_RTC 1;
#define RTC_CHAR_NUM 150 //number of characters to be printed on the screen

#ifndef _TEST_RTC_S
volatile int rtc_signal;
volatile int test_rtc;
volatile int interrupt_time_count;
#endif
//Initialize RTC
void rtc_init(void);

//handle the interrupt of RTC, run the test_interrupts test function
void rtc_int_handler(void);
int32_t rtc_read(file_entry* fp, void* buf, int32_t nbytes);
int32_t set_frequency(int32_t fre);
int32_t rtc_write(file_entry* fp, void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* fname);
int32_t rtc_close(file_entry* fp, void* buf, int32_t nbytes);
void rtc_pulse(int32_t seconds);


#endif
