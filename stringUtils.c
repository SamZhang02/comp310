#include <string.h>
#include <unistd.h>

// modification of the parseInput function from shell.c
char *split(char ui[]) {
  char tmp[200];
  char *words[100];

  int a = 0;
  int b;
  int w = 0; // wordID
  int errorCode;

  for (a = 0; ui[a] == ' ' && a < 1000; a++)
    ; // skip white spaces
  while (ui[a] != '\n' && ui[a] != '\0' && a < 1000) {
    for (b = 0; ui[a] != ';' && ui[a] != '\0' && ui[a] != '\n' &&
                ui[a] != ' ' && a < 1000;
         a++, b++) {
      tmp[b] = ui[a];
      // extract a word
    }

    tmp[b] = '\0';
    words[w] = strdup(tmp);
    w++;

    if (ui[a] == '\0')
      break;
    a++;
  }

  return *words;
}

int isVariable(char *str) { return str[0] == '$'; }
