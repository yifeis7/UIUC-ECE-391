/* Common entry for all the interrupt handlers*/
#define ASM 1
#include "interrupt_handler_entries.h"

.extern     INT_keyboard, INT_rtc, INT_pit
.globl      keyboard_entry, rtc_entry, pit_entry
.align      4

/* the entry of keyboard interrupt handler
    Input: None
    Output: None
    Side effect: call keyboard interrupt handler
 */
keyboard_entry: 
    push %eax
    push %ebx
    push %ecx
    push %edx
    push %edi
    push %esi  
    cld 
    call INT_keyboard  # call the keyboard interrupt handler
    pop %esi 
    pop %edi 
    pop %edx 
    pop %ecx 
    pop %ebx 
    pop %eax
    iret

/* the entry of RTC interrupt handler
    Input: None
    Output: None
    Side effect: call RTC interrupt handler
 */
rtc_entry:
    push %eax
    push %ebx
    push %ecx
    push %edx
    push %edi
    push %esi  
    cld
    call INT_rtc      # call the rtc interrupt handler
    pop %esi 
    pop %edi 
    pop %edx 
    pop %ecx 
    pop %ebx 
    pop %eax
    iret

/* the entry of PIT interrupt handler
    Input: None
    Output: None
    Side effect: call RTC interrupt handler
 */
pit_entry:
    push %eax
    push %ebx
    push %ecx
    push %edx
    push %edi
    push %esi  
    cld
    call INT_pit      # call the rtc interrupt handler
    pop %esi 
    pop %edi 
    pop %edx 
    pop %ecx 
    pop %ebx 
    pop %eax
    iret

.end
