# This is just for reference in the project development

## Exceptions

We got : 

PANIC : kernel.c:157: I am trapped!, scause = 00000002, stval = 00000000, sepc = 80200184

When you do : 

❯ llvm-addr2line -e kernel.elf 80200184
/Users/tridhatri/Documents/git-repos/Kernel_project/kernel.c:182

This just proves sepc really points to where the exception occcured.
