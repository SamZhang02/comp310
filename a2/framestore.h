#ifndef FRAMESTORE_H
#define FRAMESTORE_H

// default framesize value so my lsp doesn't give me errors
#ifndef FRAMESIZE
#define FRAMESIZE 0
#endif

#define FRAMESTORE_LENGTH (FRAMESIZE / 3)

#include "page.h"
#include "pcb.h"
#include <stdbool.h>
#include <stdio.h>

void framestore_init();
int get_free_page_space();
pagetable get_page_table(int pid);
char *get_line(int page_index, int line_index);
int get_num_pages(int pid);
void free_process_pages(int pid);
Page *get_page_from_framestore(int i);
void print_framestore();
int evict_page(int index);
int get_victim_page_index();
int load_file(FILE **fpp, int pid);
int load_multiple_files(FILE **fpp1, FILE **fpp2, FILE **fpp3, int pid1,
                        int pid2, int pid3);
#endif // !FRAMESTORE_H
