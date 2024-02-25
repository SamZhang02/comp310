#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H

// default varmemsize value so my lsp doesn't give me errors
#ifndef VARMEMSIZE
#define VARMEMSIZE 0
#endif

#define SHELL_MEM_LENGTH VARMEMSIZE

#include <stdio.h>

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
char *mem_get_value_at_line(int index);
void mem_free_lines_between(int start, int end);
void printShellMemory();
void mem_clear();
#endif
