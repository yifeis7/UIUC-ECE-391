/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)
#define half_byte_mask = 0x0F  //extract the left most bits

int tux_set_led(struct tty_struct* tty, unsigned long val_led);
//The flag which represent the change of MTCP_ACK(acknowledge)	
static int ACK = 0;
static spinlock_t t_lock = SPIN_LOCK_UNLOCKED;  //initialize the spinlock for tux
//seven segment display vector for each hex number 
//reference:
/*; 	__7___6___5___4____3___2___1___0__
 		| A | E | F | dp | G | C | B | D | 
 		+---+---+---+----+---+---+---+---+   */
unsigned char led_vec_predefine[16] = {0xE7, 0x06, 0xCB, 0x8F,0x2E, 0xAD, 0xED, 0x86, 0xEF, 0xAE, 0xEE, 0x6D, 0xE1, 0x4F, 0xE9, 0xE8};  
static unsigned char half_byte_bitwise_mask[4] = {1,2,4,8};  //mask array for judging whether a place should display dot point
static unsigned char led_info[6]; //led buffer for storing opcode, local variable, etc
static unsigned char init_info[2];  //init buffer for the TUX_INIT function
static unsigned int button_bits;  //take the input botton vector
static unsigned long val_led;  //32bit argument for tux_set_led

static unsigned int int_byte_mask = 0x0F;
static unsigned int int_bitwise_mask[4] = {1,2,4,8};  //mask array for judging whether a place should display dot point
static unsigned long long_byte_mask = 0x0F;



/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned int flags;


    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];
	switch(a){
		case MTCP_ACK:
			spin_lock_irqsave(&t_lock, flags);
			ACK = 1;
			spin_unlock_irqrestore(&t_lock, flags);
			return;
		case MTCP_RESET:
			//first initialize the tux
			init_info[0] = MTCP_BIOC_ON; //Enable Button interrupt-on-change
			init_info[1] = MTCP_LED_USR; //initialize the led into user mode
			tuxctl_ldisc_put(tty, init_info, 2); //2 is the length of the init_info vector
			//recover the led status
			tux_set_led(tty, val_led);  
			return;
		case MTCP_BIOC_EVENT:
/*	Packet format:
	Byte 0 - MTCP_BIOC_EVENT
	byte 1  +-7-----4-+-3-+-2-+-1-+---0---+
		| 1 X X X | C | B | A | START |
		+---------+---+---+---+-------+
	byte 2  +-7-----4-+---3---+--2---+--1---+-0--+
		| 1 X X X | right | down | left | up |
		+---------+-------+------+------+----+*/
			spin_lock_irqsave(&t_lock, flags);
			button_bits = (b & int_byte_mask); //C B A START
			button_bits += (c & int_bitwise_mask[0]) << 4 ; //up
			button_bits += (c & int_bitwise_mask[2]) << 3 ; //down
			button_bits += (c & int_bitwise_mask[1]) << 5 ; //left
			button_bits += (c & int_bitwise_mask[3]) << 4 ; //right
			button_bits = ~button_bits;
			spin_unlock_irqrestore(&t_lock, flags);
			return;
		default:
			return;
		}

	

    /*printk("packet : %x %x %x\n", a, b, c); */

}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
	int ret_val;
	unsigned int flags;
    switch (cmd) {
	case TUX_INIT:{
		ACK = 0;                     //clear the MTCP_ACK shadow flag
		button_bits = 0;             //clear the button info bits
		t_lock = SPIN_LOCK_UNLOCKED; //initialize the lock
		init_info[0] = MTCP_BIOC_ON; //Enable Button interrupt-on-change
		init_info[1] = MTCP_LED_USR; //initialize the led into user mode
		tuxctl_ldisc_put(tty, init_info, 2); //2 is the length of the init_info vector
		return 0;
	}
	case TUX_BUTTONS:
		if(arg == 0)return -EINVAL;
		spin_lock_irqsave(&t_lock, flags);
		ret_val = copy_to_user((void*)arg, &button_bits, 4);  //4 means 4 bytes, button_bits is 32-bit value
		spin_unlock_irqrestore(&t_lock, flags);
		if(ret_val == 0)return -EINVAL;
		return 0;
	case TUX_SET_LED:
		if(ACK == 0)return -EINVAL;
		//initialize the led value
		//copy the argument into variable
		val_led = arg;
		//call the function and pass the led value into tux
		ret_val = tux_set_led(tty, val_led);
		//after finishing tux_set_led, clear ACK
		ACK = 0;
		if(ret_val != 0)ret_val = -EINVAL;
		return ret_val;
	
	case TUX_LED_ACK:
		return 0;
	case TUX_LED_REQUEST:
		return 0;
	case TUX_READ_LED:
		return 0;
	default:
	    return -EINVAL;
    }
}

/*
 * tux_set_led
 *   DESCRIPTION: put relative data from input bit vector to tux led
 *   INPUTS: tty     - tty port 
 * 			 val_led - input led value
 *   OUTPUTS: 0 for success, other for failure
 *   RETURN VALUE: 0 for success, other for failure
 *   SIDE EFFECTS: put relative data from input bit vector to tux led
 */
int tux_set_led(struct tty_struct* tty, unsigned long val_led){
	
	unsigned char led_arg_vec[4];  //divid the led part of arg into four segments and store them into a vector
	unsigned char dotpoint_flag[4];  //check whether the four numbers need to add dot points
	int i;	//shift and copy the arg bits into led_val_vec
	int ret_val;

	led_info[0] = MTCP_LED_SET;  //opcode specific for this led_set operation
	led_info[1] = 0x0f;  //these four bit "1"(0x0f) mean that all the leds are lighted
	

	//store all four led values into the led_arg_vec
	for(i=0; i < 4; i++){
		led_arg_vec[i] = (unsigned char)((val_led >> (4*i)) & long_byte_mask);  //4*i is for different shift length for four leds;
	}
	//store all four flags for dot point in the array
	for(i=0; i < 4; i++){
		dotpoint_flag[i] = 1 - (((val_led>>24) & (half_byte_bitwise_mask[i])) == 0);  //arg shifts 12 bit, so the last four bits are flags for dot points
	}
	for(i=0; i < 4; i++){
		//shift 16 for letting "showing which led " flags be the first four bits
		if(((val_led>>16) & half_byte_bitwise_mask[i]) == 0){
			led_arg_vec[i] = 0x00; //if do not display, then set led_i as zero
			led_info[i+2] = led_arg_vec[i]; //i+2 is for the first two bits are witten 
		}
		else{
			int segment_val = led_vec_predefine[led_arg_vec[i]]; //write 7-segment value into the temp variable
			led_arg_vec[i] = (dotpoint_flag[i] <<4) + segment_val;  //add 7-segment value to dot point flag; 4 is for shifting the dot flag to right place in 7-segment bit vector
			led_info[i+2] = led_arg_vec[i]; //i+2 is for the first two bits are witten 
		}
	}
	ret_val = tuxctl_ldisc_put(tty, led_info, 6); //6 is length of input buffer
	return ret_val;
}
