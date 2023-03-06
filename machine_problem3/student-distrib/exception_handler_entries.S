/* Common entry for all the exception handlers*/
#define ASM 1
#include "exception_handler_entries.h"

# ---------------------------------------------------------------------------
# 0x00
.extern     EXCP_division_error
.globl      division_error_entry

.align      4
division_error_entry: 
    pushal
    call EXCP_division_error
    popal
    iret

# ---------------------------------------------------------------------------
# 0x01
.extern     EXCP_single_step_interrupt
.globl      single_step_interrupt_entry

.align      4
single_step_interrupt_entry: 
    pushal
    call EXCP_single_step_interrupt
    popal
    iret

# ---------------------------------------------------------------------------
# 0x02
.extern     EXCP_NMI
.globl      NMI_entry

.align      4
NMI_entry: 
    pushal
    call EXCP_NMI
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_breakpoint
.globl      breakpoint_entry

.align      4
breakpoint_entry: 
    pushal
    call EXCP_breakpoint
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_overflow
.globl      overflow_entry

.align      4
overflow_entry: 
    pushal
    call EXCP_overflow
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_bound_range_exceeded
.globl      bound_range_exceeded_entry

.align      4
bound_range_exceeded_entry: 
    pushal
    call EXCP_bound_range_exceeded
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_invalid_opcode
.globl      invalid_opcode_entry

.align      4
invalid_opcode_entry: 
    pushal
    call EXCP_invalid_opcode
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_coprocessor_not_available
.globl      coprocessor_not_available_entry

.align      4
coprocessor_not_available_entry: 
    pushal
    call EXCP_coprocessor_not_available
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_double_fault
.globl      double_fault_entry

.align      4
double_fault_entry: 
    pushal
    call EXCP_double_fault
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_coprocessor_segment_overrun
.globl      coprocessor_segment_overrun_entry

.align      4
coprocessor_segment_overrun_entry: 
    pushal
    call EXCP_coprocessor_segment_overrun
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_invalid_task_state_segment
.globl      invalid_task_state_segment_entry

.align      4
invalid_task_state_segment_entry: 
    pushal
    call EXCP_invalid_task_state_segment
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_segment_not_present
.globl      segment_not_present_entry

.align      4
segment_not_present_entry: 
    pushal
    call EXCP_segment_not_present
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_stack_segment_fault
.globl      stack_segment_fault_entry

.align      4
stack_segment_fault_entry: 
    pushal
    call EXCP_stack_segment_fault
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_general_protection_fault
.globl      general_protection_fault_entry

.align      4
general_protection_fault_entry: 
    pushal
    call EXCP_general_protection_fault
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_page_fault
.globl      page_fault_entry

.align      4
page_fault_entry: 
    pushal
    call EXCP_page_fault
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_x87_floating_point_exception
.globl      x87_floating_point_exception_entry

.align      4
x87_floating_point_exception_entry: 
    pushal
    call EXCP_x87_floating_point_exception
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_alignment_check
.globl      alignment_check_entry

.align      4
alignment_check_entry: 
    pushal
    call EXCP_alignment_check
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_machine_check
.globl      machine_check_entry

.align      4
machine_check_entry: 
    pushal
    call EXCP_machine_check
    popal
    iret

# ---------------------------------------------------------------------------

.extern     EXCP_SIMD_floating_point_exception
.globl      SIMD_floating_point_exception_entry

.align      4
SIMD_floating_point_exception_entry: 
    pushal
    call EXCP_SIMD_floating_point_exception
    popal
    iret

# ---------------------------------------------------------------------------

.extern    EXCP_virtualization_exception
.globl     virtualization_exception_entry

.align      4
virtualization_exception_entry: 
    pushal
    call EXCP_virtualization_exception
    popal
    iret

# ---------------------------------------------------------------------------

.extern    EXCP_control_protection_exception
.globl     control_protection_exception_entry

.align      4
control_protection_exception_entry: 
    pushal
    call EXCP_control_protection_exception
    popal
    iret

.end
