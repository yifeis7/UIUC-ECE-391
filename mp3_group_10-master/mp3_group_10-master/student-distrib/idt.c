#include "idt.h"
#include "x86_desc.h"
#include "exception_handler_entries.h"
#include "interrupt_handler_entries.h"


extern void syscall_entry();

/* idt_init
 * Description: Enable (unmask) the specified IRQ.
 * Inputs: 
        - irq_num: the IRQ to be enabled.
 * Outputs: None
 * Side Effects: Enable (unmask) the specified IRQ.
 */
void idt_init()
{
    int i;
    for (i = 0; i < NUM_VEC; i++)  // 256 times
    {
        idt_desc_t the_idt_desc;

        // common
        the_idt_desc.dpl = 0;
        the_idt_desc.present = 1;
        the_idt_desc.reserved0 = 0;
        the_idt_desc.reserved1 = 1;
        the_idt_desc.reserved2 = 1;
        the_idt_desc.reserved3 = 0;
        the_idt_desc.reserved4 = 0;
        the_idt_desc.size = 1;
        the_idt_desc.seg_selector = KERNEL_CS;

        switch (i)
        {
        // ========================== system call ==========================
        case SYSCALL:
            the_idt_desc.dpl = 3; // ring 3
            SET_IDT_ENTRY(the_idt_desc, &syscall_entry); 
            idt[i] = the_idt_desc;
            break;
        
        // ========================== interrupts ==========================
        case KBD_INT:
            SET_IDT_ENTRY(the_idt_desc, &keyboard_entry); 
            idt[i] = the_idt_desc;
            break;

        case RTC_INT:
            SET_IDT_ENTRY(the_idt_desc, &rtc_entry);
            idt[i] = the_idt_desc;
            break;

        case PIT_INT:
            SET_IDT_ENTRY(the_idt_desc, &pit_entry);
            idt[i] = the_idt_desc;
            break;

        // ========================== exceptions ==========================
        case ZERO_DIVISION_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &division_error_entry);
            idt[i] = the_idt_desc;
            break;
        
        case SINGLE_STEP_INTERRUPT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &single_step_interrupt_entry);
            idt[i] = the_idt_desc;
            break;

        case NMI_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &NMI_entry);
            idt[i] = the_idt_desc;
            break;

        case BREAKPOINT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &breakpoint_entry);
            idt[i] = the_idt_desc;
            break;

        case OVERFLOW_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &overflow_entry);
            idt[i] = the_idt_desc;
            break;

        case BOUND_RANGE_EXCEED_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &bound_range_exceeded_entry);
            idt[i] = the_idt_desc;
            break;

        case INVALID_OPCODE_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &invalid_opcode_entry);
            idt[i] = the_idt_desc;
            break;

        case COPROCESSOR_NOT_AVAILABLE_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &coprocessor_not_available_entry);
            idt[i] = the_idt_desc;
            break;

        case DOUBLE_FAULT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &double_fault_entry);
            idt[i] = the_idt_desc;
            break;

        case COPROCESSOR_SEGMENT_OVERRUN_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &coprocessor_segment_overrun_entry);
            idt[i] = the_idt_desc;
            break;

        case INVALID_TASK_STATE_SEGMENT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &invalid_task_state_segment_entry);
            idt[i] = the_idt_desc;
            break;

        case SEGMENT_NOT_PRESENT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &segment_not_present_entry);
            idt[i] = the_idt_desc;
            break;

        case STACK_SEGMENT_FAULT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &stack_segment_fault_entry);
            idt[i] = the_idt_desc;
            break;

        case GENERAL_PROTECTION_FAULT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &general_protection_fault_entry);
            idt[i] = the_idt_desc;
            break;

        case PAGE_FAULT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &page_fault_entry);
            idt[i] = the_idt_desc;
            break;

        case RESERVED_EXCP:
            break;

        case x87_FLOATING_POINT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &x87_floating_point_exception_entry);
            idt[i] = the_idt_desc;
            break;
            
        case ALIGNMENT_CHECK_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &alignment_check_entry);
            idt[i] = the_idt_desc;
            break;
        
        case MACHINE_CHECK_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &machine_check_entry);
            idt[i] = the_idt_desc;
            break;

        case SIMD_FLOATING_POINT_EXCP:
            SET_IDT_ENTRY(the_idt_desc, &SIMD_floating_point_exception_entry);
            idt[i] = the_idt_desc;
            break;

        case VIRTUALIZATION_EXCP :
            SET_IDT_ENTRY(the_idt_desc, &virtualization_exception_entry);
            idt[i] = the_idt_desc;
            break;

        case CONTROL_PROTECTION_EXCP :
            SET_IDT_ENTRY(the_idt_desc, &control_protection_exception_entry);
            idt[i] = the_idt_desc;
            break;

        default:
            break;
        }
    }

    lidt(idt_desc_ptr);
}
