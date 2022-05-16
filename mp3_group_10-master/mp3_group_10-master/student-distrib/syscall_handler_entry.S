/* Entry for system call handlers*/
#define ASM 1
#include "x86_desc.h"

.globl      syscall_entry

.extern syscall_halt
.extern syscall_execute
.extern syscall_read
.extern syscall_write
.extern syscall_open
.extern syscall_close
.extern syscall_getargs
.extern syscall_vidmap
.extern syscall_set_handler
.extern syscall_sigreturn

.data
    MAX_SYSCALL_IDX = 10
.align      4

#
# Interface: Register-based arguments (not C-style)
#    Inputs: eax - the system call number; edx, ecx, ebx - three arguments
#   Outputs: None
# Registers: None

#
# stack frame Borrow from LEC 12, do_IRQ
#               | linkage ret addr |                 
#               |       ebx        |                 
#               |       ecx        |
#               |       edx        |
#               |       esi        |
#               |       edi        |
#               |       ebp        |
#               |        ds        |
#               |        es        |
#               |        fs        |
#               |     return addr  |
#               |  16-bit old CS   |
#               |    EFLAGS        |
#               | old ESP (user)   |
#               |   16-bit old SS  |
#

syscall_entry: 
    # checking the function code
    cmpl $0, %eax
    jle  bad_code
    cmpl $MAX_SYSCALL_IDX, %eax
    jg  bad_code

    # push all regs
    pushl %fs
    pushl %es 
    pushl %ds 

    pushl %ebx
    pushl %ecx
    pushl %edx

    pushl %ebp 
    pushl %edi 
    pushl %esi

    # push esp here so we can restore
    pushl %esp 

    # set CS and DS to kernel segment and data segment
    ljmp $KERNEL_CS, $sys_call_keep_going

sys_call_keep_going:
    movw $KERNEL_DS, %di 
    movw %di, %ds

    # pushing arguments
    pushl %edx
    pushl %ecx
    pushl %ebx

    # system call
    call *syscall_jump_table-4(,%eax,4)

    # popping arguments
    addl $12, %esp
    jmp finish

bad_code:
    movl $-1, %eax
    iret

finish:
    # restore esp
    popl %esp 

    # restoring the registers
    popl %esi 
    popl %edi 

    popl %ebp 
    popl %edx
    popl %ecx
    popl %ebx

    popl %ds
    popl %es
    popl %fs

    iret

syscall_jump_table:
    .long syscall_halt
    .long syscall_execute
    .long syscall_read
    .long syscall_write
    .long syscall_open
    .long syscall_close
    .long syscall_getargs
    .long syscall_vidmap
    .long syscall_sethandler
    .long syscall_sigreturn
.end

