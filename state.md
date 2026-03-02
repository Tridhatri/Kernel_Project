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
