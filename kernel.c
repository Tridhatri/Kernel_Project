#include "kernel.h"
#include "common.h"
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef uint32_t size_t;

extern char __bss[], __bss_end[], __stack_top[];

struct sbiret sbi_call(long arg0, long arg1, long arg2, long arg3, long arg4,
                       long arg5, long fid, long eid) {
    register long a0 __asm__("a0") = arg0;
    register long a1 __asm__("a1") = arg1;
    register long a2 __asm__("a2") = arg2;
    register long a3 __asm__("a3") = arg3;
    register long a4 __asm__("a4") = arg4;
    register long a5 __asm__("a5") = arg5;
    register long a6 __asm__("a6") = fid;
    register long a7 __asm__("a7") = eid;

    __asm__ __volatile__("ecall"
                         : "=r"(a0), "=r"(a1)
                         : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(a6), "r"(a7)
                         : "memory");
    return (struct sbiret){.error = a0, .value = a1};
}


void putchar(char ch) {
    sbi_call(ch, 0, 0, 0, 0, 0, 0, 1 /* Console Putchar */);
}

void puts(const char *s) {
    while (*s) {
        putchar(*s++);
    }
}

// When a exception occurs, there are certain registers which the kernel uses to handle the exception.
// CPU checks the medeleg register to determine which operation mode should handle the exception.
// OpenSBI default configuration is S-Mode handling for both U-mode and S-mode exceptions.
// CPU saves its state (registers) into various CSRs.
// The value of stvec register is set to the PC, jumping to kernel's exception handler.
// The exception handler saves general puprise registers (the program state)
// And handles the exception.
// Once exception handling is done, the exception handler restores the saved execution state and 
// calls the sret instruction to resume execution from the point where the exception occured.

// Certain csr registers that the CPU floods when an exception occurs for the kernel to later handle.
//scause -> Type of exception.
// stval -> Additonal info about exception
// sepc -> Program counter at the point where the exception occured.
// sstatus -> Operation mode (U-mode /S-mode) where the exception has occured.

// Defining Kernel Entry point for when an exception occurs.

__attribute__((naked))
__attribute__((aligned(4)))
void kernel_entry(void) {
    __asm__ __volatile__(
        "csrw sscratch, sp\n"
        "addi sp, sp, -4 * 31\n"
        "sw ra,  4 * 0(sp)\n"
        "sw gp,  4 * 1(sp)\n"
        "sw tp,  4 * 2(sp)\n"
        "sw t0,  4 * 3(sp)\n"
        "sw t1,  4 * 4(sp)\n"
        "sw t2,  4 * 5(sp)\n"
        "sw t3,  4 * 6(sp)\n"
        "sw t4,  4 * 7(sp)\n"
        "sw t5,  4 * 8(sp)\n"
        "sw t6,  4 * 9(sp)\n"
        "sw a0,  4 * 10(sp)\n"
        "sw a1,  4 * 11(sp)\n"
        "sw a2,  4 * 12(sp)\n"
        "sw a3,  4 * 13(sp)\n"
        "sw a4,  4 * 14(sp)\n"
        "sw a5,  4 * 15(sp)\n"
        "sw a6,  4 * 16(sp)\n"
        "sw a7,  4 * 17(sp)\n"
        "sw s0,  4 * 18(sp)\n"
        "sw s1,  4 * 19(sp)\n"
        "sw s2,  4 * 20(sp)\n"
        "sw s3,  4 * 21(sp)\n"
        "sw s4,  4 * 22(sp)\n"
        "sw s5,  4 * 23(sp)\n"
        "sw s6,  4 * 24(sp)\n"
        "sw s7,  4 * 25(sp)\n"
        "sw s8,  4 * 26(sp)\n"
        "sw s9,  4 * 27(sp)\n"
        "sw s10, 4 * 28(sp)\n"
        "sw s11, 4 * 29(sp)\n"

        "csrr a0, sscratch\n"
        "sw a0, 4 * 30(sp)\n"

        "mv a0, sp\n"
        "call handle_trap\n"

        "lw ra,  4 * 0(sp)\n"
        "lw gp,  4 * 1(sp)\n"
        "lw tp,  4 * 2(sp)\n"
        "lw t0,  4 * 3(sp)\n"
        "lw t1,  4 * 4(sp)\n"
        "lw t2,  4 * 5(sp)\n"
        "lw t3,  4 * 6(sp)\n"
        "lw t4,  4 * 7(sp)\n"
        "lw t5,  4 * 8(sp)\n"
        "lw t6,  4 * 9(sp)\n"
        "lw a0,  4 * 10(sp)\n"
        "lw a1,  4 * 11(sp)\n"
        "lw a2,  4 * 12(sp)\n"
        "lw a3,  4 * 13(sp)\n"
        "lw a4,  4 * 14(sp)\n"
        "lw a5,  4 * 15(sp)\n"
        "lw a6,  4 * 16(sp)\n"
        "lw a7,  4 * 17(sp)\n"
        "lw s0,  4 * 18(sp)\n"
        "lw s1,  4 * 19(sp)\n"
        "lw s2,  4 * 20(sp)\n"
        "lw s3,  4 * 21(sp)\n"
        "lw s4,  4 * 22(sp)\n"
        "lw s5,  4 * 23(sp)\n"
        "lw s6,  4 * 24(sp)\n"
        "lw s7,  4 * 25(sp)\n"
        "lw s8,  4 * 26(sp)\n"
        "lw s9,  4 * 27(sp)\n"
        "lw s10, 4 * 28(sp)\n"
        "lw s11, 4 * 29(sp)\n"
        "lw sp,  4 * 30(sp)\n"
        "sret\n"
    );
}


