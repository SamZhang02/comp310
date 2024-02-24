#include "page.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHELL_MEM_LENGTH 1000
#define NUM_PAGES 500
#define LINES_PER_PAGE 3

struct memory_struct {
  char *var;
  char *value;
};

struct memory_struct varmemory[SHELL_MEM_LENGTH];

// Helper functions
int match(char *model, char *var) {
  int i, len = strlen(var), matchCount = 0;
  for (i = 0; i < len; i++)
    if (*(model + i) == *(var + i))
      matchCount++;
  if (matchCount == len)
    return 1;
  else
    return 0;
}

char *extract(char *model) {
  char token = '='; // look for this to find value
  char value[1000]; // stores the extract value
  int i, j, len = strlen(model);
  for (i = 0; i < len && *(model + i) != token; i++)
    ; // loop till we get there
  // extract the value
  for (i = i + 1, j = 0; i < len; i++, j++)
    value[j] = *(model + i);
  value[j] = '\0';
  return strdup(value);
}

// Shell memory functions

void mem_init() {
  int i;
  for (i = 0; i < 1000; i++) {
    varmemory[i].var = "none";
    varmemory[i].value = "none";
  }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
  int i;
  for (i = 0; i < 1000; i++) {
    if (strcmp(varmemory[i].var, var_in) == 0) {
      varmemory[i].value = strdup(value_in);
      return;
    }
  }

  // Value does not exist, need to find a free spot.
  for (i = 0; i < 1000; i++) {
    if (strcmp(varmemory[i].var, "none") == 0) {
      varmemory[i].var = strdup(var_in);
      varmemory[i].value = strdup(value_in);
      return;
    }
  }

  return;
}

// get value based on input key
char *mem_get_value(char *var_in) {
  int i;
  for (i = 0; i < 1000; i++) {
    if (strcmp(varmemory[i].var, var_in) == 0) {
      return strdup(varmemory[i].value);
    }
  }
  return NULL;
}

void printShellMemory() {
  int count_empty = 0;
  for (int i = 0; i < SHELL_MEM_LENGTH; i++) {
    if (strcmp(varmemory[i].var, "none") == 0) {
      count_empty++;
    } else {
      printf("\nline %d: key: %s\t\tvalue: %s\n", i, varmemory[i].var,
             varmemory[i].value);
    }
  }
  printf("\n\t%d lines in total, %d lines in use, %d lines free\n\n",
         SHELL_MEM_LENGTH, SHELL_MEM_LENGTH - count_empty, count_empty);
}

char *mem_get_value_at_line(int index) {
  if (index < 0 || index > SHELL_MEM_LENGTH)
    return NULL;
  return varmemory[index].value;
}
