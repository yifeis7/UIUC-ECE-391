# Bug Log
> Group 10: Martian Pendulum

## Initialize the devices
### Bug 1
#### Description
After implementing the keyboard, we typed in but nothing showed on the screen.
#### Reason
We didn't uncomment "sti()".
#### Solution
Uncomment "sti()".
### Bug 2
#### Description
The screen does not blink after I turn on RTC interrupt.
#### Reason
I only called enable_irq for the slave port.
#### Solution
Add enable_irq for master PIC port 2.

## Initialize paging
### Bug 1
#### Description
The most significant bit of CR0 is set, but paging is still unabled. 
#### Reason
In the loop where we fill in the page directory, we accidentally wrote "page_dir_base[1] = the_page_dir_entry", which means we didn't initialize any of the page directory entry at indices larger than 2, and at the same time the correct value in "page_dir_base[1]" was modified.
#### Solution
Change "page_dir_base[1] = the_page_dir_entry" to "page_dir_base[i] = the_page_dir_entry", so that the loop is correctly filling in each page directory entry.

### Bug 2
#### Description
The present bit for the page table entries are all set to 1.
#### Reason
In the init_user_page_table, we wrongly set all the present bit of page table entries to one. 
#### Solution
Change "the_page_table_entry.present = 1" to "the_page_table_entry.present = (i == VIDEO_START >> 12) ? 1 : 0;". Then the output of "info mem" becomes correct.

## Terminal Driver
### Bug 1
#### Description
Pressing both capslock and shift simultaneously should generate normal key (not capital), but it didn't.
#### Reason
The modify flag is calculated from "shift || capslock" to "shift ^ capslock", so that when one of them are pressed or both or them are pressed the capital version would be displayed.
#### Solution
Change the modify flag from "shift || capslock" to "shift ^ capslock", so that only when one of them are pressed the capital version would be displayed.
### Bug 2
#### Description
Writing to stdin and reading from stdout should fail, but they didn't.
#### Reason
We didn't check for "fd" when writing and reading.
#### Solution
Add sanity check for "fd" before writing and reading.

## Filesystem
### Bug 1
#### Description
Trigger `page fault` in `fs_init()`.
#### Reason
After paging initialization, we cannot reach the module address of filesys_img
#### Solution
We copy the filesys_img into kernel using memcpy, so that we won't reach absent page address.

### Bug 2
#### Description
In `file_read()`, we fail to update the `file_pos` with inputting `file_entry`.
#### Reason
The change of local variable `file_entry.file_pos` in the function cannot be kept after we quit the function.
#### Solution
We input `file_entry *` instead of `file_entry` to `file_read()`.


## System Call
### Bug 1
#### Description
In `execute_SYSCALL`, `Page Fault` occurs when `iret`.
#### Reason
The user/superviser bit of the page assigned for process to be executed is set to 0.
#### Solution
Set the user/superviser bit to enable the page to be accessed by all.

### Bug 2
#### Description
The `ls` command always trigger page fault in the terminal
#### Reason
We forget to store the return value `status` into eax before jump to the execute system call.
#### Solution
We added `movl $status, %eax` before jump to the execute function.

### Bug 3
#### Description
The execution of base shell ends when we `exit` the second shell or `ls` in base shell.
#### Reason
In `execute_SYSCALL`, we store the wrong stack in the pcb of current process (executant).
#### Solution
We store the stack of the executant(caller) of `execute_SYSCALL` and all is good.


### Bug 4
#### Description
If we first "cat verylargetextwithverylongname.txt" then we can not open "verylargetextwithverylongname.tx"
#### Reason
#### Solution

### Bug 5
#### Description
We encounted bootloop when changing the inline assembly of paging.c.
#### Reason
We considered the first line of code as clearing the %eax reg, but it is actually
written for loading the argument "page_directory_base", so we wrongly changed it to xorl %eax %eax. 
#### Solution
We change that line of code to movl %0, %%eax and the bootloop disappeared.

