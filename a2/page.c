#include "page.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * takes in a wild page pointer and sets its values to default
 */
void init_page(Page *self) {
  self->pid = -1;
  self->available = true;

  for (int i = 0; i < 3; i++) {
    self->lines[i] = malloc(strlen("none") + 1);

    if (self->lines[i] != NULL) {
      strcpy(self->lines[i], "none");
    } else {
      printf("Failed to allocate memory for lines[%d]\n", i);
      exit(1);
    }
  }
}

/*
 * set the values within the page, set the availability to false
 */
void set_page(Page *self, int pid, char *lines[3]) {
  self->pid = pid;

  for (int i = 0; i < 3; i++) {
    self->lines[i] = malloc(strlen(lines[i]) + 1);

    if (self->lines[i] == NULL) {
      printf("Failed to allocate memory for lines[%d]\n", i);
      exit(1);
    }

    strcpy(self->lines[i], lines[i]);
  }

  self->available = false;
}
