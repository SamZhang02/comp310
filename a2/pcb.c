#include "pcb.h"
#include "framestore.h"
#include "page.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// global pid counter
int pid_counter = 1;

int generatePID() { return pid_counter++; }

PCB *makePCB(int *pagetable, int num_pages, int job_length_score, FILE *fp,
             long file_position) {
  PCB *newPCB = malloc(sizeof(PCB));
  if (newPCB == NULL) {
    return NULL;
  }

  newPCB->pid = generatePID();
  newPCB->job_length_score = job_length_score;
  newPCB->priority = false;

  newPCB->pagetable = pagetable;
  newPCB->num_pages = num_pages;
  newPCB->curr_page = 0;
  newPCB->curr_line = 0;

  newPCB->fp = fp;
  newPCB->file_position = file_position;
  newPCB->file_is_done = false;

  return newPCB;
}

/*
 * Fetch a new page in the file, update the pagetable
 * Return true if a page was fetched, if end of file was reached, return false
 */
bool fetch_a_page(PCB *self) {
  fseek(self->fp, self->file_position, SEEK_SET);

  Page *page;
  init_page(page);

  char buffer[1024];
  char *page_lines[3] = {};

  for (int i = 0; i < 3 && fgets(buffer, sizeof(buffer), self->fp) != NULL;
       i++) {
    page_lines[i] = malloc(sizeof(buffer));
    strcpy(page_lines[i], buffer);
  }

  bool new_lines_were_read = page_lines[0] != NULL;

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
  set_page(get_page_from_framestore(free_space_index), self->pid, page_lines);

  self->file_position = ftell(self->fp);
  self->pagetable = get_page_table(self->pid);

  return true;
}
