#ifndef FRAMESTORE_H
#define FRAMESTORE_H

#include "page.h"
#include "pcb.h"
#include <stdbool.h>
#include <stdio.h>

void framestore_init();
int get_free_page_space();
int load_file(FILE **fpp, char *filename, int pid);
pagetable get_page_table(int pid);
char *get_line(int page_index, int line_index);
int get_num_pages(int pid);
void free_process_pages(int pid);
Page *get_page_from_framestore(int i);
void print_framestore();

#endif // !FRAMESTORE_H
