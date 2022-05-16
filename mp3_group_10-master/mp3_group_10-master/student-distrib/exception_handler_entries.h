#ifndef _EXCEPTION_HANDLER_ENTRY_H
#define _EXCEPTION_HANDLER_ENTRY_H

#ifndef ASM

// declarations for the exception entries
void division_error_entry();
void single_step_interrupt_entry();
void NMI_entry();
void breakpoint_entry();
void overflow_entry();
void bound_range_exceeded_entry();
void invalid_opcode_entry();
void coprocessor_not_available_entry();
void double_fault_entry();
void coprocessor_segment_overrun_entry();
void invalid_task_state_segment_entry();
void segment_not_present_entry();
void stack_segment_fault_entry();
void general_protection_fault_entry();
void page_fault_entry();
void x87_floating_point_exception_entry();
void alignment_check_entry();
void machine_check_entry();
void SIMD_floating_point_exception_entry();
void virtualization_exception_entry();
void control_protection_exception_entry();

#endif
#endif
