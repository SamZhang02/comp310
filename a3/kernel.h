#ifndef KERNEL_H
#define KERNEL_H

void kernel_setup();
int new_proc(const char *script);
int run_proc_FIFO(char *cwd);
int run_proc_RR(char *cwd);

#endif