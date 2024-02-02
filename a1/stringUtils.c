#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int isVariable(char *str) { return str[0] == '$' ? 1 : 0; }

void slice(const char *str, char *result, size_t start, size_t end) {
  size_t len = end - start;

  strncpy(result, str + start, len);

  result[len] = '\0';
}

char *parseNull(char *str) { return str ? str : ""; }

// Takes in up to 5 inputs to generate a concanated string of all inputs
char *parseSetInput(char *args[], int argsize) {

  char *out_str = malloc(sizeof(char) * 120 * 5);

  strcpy(out_str, args[2]);

  for (int i = 3; i < argsize; i++) {
    strcat(out_str, " ");
    strcat(out_str, parseNull(args[i]));
  }

  return out_str;
}
