#include "pcb.h"
#include <stdbool.h>
#include <stdio.h>

void framestore_init();
int get_free_page_space();
int load_file(FILE *sourcefile, char *filename, int pid);
pagetable get_page_table(int pid);
