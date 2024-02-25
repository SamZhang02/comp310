#ifndef PAGE_H
#define PAGE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int pid;
  int page_number;
  char *lines[3];
  bool available;
} Page;

void init_page(Page *self);
void set_page(Page *self, int page_number, int pid, char *lines[3]);

#endif // PAGE_H
