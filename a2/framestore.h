#include "pcb.h"
#include <stdbool.h>
#include <stdio.h>

void framestore_init();
int get_free_page_space();
int load_file(FILE *sourcefile, char *filename, int pid);
pagetable get_page_table(int pid);
char *get_line(int page_index, int line_index);
int get_num_pages(int pid);
void free_process_pages(int pid);