// So, at the time of exception occurance, 
// sscratch register is used as a temporary storagr to save the stack pointer
// at the time of exception occurence.
//1. So, saving the kernel state before exception handling.

// Stack pinter is set in a0 register, 
// and the handle_trap function is called.
//  2. Call the trap handler.
// 3. trap_frame structure is basically now containing all the kernel entry stack registers states.
// Adding __attribute((aligned(4))) aligns the function's starting address to a 4-byte boundary.


// Define the handle_trap func to handle the exception 

void handle_trap(struct trap_frame *f __attribute__((unused))) {
    uint32_t scause = READ_CSR(scause);
    uint32_t stval = READ_CSR(stval);
    uint32_t user_pc = READ_CSR(sepc);
    //This function is basically called from the kernel_entry point
    // This is the actual handler
    // Where you print out the scause, stval, sepc
    PANIC("I am trapped!, scause = %x, stval = %x, sepc = %x\n", scause, stval,user_pc);
}


// Defining a Memory Allocator
extern char __free_ram[], __free_ram_end[];

// Basically a bump allocator/ Linear allocator algorithm.

paddr_t alloc_pages(uint32_t n) {
    static paddr_t next_paddr = (paddr_t) __free_ram;
    // static variable becuase we want to preserve this across calls.
    paddr_t paddr = next_paddr;
    next_paddr += n * PAGE_SIZE;
    // So basically just advancing the next_paddr to the end of the PAGE size * n amount of memory.
    
    if(next_paddr > (paddr_t) __free_ram_end){
        PANIC("Bruh. Out of memory! ");
    }

    memset((void *) paddr, 0, n * PAGE_SIZE);
    // memset enusres that the allocated memory area is always filled with zeroes.
    // This is to ensure to avoid hard-to-debug issues.
    return paddr;
    // Return the tip of the memory until now.
}


// Context switch
__attribute((naked)) void switch_context(uint32_t *prev_sp, uint32_t *next_sp) {
    __asm__ __volatile__ (
        // Save callee-saved registers onto the current procss's stack.
        "addi sp, sp, -13*4\n"
        "sw ra, 0*4(sp)\n"
        "sw s0, 1*4(sp)\n"
        "sw s1, 2*4(sp)\n"
        "sw s2,  3  * 4(sp)\n"
        "sw s3,  4  * 4(sp)\n"
        "sw s4,  5  * 4(sp)\n"
        "sw s5,  6  * 4(sp)\n"
        "sw s6,  7  * 4(sp)\n"
        "sw s7,  8  * 4(sp)\n"
        "sw s8,  9  * 4(sp)\n"
        "sw s9,  10 * 4(sp)\n"
        "sw s10, 11 * 4(sp)\n"
        "sw s11, 12 * 4(sp)\n"

        // Switch the stack pointer
        "sw sp, (a0)\n" //*prev_sp = sp;
        "lw sp, (a1)\n" // Switch the stack pointer sp here.

        // Restore callee-saved registers from the next process's stack.
        "lw ra, 0*4(sp)\n" // Restore callee-saved registers only
        "lw s0,  1  * 4(sp)\n"
        "lw s1,  2  * 4(sp)\n"
        "lw s2,  3  * 4(sp)\n"
        "lw s3,  4  * 4(sp)\n"
        "lw s4,  5  * 4(sp)\n"
        "lw s5,  6  * 4(sp)\n"
        "lw s6,  7  * 4(sp)\n"
        "lw s7,  8  * 4(sp)\n"
        "lw s8,  9  * 4(sp)\n"
        "lw s9,  10 * 4(sp)\n"
        "lw s10, 11 * 4(sp)\n"
        "lw s11, 12 * 4(sp)\n"
        "addi sp, sp, 13 * 4\n"  // We've popped 13 4-byte registers from the stack
        "ret\n"
    );
}
/*
Callee-saved registers are registers that a called function must restore before returning. 
In RISC-V, s0 to s11 are callee-saved registers. 
Other registers like a0 are caller-saved registers, 
and already saved on the stack by the caller.
This is why switch_context handles only part of registers.
*/


