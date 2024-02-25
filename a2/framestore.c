#include "framestore.h"
#include "lru.h"
#include "page.h"
#include "pcb.h"
#include "ready_queue.h"
#include "shell.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EVICTION_MSG_START "Page fault! Victim page contents:"
#define EVICTION_MSG_END "End of victim page contents."

// singleton page array
Page *framestore[FRAMESTORE_LENGTH];

/*
 * Initialize all pages in the framestore with empty values;
 */
void framestore_init() {
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    Page *page = malloc(sizeof(Page));
    init_page(page);
    framestore[i] = page;
  }
}

/*
 * Get the index of the first available free page space in the framestore,
 * return -1 if no more space left
 */
int get_free_page_space() {

  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    if (framestore[i]->available)
      return i;
  };

  return -1;
}

/*
 * Getting function to get a page from the framestore
 */
Page *get_page_from_framestore(int i) { return framestore[i]; }

/*
 * Print all the pages taken, their pid.
 * For debugging purposes
 */
void print_framestore() {
  int count_empty = 0;
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    if (framestore[i]->available) {
      count_empty++;
    } else {
      Page *page = framestore[i];
      printf("\nindex %d: \t page number: %d \t\t pid: "
             "%d\t\tis_availible: %d\n \t\t last used: %d",
             i, page->page_number, page->pid, page->available, page->last_used);
    }
  }
  printf("\n\t%d pages in total, %d pages in use, %d pages free\n\n",
         FRAMESTORE_LENGTH, FRAMESTORE_LENGTH - count_empty, count_empty);
}

/*
 * Load the file into the framestore.
 * In chunks of 3 lines, construct a page containing the 3 lines,
 * and load it to the first free space in the framestore.
 * as per part 2 of the assignment: load only 3 lines at a time, twice.
 */
int load_file(FILE **fpp, char *filename, int pid) {

  // make a copy of the file
  char destinationPath[1024];
  sprintf(destinationPath, "%s/%d", BACKING_STORE_PATH, pid);

  FILE *destFile = fopen(destinationPath, "wb");
  if (destFile == NULL) {
    perror("Error opening destination file");
    fclose(*fpp);
    exit(EXIT_FAILURE);
  }

  char buffer[4096];
  size_t bytesRead;
  while ((bytesRead = fread(buffer, 1, sizeof(buffer), *fpp)) > 0) {
    fwrite(buffer, 1, bytesRead, destFile);
  }

  fclose(*fpp);
  fclose(destFile);

  // redirect the given file pointer to the backing store and load 2 pages
  *fpp = fopen(destinationPath, "r");
  int counter = 0;
  char *page_lines[3] = {"none", "none", "none"};

  for (int page_num = 0; page_num < 2; page_num++) {
    bool new_lines_were_read = false;

    for (int i = 0; i < 3 && fgets(buffer, sizeof(buffer), *fpp) != NULL; i++) {
      page_lines[i] = malloc(sizeof(buffer));
      strcpy(page_lines[i], buffer);

      new_lines_were_read = true;
    }

    if (!new_lines_were_read)
      continue;

    // load it into the framestore
    int free_space_index = get_free_page_space();
    set_page(framestore[free_space_index], page_num, pid, page_lines,
             increment_timer());

    // clear the page_lines buffer
    for (int i = 0; i < 3; i++) {
      page_lines[i] = "none";
    }
  }

  // don't close fp because we will keep it in the pcb

#ifdef DEBUG
  printf("%s\n", "just loaded a new file");
  print_framestore();
#endif

  return 0;
}

/*
 * Get the number of pages with specific pid;
 * */
int get_num_pages(int pid) {
  int count = 0;

  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    Page *page = framestore[i];
    if (page->pid == pid) {
      count++;
    }
  }

  return count;
}

// Given a program's pid, return the program's pagetable
pagetable get_page_table(int pid) {
  int count = get_num_pages(pid);

  pagetable table = malloc(count * sizeof(int));

  int index = 0;
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    Page *page = framestore[i];
    if (page->pid == pid) {
      table[page->page_number] = i;
      index++;
    }
  }

  return table;
}

// Returns line of code from page at page_index line at line_index
char *get_line(int page_index, int line_index) {
  return framestore[page_index]->lines[line_index];
}

// Set all pages of completed process to available
void free_process_pages(int pid) {

  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    Page *page = framestore[i];
    if (page->pid == pid) {
      page->available = true;
    }
  }
}

// clear the entire framestore, replace all pages with the default page
void clear_framestore() {
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    init_page(framestore[i]);
  }
}

/*
 * Find the victim page according to the page replacement policy.
 * */
int get_victim_page_index() {
  // only LRU for now
  int index;
  int least_recent_timestamp = MAX_INT;

  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    if (framestore[i]->available == false &&
        framestore[i]->last_used < least_recent_timestamp) {
      index = i;
      least_recent_timestamp = framestore[i]->last_used;
    }
  }

  return index;
}

/*
 * Evict page at index i and display the messages accordingly
 * This essentially toggles the "available" bit to true, and that spot can be
 * used for other pages.
 *
 * return the index of the now available page to avoid extra traversal.
 * */

int evict_page(int index) {
  Page *page_to_evict = framestore[index];
  page_to_evict->available = true;

#ifdef DEBUG
  printf("evicting page at index %d\n", index);
#endif
  printf("%s\n", EVICTION_MSG_START);

  for (int i = 0; i < 3; i++) {
    if (strcmp(page_to_evict->lines[i], "none") == 0) {
      break;
    }

    // the stored lines already end with newlines,
    // so we don't need it here
    printf("%s", page_to_evict->lines[i]);
  }

  printf("%s\n", EVICTION_MSG_END);

  return index;
}
