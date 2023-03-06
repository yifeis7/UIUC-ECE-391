#ifndef _EXCEPTION_HANDLER_H
#define _EXCEPTION_HANDLER_H

/* Declarations for the exception handlers */

// 0x00
void EXCP_division_error(void *arg);

// 0x01
void EXCP_single_step_interrupt(void *arg);

// 0x02, non-maskable interrupt
void EXCP_NMI(void *arg);

// 0x03
void EXCP_breakpoint(void *arg);

// 0x04
void EXCP_overflow(void *arg);

// 0x05
void EXCP_bound_range_exceeded(void *arg);

// 0x06
void EXCP_invalid_opcode(void *arg);

// 0x07
void EXCP_coprocessor_not_available(void *arg);

// 0x08
void EXCP_double_fault(void *arg);

// 0x09
void EXCP_coprocessor_segment_overrun(void *arg);

// 0x0A
void EXCP_invalid_task_state_segment(void *arg);

// 0x0B
void EXCP_segment_not_present(void *arg);

// 0x0C
void EXCP_stack_segment_fault(void *arg);

// 0x0D
void EXCP_general_protection_fault(void *arg);

// 0x0E
void EXCP_page_fault(void *arg);

// 0x0F (reserved)

// 0X10
void EXCP_x87_floating_point_exception(void *arg);

// 0x11
void EXCP_alignment_check(void *arg);

// 0x12
void EXCP_machine_check(void *arg);

// 0x13
void EXCP_SIMD_floating_point_exception(void *arg);

// 0x14
void EXCP_virtualization_exception(void *arg);

// 0x15
void EXCP_control_protection_exception(void *arg);


#endif  /* exception_handler.h */
