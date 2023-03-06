#include "exception_handler.h"
#include "lib.h"
#include "syscall_handler.h"

/* Definitions for the exception handlers */

// 0x00
void EXCP_division_error(void *arg)
{
    printf("division error occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x01
void EXCP_single_step_interrupt(void *arg)
{
    printf("single_step_interrupt occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x02, non-maskable interrupt
void EXCP_NMI(void *arg)
{
    printf("NMI occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x03
void EXCP_breakpoint(void *arg)
{
    printf("breakpoint occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x04
void EXCP_overflow(void *arg)
{
    printf("overflow occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x05
void EXCP_bound_range_exceeded(void *arg)
{
    printf("bound_range_exceeded occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x06
void EXCP_invalid_opcode(void *arg)
{
    printf("invalid_opcode occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x07
void EXCP_coprocessor_not_available(void *arg)
{
    printf("coprocessor_not_available occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x08
void EXCP_double_fault(void *arg)
{
    printf("double_fault occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x09
void EXCP_coprocessor_segment_overrun(void *arg)
{
    printf("coprocessor_segment_overrun occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x0A
void EXCP_invalid_task_state_segment(void *arg)
{
    printf("invalid_task_state_segment occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x0B
void EXCP_segment_not_present(void *arg)
{
    printf("segment_not_present occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x0C
void EXCP_stack_segment_fault(void *arg)
{
    printf("stack_segment_fault occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x0D
void EXCP_general_protection_fault(void *arg)
{
    printf("general_protection_fault occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x0E
void EXCP_page_fault(void *arg)
{
    uint32_t page_fault_addr;
    asm volatile (
        "movl %%cr2, %0     \n\t"
        : "=r"(page_fault_addr)
    );
    printf("page fault address is: %x\n", page_fault_addr);
    printf("page_fault occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0xFF (reserved)

// 0X10
void EXCP_x87_floating_point_exception(void *arg)
{
    printf("x87_floating_point_exception occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x11
void EXCP_alignment_check(void *arg)
{
    printf("alignment_check occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x12
void EXCP_machine_check(void *arg)
{
    printf("machine_check occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x13
void EXCP_SIMD_floating_point_exception(void *arg)
{
    printf("SIMD_floating_point_exception occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x14
void EXCP_virtualization_exception(void *arg)
{
    printf("virtualization_exception occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}

// 0x15
void EXCP_control_protection_exception(void *arg)
{
    printf("control_protection_exception occured!\n");
    // asm volatile("hlt;");
    syscall_halt(0xFF);
}
