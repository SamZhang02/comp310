#include "page.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_page(Page *self) {
  /*
   * takes in a wild page pointer and sets its values to default
   */

  self->pid = -1;
  self->available = false;

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

void set_page(Page *self, int pid, char *lines[3]) {
  /*
   * set the values within the page, set the availability to false
   */
  self->pid = pid;

  for (int i = 0; i < 3; i++) {
    self->lines[i] = malloc(strlen(lines[i]) + 1);

    if (self->lines[i] == NULL) {
      printf("Failed to allocate memory for lines[%d]\n", i);
      exit(1);

      strcpy(self->lines[i], lines[i]);
    }
  }

  self->available = false;
}