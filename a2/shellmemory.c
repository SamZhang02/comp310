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
struct memory_struct framestore[NUM_PAGES * LINES_PER_PAGE];

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
 *
 * returns: error code, 21: no space left
 */
int load_file(FILE *fp, int *pStart, int *pEnd, char *filename) {
  char *line;
  size_t i;
  int error_code = 0;
  bool hasSpaceLeft = false;
  bool flag = true;
  i = 101;
  size_t candidate;
  while (flag) {
    flag = false;
    for (i; i < SHELL_MEM_LENGTH; i++) {
      if (strcmp(varmemory[i].var, "none") == 0) {
        *pStart = (int)i;
        hasSpaceLeft = true;
        break;
      }
    }
    candidate = i;
    for (i; i < SHELL_MEM_LENGTH; i++) {
      if (strcmp(varmemory[i].var, "none") != 0) {
        flag = true;
        break;
      }
    }
  }
  i = candidate;
  // shell memory is full
  if (hasSpaceLeft == 0) {
    error_code = 21;
    return error_code;
  }

  for (size_t j = i; j < SHELL_MEM_LENGTH; j++) {
    if (feof(fp)) {
      *pEnd = (int)j - 1;
      break;
    } else {
      line = calloc(1, SHELL_MEM_LENGTH);
      if (fgets(line, SHELL_MEM_LENGTH, fp) == NULL) {
        continue;
      }
      varmemory[j].var = strdup(filename);
      varmemory[j].value = strndup(line, strlen(line));
      free(line);
    }
  }

  // no space left to load the entire file into shell memory
  if (!feof(fp)) {
    error_code = 21;
    // clean up the file in memory
    for (int j = 1; i <= SHELL_MEM_LENGTH; i++) {
      varmemory[j].var = "none";
      varmemory[j].value = "none";
    }
    return error_code;
  }
  // printShellMemory();
  return error_code;
}

char *mem_get_value_at_line(int index) {
  if (index < 0 || index > SHELL_MEM_LENGTH)
    return NULL;
  return varmemory[index].value;
}

void mem_free_lines_between(int start, int end) {
  for (int i = start; i <= end && i < SHELL_MEM_LENGTH; i++) {
    if (varmemory[i].var != NULL) {
      free(varmemory[i].var);
    }
    if (varmemory[i].value != NULL) {
      free(varmemory[i].value);
    }
    varmemory[i].var = "none";
    varmemory[i].value = "none";
  }
}
