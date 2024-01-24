#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *parseNull(char *str) { return str ? str : ""; }

// Takes in up to 5 inputs to generate a concanated string of all inputs
char *parseSetInput(char *input1, char *input2, char *input3, char *input4,
                    char *input5) {

  // printf("%s", "parsing null");

  char *arg1 = parseNull(input1);
  char *arg2 = parseNull(input2);
  char *arg3 = parseNull(input3);
  char *arg4 = parseNull(input4);
  char *arg5 = parseNull(input5);

  // printf("%s, %s, %s, %s, %s", arg1, arg2, arg3, arg4, arg5);

  char *out_str = malloc(sizeof(char) * 100);

  strcpy(out_str, arg1);
  strcat(out_str, " ");
  strcat(out_str, arg2);
  strcat(out_str, " ");
  strcat(out_str, arg3);
  strcat(out_str, " ");
  strcat(out_str, arg4);
  strcat(out_str, " ");
  strcat(out_str, arg5);

  return out_str;
}
