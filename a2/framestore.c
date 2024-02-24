#include "page.h"
#include "pcb.h"
#include "shell.h"
#include <stdbool.h>

#define FRAMESTORE_LENGTH 100

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
    if (framestore[i]->available == true)
      return i;
  };

  return -1;
}

/*
 * Print all the pages taken, their pid.
 * For debugging purposes
 */
void print_framestore() {
  int count_empty = 0;
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    if (framestore[i]->pid == -1) {
      count_empty++;
    } else {
      Page *page = framestore[i];
      printf("\npage at index %d: \t\t pid: %d\t\tis_availible: %d\n \t\t ", i,
             page->pid, page->available);
    }
  }
  printf("\n\t%d pages in total, %d pages in use, %d pages free\n\n",
         FRAMESTORE_LENGTH, FRAMESTORE_LENGTH - count_empty, count_empty);
}

/*
 * Load the file into the framestore.
 * In chunks of 3 lines, construct a page containing the 3 lines,
 * and load it to the first free space in the framestore.
 */
int load_file(FILE *sourcefile, char *filename, int pid) {

  // make a copy of the file
  char destinationPath[1024];
  sprintf(destinationPath, "%s/%d", BACKING_STORE_PATH, pid);

  FILE *destFile = fopen(destinationPath, "wb");
  if (destFile == NULL) {
    perror("Error opening destination file");
    fclose(sourcefile);
    exit(EXIT_FAILURE);
  }

  char buffer[4096];
  size_t bytesRead;
  while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourcefile)) > 0) {
    fwrite(buffer, 1, bytesRead, destFile);
  }

  fclose(sourcefile);
  fclose(destFile);

  FILE *fp = fopen(destinationPath, "r");
  int counter = 0;
  char *page_lines[3] = {};

  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    page_lines[counter] = malloc(sizeof(buffer));
    strcpy(page_lines[counter], buffer);
    counter++;

    if (counter < 3)
      continue;

    // at every 3 lines, make a page and load it into the framestore
    int free_space_index = get_free_page_space();
    set_page(framestore[free_space_index], pid, page_lines);

    for (int i = 0; i < 3; i++)
      // reset the 3 item array that stores the lines while the file is being
      // traversed
      for (int i = 0; i < 3; i++) {
        page_lines[i] = NULL;
      }
    counter = 0;
  }

  // if there were some lines loaded, but less than 3
  if (page_lines[0] != NULL) {
    for (int i = 0; i < 3; i++) {
      if (page_lines[i] == NULL)
        page_lines[i] = "none";
    }

    int free_space_index = get_free_page_space();
    set_page(framestore[free_space_index], pid, page_lines);
  }

  fclose(fp);

#ifdef DEBUG
  print_framestore();
#endif

  return 0;
}

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
      table[index] = i;
      index++;
    }
  }

  return table;
}

// Returns line of code from page at page_index line at line_index
char *get_line(int page_index, int line_index) {
  return framestore[page_index]->lines[line_index];
}

// Free all pages with some pid
void free_process_pages(int pid) {

  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    Page *page = framestore[i];
    if (page->pid == pid) {
      free(page);
    }
  }
}

// clear the entire framestore, replace all pages with the default page
void clear_framestore() {
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    init_page(framestore[i]);
  }
}
