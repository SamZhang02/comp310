#include "shell.h"
#include "framestore.h"
#include "interpreter.h"
#include "shellmemory.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

char backing_store_path[1024];

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

int rm_rf(const char *path) {
  DIR *d = opendir(path);
  size_t path_len = strlen(path);
  int r = -1;

  if (d) {
    struct dirent *p;
    r = 0;
    while (!r && (p = readdir(d))) {
      int r2 = -1;
      char *buf;
      size_t len;

      // Ignore the names "." and ".." as we don't want to recurse on them.
      if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
        continue;
      }

      len = path_len + strlen(p->d_name) + 2;
      buf = malloc(len);

      if (buf) {
        struct stat statbuf;

        snprintf(buf, len, "%s/%s", path, p->d_name);
        if (!stat(buf, &statbuf)) {
          if (S_ISDIR(statbuf.st_mode)) {
            r2 = rm_rf(
                buf); // recursively remove all items in the nested diretorye=
          } else {
            r2 = unlink(buf); // remove non-directory file
          }
        }
        free(buf);
      }
      r = r2;
    }
    closedir(d);
  }

  if (!r) {
    r = rmdir(path);
  }

  return r;
}

void create_backing_store() {

  if (access(backing_store_path, F_OK) == 0) {

    if (removeBackingStore() != 0) {
      perror("Error removing directory");
      exit(EXIT_FAILURE);
    }
  }

  if (mkdir(backing_store_path, 0777) != 0) {
    perror("Error creating directory");
    exit(EXIT_FAILURE);
  }
};

int removeBackingStore() { return rm_rf(backing_store_path); }

int main(int argc, char *argv[]) {
  getcwd(backing_store_path, sizeof(backing_store_path));
  strcat(backing_store_path, "/backing_store");

  create_backing_store();

  printf("%s", "Shell v2.0\n");
  printf("Frame Store Size = %d; Variable Store Size = %d\n",
         FRAMESTORE_LENGTH * 3, SHELL_MEM_LENGTH);

  char prompt = '$';              // Shell prompt
  char userInput[MAX_USER_INPUT]; // user's input stored here
  int errorCode = 0;              // zero means no error, default

  // init user input
  for (int i = 0; i < MAX_USER_INPUT; i++)
    userInput[i] = '\0';

  // init shell memory
  mem_init();
  framestore_init();

  while (1) {
    if (isatty(fileno(stdin)))
      printf("%c ", prompt);

    char *str = fgets(userInput, MAX_USER_INPUT - 1, stdin);
    if (feof(stdin)) {
      freopen("/dev/tty", "r", stdin);
    }

    if (strlen(userInput) > 0) {
      errorCode = parseInput(userInput);
      if (errorCode == -1)
        exit(99); // ignore all other errors
      memset(userInput, 0, sizeof(userInput));
    }
  }

  return 0;
}

int parseInput(char *ui) {
  char tmp[200];
  char *words[100];
  memset(words, 0, sizeof(words));

  int a = 0;
  int b;
  int w = 0; // wordID
  int errorCode;
  for (a = 0; ui[a] == ' ' && a < 1000; a++)
    ; // skip white spaces

  while (a < 1000 && a < strlen(ui) && ui[a] != '\n' && ui[a] != '\0') {
    while (ui[a] == ' ') {
      a++;
    }

    if (ui[a] == '\0') {
      break;
    }

    for (b = 0; ui[a] != ';' && ui[a] != '\0' && ui[a] != '\n' &&
                ui[a] != ' ' && a < 1000;

         a++, b++) {
      tmp[b] = ui[a];
    }

    tmp[b] = '\0';

    if (strlen(tmp) == 0)
      continue;

    words[w] = strdup(tmp);

    if (ui[a] == ';') {
      w++;
      errorCode = interpreter(words, w);
      if (errorCode == -1)
        return errorCode;
      a++;
      w = 0;
      for (; ui[a] == ' ' && a < 1000; a++)
        ; // skip white spaces
      continue;
    }

    w++;
    a++;
  }
  errorCode = interpreter(words, w);

  for (int i = 0; i < w; i++) {
    free(words[i]);
  }

  return errorCode;
}
