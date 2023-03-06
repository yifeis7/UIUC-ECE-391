#define RTC_INT                             0x28
#define KBD_INT                             0x21
#define PIT_INT                             0x20

#define ZERO_DIVISION_EXCP                  0x00
#define SINGLE_STEP_INTERRUPT_EXCP          0x01
#define NMI_EXCP                            0X02
#define BREAKPOINT_EXCP                     0x03
#define OVERFLOW_EXCP                       0x04
#define BOUND_RANGE_EXCEED_EXCP             0x05
#define INVALID_OPCODE_EXCP                 0x06
#define COPROCESSOR_NOT_AVAILABLE_EXCP      0x07
#define DOUBLE_FAULT_EXCP                   0X08
#define COPROCESSOR_SEGMENT_OVERRUN_EXCP    0x09
#define INVALID_TASK_STATE_SEGMENT_EXCP     0x0A
#define SEGMENT_NOT_PRESENT_EXCP            0x0B
#define STACK_SEGMENT_FAULT_EXCP            0x0C
#define GENERAL_PROTECTION_FAULT_EXCP       0x0D
#define PAGE_FAULT_EXCP                     0x0E
#define RESERVED_EXCP                       0x0F
#define x87_FLOATING_POINT_EXCP             0x10
#define ALIGNMENT_CHECK_EXCP                0X11
#define MACHINE_CHECK_EXCP                  0X12
#define SIMD_FLOATING_POINT_EXCP            0X13
#define VIRTUALIZATION_EXCP                 0X14
#define CONTROL_PROTECTION_EXCP             0X15

#define SYSCALL                             0x80

void idt_init();
