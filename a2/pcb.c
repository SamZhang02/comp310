#include "pcb.h"
#include "framestore.h"
#include "lru.h"
#include "page.h"
#include "ready_queue.h"
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

  bool oom = free_space_index == -1;
  if (oom) {
    free_space_index = evict_page(get_victim_page_index());
  }

  set_page(get_page_from_framestore(free_space_index), self->curr_page,
           self->pid, page_lines, increment_timer());

  // update pcb metadata
  free(self->pagetable);
  self->pagetable = get_page_table(self->pid);
  self->num_pages++;

#ifdef DEBUG
  printf("%s\n", "just fetched a new page");
  print_framestore();
  printf("%d\n", self->num_pages);
  print_ready_queue();
#endif

  return true;
}

// Dynamically allocate or reallocate the pagetable to accommodate a new page
void ensure_page_table_capacity(PCB *pcb, int new_page_number) {
  if (new_page_number >= pcb->num_pages) {
    // Determine the new size needed
    int new_size = new_page_number + 1; // or more, depending on your needs

    // Reallocate the pagetable to the new size
    int *new_table = realloc(pcb->pagetable, new_size * sizeof(int));
    if (!new_table) {
      // Handle allocation failure (e.g., by exiting or by not adding the page)
      fprintf(stderr, "Failed to allocate memory for pagetable\n");
      exit(1);
    }

    // Initialize new entries to -1 or a suitable default value
    for (int i = pcb->num_pages; i < new_size; i++) {
      new_table[i] = -1; // Assuming -1 indicates an unmapped page
    }

    // Update PCB
    pcb->pagetable = new_table;
    pcb->num_pages = new_size;
  }
}

// Example of adding a page to the pagetable
void add_page_to_pagetable(PCB *pcb, int page_number, int frame_number) {
  ensure_page_table_capacity(pcb, page_number);
  pcb->pagetable[page_number] = frame_number;
}
