#pragma once

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