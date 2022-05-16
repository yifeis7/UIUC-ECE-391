#include "interrupt_handler.h"
#include "keyboard.h"
#include "rtc.h"
#include "pit.h"
#include "lib.h"

/* Definitions for the interrupt handlers */

/*
    INT_keyboard:
    Input: None
    Output: None
    Side effects: call keyboard interrupt handler
*/
void INT_keyboard()
{   
    // printf("keyboard handler called!\n");
    keyboard_int_handler();  //handle keyboard interrupt
}

/*
    INT_rtc:
    Input: None
    Output: None
    Side effects: call rtc interrupt handler
*/
void INT_rtc()
{
    // printf("RTC handler called!\n");
    rtc_int_handler();  //handle RTC interrupt
}

/*
    INT_pit:
    Input: None
    Output: None
    Side effects: call pit interrupt handler
*/
void INT_pit()
{
    // printf("PIT handler called!\n");
    pit_int_handler();  //handle PIT interrupt
}
