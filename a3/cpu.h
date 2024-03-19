#ifndef CPU_H
#define CPU_H

#include "pcb.h"

int cpu_run(int *start_pos, int *program_counter, int *size, int *valid_bit,
            char *cwd);
int proc_done(const int *size);

#endif