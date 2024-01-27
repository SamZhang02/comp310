#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <unistd.h>
// #include <sys/stat.h> // these could be useful?
#include "setUtils.h"
#include "shell.h"
#include "shellmemory.h"
#include "stringUtils.h"

int MAX_ARGS_SIZE = 6;

int badcommand() {
  printf("%s\n", "Unknown Command");
  return 1;
}

// For run command only
int badcommandFileDoesNotExist() {
  printf("%s\n", "Bad command: File not found");
  return 3;
}

int badCommandFrom(char *command) {
  printf("%s%s\n", "Bad command: ", command);
  return 4;
}

int help();
int quit();
int print(char *var);
int set(char *var, char *value);
int echo(char *arg);
int run(char *script);
int mkdir(char *dirname); // TODO
int touch(char *file);    // TODO
int cd(char *dirname);    // TODO
int cat(char *file);      // TODO

int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
  int i;

  // if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
  //   return badcommand();
  // }

  for (i = 0; i < args_size; i++) { // strip spaces new line etc
    command_args[i][strcspn(command_args[i], "\r\n")] = 0;
  }

  if (strcmp(command_args[0], "help") == 0) {
    // help
    if (args_size != 1)
      return badcommand();
    return help();

  } else if (strcmp(command_args[0], "quit") == 0) {
    // quit
    if (args_size != 1)
      return badcommand();
    return quit();

  } else if (strcmp(command_args[0], "set") == 0) {
    // set
    if (args_size < 3 || args_size > 7) {
      return badCommandFrom("set");
    }

    char *concatenated_values = parseSetInput(command_args, args_size);

    int err_code = set(command_args[1], concatenated_values);

    free(concatenated_values);

    return err_code;

  } else if (strcmp(command_args[0], "print") == 0) {
    if (args_size != 2)
      return badcommand();

    return print(command_args[1]);

  } else if (strcmp(command_args[0], "run") == 0) {
    if (args_size != 2)
      return badcommand();

    return run(command_args[1]);

  } else if (strcmp(command_args[0], "echo") == 0) {
    if (args_size != 2)
      return badcommand();

    return echo(command_args[1]);

  } else
    return badcommand();
}

int help() {

  char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n \
echo VAR 	 	echos the value of the variable VAR\n ";
  printf("%s\n", help_string);
  return 0;
}

int quit() {
  printf("%s\n", "Bye!");
  exit(0);
}

int set(char *var, char *value) {
  char *link = "=";
  char buffer[1000];
  strcpy(buffer, var);
  strcat(buffer, link);
  strcat(buffer, value);

  mem_set_value(var, value);

  return 0;
}

int print(char *var) {
  printf("%s\n", mem_get_value(var));
  return 0;
}

int echo(char *arg) {

  char out_str[100];

  if (isVariable(arg)) {
    char variable_name[100];

    slice(arg, variable_name, 1, strlen(arg));

    strcpy(out_str, mem_get_value(variable_name));

    strcmp("Variable does not exist", out_str) == 0 ? strcpy(out_str, "")
                                                    : NULL;
  } else {
    strcpy(out_str, arg);
  }

  printf("%s\n", out_str);

  return 0;
}

int run(char *script) {
  int errCode = 0;
  char line[1000];
  FILE *p = fopen(script, "rt"); // the program is in a file

  if (p == NULL) {
    return badcommandFileDoesNotExist();
  }

  fgets(line, 999, p);
  while (1) {
    errCode = parseInput(line); // which calls interpreter()
    memset(line, 0, sizeof(line));

    if (feof(p)) {
      break;
    }
    fgets(line, 999, p);
  }

  fclose(p);

  return errCode;
}
