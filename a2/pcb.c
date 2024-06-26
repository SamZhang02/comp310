#include "pcb.h"
#include "framestore.h"
#include "lru.h"
#include "page.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * placeholder job length score because it is not releant to this assignment
 * source: https://edstem.org/us/courses/52582/discussion/4371690
 */
#define JOB_LENGTH_SCORE_PLACEHOLDER 0

// global pid counter
int pid_counter = 1;

int generatePID() { return pid_counter++; }

PCB *makePCB(int pid, int *pagetable, int num_pages, FILE *fp) {
  PCB *newPCB = malloc(sizeof(PCB));
  if (newPCB == NULL) {
    return NULL;
  }

  newPCB->pid = pid;
  newPCB->job_length_score = JOB_LENGTH_SCORE_PLACEHOLDER;
  newPCB->priority = false;

  newPCB->pagetable = pagetable;
  newPCB->num_pages = num_pages;
  newPCB->curr_page = 0;
  newPCB->curr_line = 0;

  newPCB->fp = fp;

  return newPCB;
}

/*
 * Method to free the PCB and its components
 */
void free_PCB(PCB *self) {
  free(self->pagetable);
  fclose(self->fp);
  free(self);
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

  // if we didn't read in any new lines, the file is done and nothing was
  // fetched
  if (!new_lines_were_read) {
    return false;
  }

  // otherwise, store it in the framestore and update the pagetable
  for (int i = 0; i < 3; i++) {
    if (page_lines[i] == NULL)
      page_lines[i] = "none";
  }

  int free_space_index = get_free_page_space();

  bool out_of_memory = free_space_index == -1;
  if (out_of_memory) {
    int victim_index = get_victim_page_index();
    free_space_index = evict_page(victim_index);
    self->pagetable[victim_index] = -1;
  }

  // set the respective page in the framestore to its new lines
  set_page(get_page_from_framestore(free_space_index), self->curr_page,
           self->pid, page_lines, increment_timer());

  // update pcb metadata
  pagetable new_table =
      realloc(self->pagetable, (self->num_pages + 1) * sizeof(int));

  new_table[self->num_pages] = free_space_index;
  self->pagetable = new_table;
  self->num_pages++;

#ifdef DEBUG
  printf("%s\n", "just fetched a new page");
  print_framestore();
  printf("%d\n", self->num_pages);
  print_ready_queue();
#endif

  return true;
}
