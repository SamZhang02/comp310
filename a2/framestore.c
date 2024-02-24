#include "page.h"
#include "pcb.h"
#include "shell.h"
#include <stdbool.h>

#define FRAMESTORE_LENGTH 100

// singleton page array
Page *framestore[FRAMESTORE_LENGTH];

/*
 * Get the index of the first available free page space in the framestore,
 * return -1 if no more space left
 */

void framestore_init() {
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    Page *page = malloc(sizeof(Page));
    init_page(page);
    framestore[i] = page;
  }
}
int get_free_page_space() {

  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    if (framestore[i]->available == true)
      return i;
  };

  return -1;
}

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
 * Function:  addFileToMem
 * 	Added in A2
 * --------------------
 * Load the source code of the file fp into the shell memory:
 * 		Loading format - var stores fileID, value stores a line
 *		Note that the first 100 lines are for set command, the rests are
 for run and exec command
 *
 *  pStart: This function will store the first line of the loaded file
 * 			in shell memory in here
 *	pEnd: This function will store the last line of the loaded file
                        in shell memory in here
 *  fileID: Input that need to provide when calling the function,
                        stores the ID of the file
 * filename: the name that the file will be saved as
 * returns: error code, 21: no space left
 */
int load_file(FILE *sourcefile, char *filename) {
  int pid = generatePID();
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
    Page *page = malloc(sizeof(Page));
    set_page(page, pid, page_lines);
    framestore[free_space_index] = page;

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
  print_framestore();
  return 0;
}

// given a program's pid, return the program's pagetable
pagetable get_page_table(int pid) {
  int count = 0;

  // First pass: Count the number of pages for the given pid for malloc
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    Page *page = framestore[i];
    if (page->pid == pid) {
      count++;
    }
  }

  pagetable table = malloc(count * sizeof(int));

  // Second pass: Fill the pagetable
  int index = 0;
  for (int i = 0; i < FRAMESTORE_LENGTH; i++) {
    Page *page = framestore[i];
    if (page->pid == pid) {
      table[index++] = i;
      index++;
    }
  }

  return table;
}
