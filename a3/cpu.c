#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "shell.h"
#include "shellmemory.h"

/**
 * @brief executes one line of instruction; decreases the process size by 1;
 * increases the program counter by 1
 *
 * @param start_pos
 * @param program_counter
 * @param size
 * @param valid_bit
 * @return int 1: page fault; otherwise consult parseInput function
 */
int cpu_run(int *start_pos, int *program_counter, int *size, int *valid_bit,
            char *cwd) {
  if ((*valid_bit) == 0) {
    return 1; // page fault
  }
  int ret = parseInput(mem_get_value_at(*start_pos), cwd);
  (*program_counter)++, (*size)--;
  return ret;
}

/**
 * @brief check if a process is finished
 *
 * @param size a pointer to the current size of a process
 * @return int 0: not finished; 1: finished
 */
int proc_done(const int *size) { return (*size > 0) ? 0 : 1; }