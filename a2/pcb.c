#include "pcb.h"
#include "framestore.h"
#include "page.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// global pid counter
int pid_counter = 1;

int generatePID() { return pid_counter++; }

PCB *makePCB(int pid, int *pagetable, int num_pages, int job_length_score,
             FILE *fp) {
  PCB *newPCB = malloc(sizeof(PCB));
  if (newPCB == NULL) {
    return NULL;
  }

  newPCB->pid = pid;
  newPCB->job_length_score = job_length_score;
  newPCB->priority = false;

  newPCB->pagetable = pagetable;
  newPCB->num_pages = num_pages;
  newPCB->curr_page = 0;
  newPCB->curr_line = 0;

  newPCB->fp = fp;

  return newPCB;
}

/*
 * Fetch a new page in the file, update the pagetable
 * Return true if a page was fetched, if end of file was reached, return false
 */
bool fetch_a_page(PCB *self) {
  char buffer[1024];
  char *page_lines[3] = {};

  bool new_lines_were_read = false;
  for (int i = 0; i < 3 && fgets(buffer, sizeof(buffer), self->fp) != NULL;
       i++) {
    page_lines[i] = malloc(sizeof(buffer));
    strcpy(page_lines[i], buffer);

    new_lines_were_read = true;
  }

  // if we are out of lines, return false
  if (!new_lines_were_read) {
    return false;
  }

  // otherwise, store it in the framestore and update the pagetable
  for (int i = 0; i < 3; i++) {
    if (page_lines[i] == NULL)
      page_lines[i] = "none";
  }

  int free_space_index = get_free_page_space();
  set_page(get_page_from_framestore(free_space_index), self->curr_page,
           self->pid, page_lines);

  // update pcb metadata
  self->pagetable = get_page_table(self->pid);
  self->num_pages++;

  print_framestore();

  return true;
}
