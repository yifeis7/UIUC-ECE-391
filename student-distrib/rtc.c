#include "i8259.h"
#include "lib.h"
#include "rtc.h"


/* rtc_init
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: Initialize RTC, turn on IRQ8, mask NMI
 * Reference: OSdev
 */
void rtc_init(void)
{
    rtc_signal = 0;
    test_rtc = 0;
    interrupt_time_count = 0;
    uint8_t temp;
    temp = inb(RTC_PORT) | 0x80;  //0x80 is for set the first bit to 1
    outb(temp, RTC_PORT);         //write the value back to RTC port

    outb(SEL_B_DIS_NMI, RTC_PORT);      //select RTC register B and disable NMI
    char cur_val = inb(CMOS_PORT);	    // read the current value of register B
    outb(SEL_B_DIS_NMI, RTC_PORT);      //set the index again (a read resets the index to register D)
    outb(cur_val | 0x40, CMOS_PORT);	//turns on bit 6 of register B
    enable_irq(RTC_IRQ);                //IRQ8
    enable_irq(SLAVE_IRQ_LINE);         //IRQ2
    
    temp = inb(RTC_PORT) & 0X7F;        //0X7F set the first bit back to zero and unmask NMI
    outb(temp, RTC_PORT);

}

/* rtc_int_handler
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: handle the interrupt of RTC, run the test_interrupts test function
 * Reference: OSdev
 */
void rtc_int_handler(void){
    cli();  
    // if(test_rtc == 1){
    // // test_interrupts(); //test rtc_int_handler
	// 	nb_putc('1');
    // }
    interrupt_time_count ++;
    rtc_signal = 1;
    outb(RTC_REG_C, RTC_PORT);  //select register C
    inb(CMOS_PORT);    //throw away the contents
    send_eoi(RTC_IRQ);

    sti();
}


//=========================check point 2=====================================
/* rtc_read
 *
 * Inputs: fd -- file descripter
 *         buf -- the pointer to the frequency number
 *         nbytes -- returned by the function if the frequency is set successfully
 * Outputs: 0 for success, -1 for failure
 * Side Effects: block until the interrupt status is changed
 * Reference: OSdev
 */
int32_t rtc_read(file_entry* fp, void* buf, int32_t nbytes){
    // printf("Enter rtc read\n");
    // rtc_pulse(-1);
    rtc_signal = 0;
    while(rtc_signal != 1);
    rtc_signal = 0;
    return 0;

}

/* rtc_write
 *
 * Inputs: fd -- file descripter
 *         buf -- the pointer to the frequency number
 *         nbytes -- returned by the function if the frequency is set successfully
 * Outputs: 0 for success, -1 for failure
 * Side Effects: Change the frequency of the rtc 
 * Reference: OSdev
 */
int32_t rtc_write(file_entry* fp, void* buf, int32_t nbytes){
    if(nbytes != NUM_BYTE || buf ==0)return -1;  //if the number of bytes is not 4 or the buf is a NULL pointer, then fail
    int res;
    res = set_frequency(*(int32_t*)buf);  //set the frequency
    return res;
}


/* set_frequency
 *
 * Inputs: fre-- the frequency to be set of RTC
 * Outputs: 0 for success, -1 for failure
 * Side Effects: Change the frequency of the rtc 
 * Reference: OSdev
 */
int32_t set_frequency(int32_t fre){

    int log_fre = 0;
    uint8_t temp;  //temporal register
    uint8_t frame;  //specifing the input frequency
    //check bound and power of 2
    if (fre < LOW_FREQUENCY || fre > HIGH_FREQUENCY || ((fre - 1) & fre))
        return -1;
    while (fre >>= 1)
        log_fre += 1;    //logarithm of the input frequency
    frame = 16 - log_fre;  //get the low four bits of register A(reference: OSdev)

    temp = inb(RTC_PORT) | 0x80; //0x80 is for set the first bit to 1
    outb(temp, RTC_PORT);        //write the value back to RTC port

    outb(SEL_A_DIS_NMI, RTC_PORT); //select register A and disable NMI
    temp = inb(RTC_DATA);          //read from RTC data port
    outb(SEL_A_DIS_NMI, RTC_PORT); //select register A again
    outb((temp & 0xF0)| frame ,RTC_DATA); //set the rtc frequency

    temp = inb(RTC_PORT) & 0x7F; //set he first bit back to zero and undisable NMI
    outb(temp, RTC_PORT);
    return 0;
}
/* rtc_open
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: set the rtc to default (2 Hz) frequency
 * Reference: OSdev
 */
int32_t rtc_open(const uint8_t* fname){
    // printf("Enter rtc open\n");
    set_frequency(LOW_FREQUENCY); //set the default frequency
    return 0;
}
/* rtc_close
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: set the rtc to default (2 Hz) frequency
 * Reference: OSdev
 */
int32_t rtc_close(file_entry* fp, void* buf, int32_t nbytes){
    // set_frequency(LOW_FREQUENCY);
    return 0;
}
/* rtc_pulse
 *
 * Inputs: None
 * Outputs: None
 * Side Effects: sleep for a specified period of time
 * Reference: OSdev
 */
void rtc_pulse(int32_t seconds){
    int fre = LOW_FREQUENCY;
    test_rtc = 0;
    set_frequency(fre);
    interrupt_time_count = 0;
    if(seconds != -1){
        while(interrupt_time_count <= seconds*2);
    }
    else{
        while(interrupt_time_count <= 1);
    }
}



