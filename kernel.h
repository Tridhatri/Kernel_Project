#pragma once
#include "common.h"
struct sbiret {
    long error;
    long value;
};

// Kernel Panic == BSOD in windows . Sorta. Kinda.

#define PANIC(fmt, ...)\
do{\
printf("PANIC : %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);\
while(1) {}\
}while(0)

// Description : Prints the file and line where panic occurs. Runs forever to stop all other process.
// Also the reason to define it as a macro, is to display the correct source file and line number where panic occured.
// If you define it in a function, it will show the file name and line number where PANIC is defined ,not where it is called.
// __VA__ARGS__ is a compiler extension for defining macros that accept a  variable number of arguments.
// ## __VA_ARGS__ Removes the preceding , when the variable arguments are empty.

struct trap_frame {
    uint32_t ra;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t sp;
} __attribute__((packed));

// Trap frame struct represents the program state saved in kernel_entry.


// Handy calls for csr.

#define READ_CSR(reg)                                                          \
    ({                                                                         \
        unsigned long __tmp;                                                   \
        __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                  \
        __tmp;                                                                 \
    })

#define WRITE_CSR(reg, value)                                                  \
    do {                                                                       \
        uint32_t __tmp = (value);                                              \
        __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                \
    } while (0)

#define PROCS_MAX 8

#define PROC_UNUSED 0
#define PROC_RUNNABLE 1

struct process {
    int pid; // Process ID
    int state; // Either PROC_UNUSED or PROC_RUNNABLE
    vaddr_t sp; // Stack pointer for this process
    uint8_t stack[8192]; // Kernel stack
};

// so, this basiaclly does having kernel stack for each process/thread
// Instead of a single stack per CPU (processor).

// For Page Table Implementation

#define SATP_SV32 (1u << 31)
#define PAGE_V    (1 << 0)   // "Valid" bit (entry is enabled)
#define PAGE_R    (1 << 1)   // Readable
#define PAGE_W    (1 << 2)   // Writable
#define PAGE_X    (1 << 3)   // Executable
#define PAGE_U    (1 << 4)   // User (accessible in user mode)