// Creating a process
struct process procs[PROCS_MAX]; // All process control structures.

struct process* create_process(uint32_t pc) {
    // Finding an unused process control structure
    // So you are passing the Program context pc.
    // And basically creating the process
    // First, by checking if there are process slots (Bookkeeping)
    // Second, Put

    struct process* proc = NULL;
    int i;

    for(i = 0; i < PROCS_MAX; i++) {
        if(procs[i].state == PROC_UNUSED){
            // Then it is free
            proc = &procs[i];
            // Assign the proc pointer to the struct proc in the array.
            break;
        }
    }
        if(!proc) {
            PANIC("No free process slots!!");
        }

        // Stack the callee-saved registers.
        // These register values will be resotred in the first contxt switch in switch_context.

        uint32_t *sp = (uint32_t*) &proc->stack[sizeof(proc->stack)];
        *--sp = 0; //s11
        *--sp = 0; //s10
        *--sp = 0; //s9
        *--sp = 0; //s8
        *--sp = 0; //s7
        *--sp = 0; //s6
        *--sp = 0; //s5
        *--sp = 0; // s4
        *--sp = 0; // s3
        *--sp = 0; // s2
        *--sp = 0; // s1
        *--sp = 0; // s0
        *--sp = (uint32_t) pc;// ra
        // Initialize fields.
        proc->pid = i + 1;
        proc->state = PROC_RUNNABLE;
        proc->sp = (uint32_t) sp;
        return proc;
}

// Creating Process
void delay(void) {
    for(int i =0; i<30000000;i++)
        __asm__ __volatile("nop"); // Do nothing really.
        // Basically added to tell the compiler to not to optimise and remove the loop.
}

struct process *proc_a;
struct process *proc_b;

void proc_a_entry(void) {
    printf("Starting process A\n");
    while(1) {
        putchar('A');
        switch_context(&proc_a->sp, &proc_b->sp);
        delay();
    }
}

void proc_b_entry(void) {
    printf("Starting process B\n");
    while(1) {
        putchar('B');
        switch_context(&proc_b->sp, &proc_a->sp);
        delay();
    }
}




void kernel_main(void) {
    // Clear BSS section
    memset(__bss, 0, (size_t) __bss_end - (size_t) __bss);

    // Print a message to test output using putchar
    puts("\n\nHello World from RISC-V!\n");
   
    // Hello world using printf 
    printf("\n\nHello %s\n", "World! using Printf!");
    printf("1 + 3 = %d, %x\n", 1 + 3, 0x123abcde);

    // We'll simulate kernel panic
    //PANIC("WE are booted!");
    //printf("Obvously cannot reach here!");

    //WRITE_CSR(stvec, (uint32_t) kernel_entry);
    // Basically saving the kernel entry to save the register in stvec.
    // So that when the exception is handled by the kernel entry, it returns 
    // back to stvec for the CPU to process back again.

    //__asm__ __volatile__("unimp");
    // Unimp instruction basicallt is a pseudo instruction which triggers
    // an illegal instruction exception.


    /* Allocating memory */
    paddr_t paddr0 = alloc_pages(2);
    paddr_t paddr1 = alloc_pages(1);
    printf("Alloc_pages test :paddr0 is %x\n", paddr0);
    printf("Alloc_pages test :paddr1 is %x\n", paddr1);


    WRITE_CSR(stvec, (uint32_t)kernel_entry);
    proc_a = create_process((uint32_t) proc_a_entry);
    proc_b = create_process((uint32_t) proc_b_entry);
    proc_a_entry();
    PANIC("Unreachable here!");

    //PANIC("WE are booted!");

    for (;;);
}

__attribute__((section(".text.boot")))
__attribute__((naked))
void boot(void) {
    __asm__ __volatile__(
        "mv sp, %[stack_top]\n" // Set the stack pointer
        "j kernel_main\n"       // Jump to the kernel main function
        :
        : [stack_top] "r" (__stack_top) // Pass the stack top address
    );
}
