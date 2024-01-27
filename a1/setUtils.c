#include <stdlib.h>
#include <string.h>

char *parseNull(char *str) { return str ? str : ""; }

// Takes in up to 5 inputs to generate a concanated string of all inputs
char *parseSetInput(char *args[], int argsize) {

  char *out_str = malloc(sizeof(char) * 120);

  strcpy(out_str, args[2]);

  for (int i = 3; i < argsize; i++) {
    strcat(out_str, " ");
    strcat(out_str, parseNull(args[i]));
  }

  return out_str;
}
