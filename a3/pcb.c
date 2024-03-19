#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcb.h"
#include "shell.h"
#include "shellmemory.h"

int count_lines(const char *script);
int PAGE_table_size(int lines);
PAGE *PAGE_init(int page_pid, int page_table_index);

/**
 * @brief Factory function: produces a pointer to a new PCB
 *
 * @param PID the PID of this PCB
 * @param script the script that contains all the commands
 * @return PCB*  the pointer instance of the new PCB
 */
PCB *PCB_init(int PID, const char *script) {
  char line[1000];
  PCB *new_PCB = (PCB *)malloc(sizeof(PCB));
  new_PCB->pid = PID;
  new_PCB->source_file = fopen(script, "rt"); // the program is in a file
  new_PCB->size =
      count_lines(script); // count the number of lines in the script
  new_PCB->page_table_size = PAGE_table_size(new_PCB->size);
  new_PCB->page_table = (PAGE **)malloc(
      sizeof(PAGE *) *
      new_PCB->page_table_size); // allocate a space for page table
  for (int i = 0; i < new_PCB->page_table_size; i++) {
    new_PCB->page_table[i] = PAGE_init(PID, i);
  }

  if (new_PCB->source_file == NULL) {
    printf("%s: %s\n", "File not found", script);
    exit(99);
  }

  new_PCB->program_counter =
      (int *)calloc(new_PCB->page_table_size, sizeof(int));

  // store the script in the memory
  // only load at most 2 pages at first
  int page_limit = 2 > new_PCB->page_table_size ? new_PCB->page_table_size : 2;
  for (int i = 0; i < page_limit; i++) {
    PAGE *cur_page = new_PCB->page_table[i];
    char *pid_str = (char *)malloc(10);
    sprintf(pid_str, "%d", new_PCB->pid);
    if (frame_alloc(pid_str, cur_page->index, cur_page->valid_bit) == 1) {
      printShellMemory();
      printf("frame_alloc failed\n");
      exit(99);
    }
    for (int j = 0; j < 3; j++) {
      if (fgets(line, 999, new_PCB->source_file) != NULL) {
        mem_set_value_at(cur_page->index[j], pid_str, line,
                         &cur_page->valid_bit[j]);
        memset(line, 0, sizeof(line));
      } else {
        break;
      }
    }
    free(pid_str);
  }

  return new_PCB;
}

/**
 * @brief load a page to the memory when a page fault occurs for a PCB
 *
 * @param PCB
 * @param page_index
 * @return int
 */
int load_PAGE(PCB *PCB, int page_index) {
  char line[1000] = {'\0'};
  PAGE *cur_page = PCB->page_table[page_index];
  char *pid_str = (char *)malloc(10);
  sprintf(pid_str, "%d", PCB->pid);
  if (frame_alloc(pid_str, cur_page->index, cur_page->valid_bit) == 1) {
    return 1;
  }

  for (int j = 0; j < 3; j++) {
    if (fgets(line, 999, PCB->source_file) != NULL) {
      mem_set_value_at(PCB->page_table[page_index]->index[j], pid_str, line,
                       &PCB->page_table[page_index]->valid_bit[j]);
      memset(line, 0, sizeof(line));
    } else {
      break;
    }
  }
  free(pid_str);
  return 0;
}

/**
 * @brief count the number of newlines in a text file
 *
 * @param script file name
 * @return int -1: failed to open the file; n: there are n newlines in the file
 * (0 <= n)
 */
int count_lines(const char *script) {
  FILE *p;
  int count = 0;
  int c;
  int last = '\n';

  // open the file
  p = fopen(script, "r");

  // if fail to open the file, return -1
  if (p == NULL) {
    return -1; // fail to open the file
  }

  // iterate through the file char by char, count the number of '\n'
  while (EOF != (c = fgetc(p))) {
    if (c == '\n' && last != '\n') {
      count++;
    }
    last = c;
  }

  // close the file and return the result
  fclose(p);
  return count + 1; // last line doesn't have a '\n'
}

/**
 * @brief free a PCB pointer
 *
 * @param PCB
 */
void free_PCB(PCB *PCB_data_) {
  fclose(PCB_data_->source_file);
  for (int i = 0; i < PCB_data_->page_table_size; i++) {
    for (int j = 0; j < 3; j++) {
      clear_frame(PCB_data_->page_table[i]->index[j]);
    }
    free(PCB_data_->page_table[i]);
  }
  free(PCB_data_->page_table);
  free(PCB_data_->program_counter);
  free(PCB_data_);
}

/**
 * @brief compute the size of the page table of a PCB
 *
 * @param lines size of the process, measured by the lines of the source file
 * @return int size of the page table to be created
 */
int PAGE_table_size(int lines) {
  return (lines % 3 == 0) ? lines / 3 : lines / 3 + 1;
}

/**
 * @brief initialize a page instance
 *
 * @param page_pid
 * @param page_table_index
 * @return PAGE*
 */
PAGE *PAGE_init(int page_pid, int page_table_index) {
  PAGE *page = (PAGE *)malloc(sizeof(PAGE));
  page->page_index = page_table_index, page->page_pid = page_pid;
  for (int i = 0; i < 3; i++) {
    page->index[i] = 1000, page->valid_bit[i] = 0;
  }
  return page;
}

/**
 * @brief evicts a page from the memory; prints out the content
 *
 * @param PCB
 * @param page_index
 */
void PAGE_evict(PCB *PCB, int page_index) {
  printf("Page fault! Victim page contents:\n");
  for (int i = 0; i < 3; i++) {
    printf("%s", mem_get_value_at(PCB->page_table[page_index]->index[i]));
    mem_free_at(PCB->page_table[page_index]->index[i]);
    PCB->page_table[page_index]->index[i] = 0;
    PCB->page_table[page_index]->valid_bit[i] = 0;
  }
  printf("End of victim page contents.\n");
}

/**
 * @brief decides if two PCB's are equal by comparing their pid's
 *
 * @param p1
 * @param p2
 * @return int
 */
int PCB_equal(const void *p1, const void *p2) {
  PCB *p11 = (PCB *)p1, *p22 = (PCB *)p2;
  return p11->pid == p22->pid ? 1 : 0;
}

/**
 * @brief decides if two pages are equal by comparing their starting position in
 * memory
 *
 * @param p1
 * @param p2
 * @return int
 */
int PAGE_equal(const void *p1, const void *p2) {
  PAGE *p11 = (PAGE *)p1, *p22 = (PAGE *)p2;
  return p11->index[0] == p22->index[0] ? 1 : 0;
}

/**
 * @brief decides if a PCB is done by chcking if all of its program counters are
 * 3
 *
 * @param pcb
 * @return int
 */
int PCB_done(PCB *pcb) {
  for (int i = 0; i < pcb->page_table_size; i++) {
    if (pcb->program_counter[i] != 3) {
      return 0;
    }
  }
  return 1;

}
