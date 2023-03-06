# Martian pendulum OS

In the course ECE 391: Computer Systems Engineering, my teammates and I designed and implemented a Linux-like operating system.

## Project structure

```
├── student-distrib #kernel code files like paging and scheduler
│   ├── DEBUG
│   ├── INSTALL
│   ├── Makefile
│   ├── boot.S
│   ├── debug.h
│   ├── debug.sh
│   ├── exception_handler.c
│   ├── exception_handler.h
│   ├── exception_handler_entries.S
│   ├── exception_handler_entries.h
│   ├── filesys_img    #image of file system
│   ├── fs.c    #implementation of im-memory file system
│   ├── fs.h
│   ├── i8259.c    #functions to interact with the 8259 interrupt controller
│   ├── i8259.h
│   ├── idt.c    #interrupt descriptor table
│   ├── idt.h
│   ├── interrupt_handler.c
│   ├── interrupt_handler.h
│   ├── interrupt_handler_entries.S
│   ├── interrupt_handler_entries.h
│   ├── kernel.c
│   ├── keyboard.c    #keyboard support
│   ├── keyboard.h
│   ├── l.sh
│   ├── lib.c
│   ├── lib.h
│   ├── mp3.img
│   ├── multiboot.h
│   ├── paging.c    #paging support
│   ├── paging.h
│   ├── pit.c    #programmable interrupt controller
│   ├── pit.h
│   ├── rtc.c    #real time clock
│   ├── rtc.h
│   ├── syscall_handler.c    #system call support
│   ├── syscall_handler.h
│   ├── syscall_handler_entry.S
│   ├── terminal.c    #implementation of terminal
│   ├── terminal.h
│   ├── types.h
│   ├── x86_desc.S
│   └── x86_desc.h
```

## Features

- Memory paging
- i8259 PIC interrupt handling
- Exception handling
- Support for devices: keyboard, real-time clock, programmable interrupt controller
- In memory read-only filesystem
- Round-robin scheduling based on Programmable Interrupt Timer

## **My contribution:**

- Implemented IDT and GDT
- Implemented paging with 4KB page and 4MB page for both kernel and user space
- Implemented system calls and provided support for running six tasks concurrently from system images in the file system
- Implemented the terminal driver and device drivers to support devices such as keyboard and RTC
- Provided support for basic scheduling for six processes and switching for three terminals
