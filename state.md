# This is just for reference in the project development

## Exceptions

We got :

PANIC : kernel.c:157: I am trapped!, scause = 00000002, stval = 00000000, sepc = 80200184

When you do :

❯ llvm-addr2line -e kernel.elf 80200184
/Users/tridhatri/Documents/git-repos/Kernel_project/kernel.c:182

This just proves sepc really points to where the exception occcured.

## Memory Allocation

We got this :

Alloc_pages test :paddr0 is 80221000
Alloc_pages test :paddr1 is 80223000

And verification :

❯ llvm-nm kernel.elf | grep __free_ram
80221000 B __free_ram
84221000 B __free_ram_end

So basically, paddr0 matches the start address of __free_ram

Sweet.

# For my undestanding

## High level overview

Time T0: CPU Reset
   └─ No stack

Time T1: boot()
   └─ sp = 0x80020000

Time T2: boot() → kernel_main()
   └─ sp still 0x80020000
   └─ ra points to boot's next instruction (never used, jump not call)

Time T3: kernel_main() → alloc_pages(2)
   └─ sp = 0x80020000
   └─ Stack frame created for alloc_pages

Time T4: alloc_pages() returns paddr0
   └─ sp = 0x80020000
   └─ Back in kernel_main()

Time T5: kernel_main() → alloc_pages(1)
   └─ sp = 0x80020000
   └─ Stack frame created for alloc_pages (reused)

Time T6: alloc_pages() returns paddr1
   └─ sp = 0x80020000
   └─ Back in kernel_main()

Time T7: PANIC() called
   └─ Kernel terminates


### Stack layout in kernel_entry

High Address
┌────────────────────────┐
│ sp+120  │ Original SP  │ ← Saved from sscratch
├────────────────────────┤
│ sp+116  │ s11          │
│ sp+112  │ s10          │
│         │ ...          │
│ sp+68   │ a7           │
│ sp+64   │ a6           │
│         │ ...          │
│ sp+40   │ t6           │
│ sp+36   │ t5           │
│         │ ...          │
│ sp+0    │ ra           │
├────────────────────────┤
Low Address



### Execution order


1. boot() ← First code the CPU executes
   ├─ Initializes stack
   └─ Jumps to kernel_main()
2. kernel_main() ← Main kernel loop
   ├─ Prints hello
   ├─ Sets up exception handler
   └─ Allocates memory
3. kernel_entry() ← Called automatically on exception
   ├─ Saves all registers
   ├─ Calls handle_trap()
   └─ Restores and returns
