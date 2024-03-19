
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fs/filesys.h"
#include "fs/ide.h"
#include "interpreter.h"
#include "kernel.h"
#include "shellmemory.h"

#define MAX_USER_INPUT 1000

int parseInput(char ui[], char *cwd);

// Start of everything
int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("%s\n", "Error: myshell must be called with the hard drive as "
                   "argument. E.g., ./myshell myhd.dsk");
    return 1;
  }
  char *hd = argv[1];

  bool format = false;
  if (argc == 3 && strcmp(argv[2], "-f") == 0) {
    format = true;
  }

  char *cwd = malloc(1024 * sizeof(char));
  getcwd(cwd, 1024);

  system("rm -rf backing_store");
  system("mkdir backing_store");

  printf("%s\n", "Shell v2.1");

  char var_size_str1[256] = "; Variable Store Size = ";
  char var_size_str2[20];
  snprintf(var_size_str2, sizeof(var_size_str2), "%d", VAR_STORE_SIZE);
  strncat(var_size_str1, var_size_str2,
          sizeof(var_size_str1) - strlen(var_size_str1) - 1);

  char frame_size_str1[256] = "Frame Store Size = ";
  char frame_size_str2[20];
  snprintf(frame_size_str2, sizeof(frame_size_str2), "%d", FRAME_STORE_SIZE);
  strncat(frame_size_str1, frame_size_str2,
          sizeof(frame_size_str1) - strlen(frame_size_str1) - 1);

  char final_str[512] = "";
  strncat(final_str, frame_size_str1,
          sizeof(final_str) - strlen(final_str) - 1);
  strncat(final_str, var_size_str1, sizeof(final_str) - strlen(final_str) - 1);
  printf("%s\n", final_str);

  char prompt = '$';              // Shell prompt
  char userInput[MAX_USER_INPUT]; // user's input stored here
  int errorCode = 0;              // zero means no error, default

  // init user input
  for (int i = 0; i < MAX_USER_INPUT; i++)
    userInput[i] = '\0';

  // init shell memory
  mem_init();

  // init kernel
  kernel_setup();

  // init FS
  ide_init(hd);
  filesys_init(format);

  while (1) {
    if (isatty(fileno(stdin)))
      printf("%c ", prompt);

    fgets(userInput, MAX_USER_INPUT - 1, stdin);

    if (feof(stdin)) {
      freopen("/dev/tty", "r", stdin);
    }

    errorCode = parseInput(userInput, cwd);
    if (errorCode == -1)
      exit(99); // ignore all other errors
    memset(userInput, 0, sizeof(userInput));
  }

  return 0;
}

// Extract words from the input then call interpreter
//
// supports one-liners separated by ';'
// step 1: use "strtok" to splits up the user input, the delimeter is ';'
// step 2: apply the original starter code to parse each section of the user
// input step 3: once a section is interpreted, keep applying strtok and moves
// to the next section until
//         the cursor reaches the end of the string
int parseInput(char ui[], char *cwd) {
  char *tokens = strtok(ui, ";"); // the user input is now a list of tokens
  int ret =
      0; // the final return value, we update it each time we use "interpreter"

  // iterate through the list of input tokens, the delimeter is ';'
  while (tokens != NULL) {
    char token[MAX_USER_INPUT] = {'\0'};
    strcpy(token, tokens);
    char tmp[MAX_USER_INPUT] = {'\0'};
    char *words[100];
    int a, b;
    int w = 0; // wordID

    for (a = 0; token[a] == ' ' && a < MAX_USER_INPUT; a++)
      ; // skip the leading white spaces

    while (token[a] != '\0' && a < MAX_USER_INPUT) {
      for (b = 0; token[a] != '\0' && token[a] != ' ' && a < MAX_USER_INPUT;
           a++, b++)
        tmp[b] = token[a]; // extract a word

      tmp[b] = '\0';

      words[w++] = strdup(tmp);

      if (token[a++] == '\0') {
        break;
      }
    }
    ret = interpreter(words, w, cwd); // call "interpreter" and update "ret"
    tokens = strtok(NULL, ";");       // move to the next section
    for (int i = 0; i < w; i++) {
      free(words[i]);
    }
    if (ret == -1) {
      break;
    }
  }

  return ret;
}
