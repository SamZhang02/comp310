#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"

struct memory_struct {
  char *var;
  char *value;
};

// 0 - FRAME_STORE_SIZE: reserved for frames
// FRAME_STORE_SIZE - FRAME_STORE_SIZE + VAR_STORE_SIZE: reserved for variables

struct memory_struct shellmemory[FRAME_STORE_SIZE + VAR_STORE_SIZE];
int FRAME_VAR_BORDER = FRAME_STORE_SIZE;

// Helper functions
int match(char *model, char *var) {
  int i, len = strlen(var), matchCount = 0;
  for (i = 0; i < len; i++)
    if (*(model + i) == *(var + i))
      matchCount++;
  return matchCount == len ? 1 : 0;
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
  for (int i = 0; i < FRAME_STORE_SIZE + VAR_STORE_SIZE; i++) {
    shellmemory[i].var = "none";
    shellmemory[i].value = "none";
  }
}

/**
 * @brief clears the memory slot at a given position
 *
 * @param pos mempory position
 */
void mem_free_at(int pos) {
  shellmemory[pos].var = strdup("none"),
  shellmemory[pos].value = strdup("none");
}

/**
 * @brief allocate a frame in the physical memory
 *
 * @param pid
 *
 * @return int** NULL: unable to allocate a space; otherwise: successfully
 * allocated a space, the starting position is n(0 <= n <= 899)
 */
int frame_alloc(const char *pid, int *index, int *valid_bit) {
  int i, j, k;
  for (i = 0; i < FRAME_VAR_BORDER; i++) {
    // check if there is a contiguous space of enough size
    for (j = i; j < i + 3 && j < FRAME_VAR_BORDER; j++) {
      // if no, break
      if (strcmp(shellmemory[j].var, "none") != 0) {
        break;
      }
    }
    // if yes, mark this contiguous space as "occupied"
    if (j == i + 3) {
      for (k = i; k < i + 3; k++) {
        shellmemory[k].var = strdup(pid);
        index[k - i] = k, valid_bit[k - i] = 0;
      }
      return 0;
    }
  }
  return 1; // unable to find a contiguous space in the memory
}

/**
 * @brief store a key-value pair at a given position
 *
 * @param pos the position in the memory
 * @param pid the id of the process
 * @param value_in the command
 * @param valid_bit
 */
void mem_set_value_at(int pos, const char *pid, const char *value_in,
                      int *valid_bit) {
  shellmemory[pos].var = strdup(pid);
  shellmemory[pos].value = strdup(value_in);
  (*valid_bit) = 1; // set valid bit to 1
}

/**
 * @brief get the value stored at a given position in the memory
 *
 * @param pos the memory position
 * @return char* the value stored in that position
 */
char *mem_get_value_at(int pos) { return strdup(shellmemory[pos].value); }

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
  int i;

  for (i = FRAME_VAR_BORDER; i < FRAME_STORE_SIZE + VAR_STORE_SIZE; i++) {
    if (strcmp(shellmemory[i].var, var_in) == 0) {
      shellmemory[i].value = strdup(value_in);
      return;
    }
  }

  // Value does not exist, need to find a free spot.
  for (i = FRAME_VAR_BORDER; i < FRAME_STORE_SIZE + VAR_STORE_SIZE; i++) {
    if (strcmp(shellmemory[i].var, "none") == 0) {
      shellmemory[i].var = strdup(var_in);
      shellmemory[i].value = strdup(value_in);
      return;
    }
  }
}

// get value based on input key
char *mem_get_value(char *var_in, char caller) {
  int i;

  for (i = FRAME_VAR_BORDER; i < FRAME_STORE_SIZE + VAR_STORE_SIZE; i++) {
    if (strcmp(shellmemory[i].var, var_in) == 0) {

      return strdup(shellmemory[i].value);
    }
  }
  if (caller == 'p') {
    // 'p' stands for print
    return "Variable does not exist";
  } else {
    return ""; // echo a newline if not found
  }
}

/**
 * @brief reset the memory zone allocated for the variables
 *
 */
void reset_var_zone() {
  for (int i = FRAME_VAR_BORDER; i < FRAME_STORE_SIZE + VAR_STORE_SIZE; i++) {
    mem_free_at(i);
  }
}

/**
 * @brief clear a frame in memory
 *
 * @param start_pos
 */
void clear_frame(int start_pos) {
  for (int i = start_pos; i < start_pos + 3; i++) {
    mem_free_at(i);
  }
}

void printShellMemory() {
  int count_empty = 0;
  for (int i = 0; i < FRAME_STORE_SIZE + VAR_STORE_SIZE; i++) {
    if (strcmp(shellmemory[i].var, "none") == 0) {
      count_empty++;
    } else {
      printf("\nline %d: key: %s\t\tvalue: %s\n", i, shellmemory[i].var,
             shellmemory[i].value);
    }
  }
  printf("\n\t%d lines in total, %d lines in use, %d lines free\n\n",
         FRAME_STORE_SIZE + VAR_STORE_SIZE,
         FRAME_STORE_SIZE + VAR_STORE_SIZE - count_empty, count_empty);
}
