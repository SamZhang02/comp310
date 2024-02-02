#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "shell.h"
#include "shellmemory.h"
#include "stringUtils.h"

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
  return 1;
}

int help();
int quit();
int print(char *var);
int set(char *var, char *value);
int echo(char *arg);
int my_mkdir(char *dirname);
int my_touch(char *file);    
int my_cd(char *dirname);   
int my_cat(char *file);    
int my_ls();
int run(char *script);

int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
  int i;

  for (i = 0; i < args_size; i++) { // strip spaces new line etc
    command_args[i][strcspn(command_args[i], "\r\n")] = 0;
  }

  if (args_size < 1){
    return badcommand();
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

  } else if (strcmp(command_args[0], "my_mkdir") == 0) {
    if (args_size != 2)
      return badcommand();

    return my_mkdir(command_args[1]);

  } else if (strcmp(command_args[0], "my_touch") == 0) {
    if (args_size != 2)
      return badcommand();

    return my_touch(command_args[1]);

  } else if (strcmp(command_args[0], "my_cd") == 0) {
    if (args_size != 2)
      return badcommand();

    return my_cd(command_args[1]);

  } else if (strcmp(command_args[0], "my_cat") == 0) {
    if (args_size != 2)
      return badcommand();
    
    return my_cat(command_args[1]);
  } else if (strcmp(command_args[0], "my_ls") == 0) {
    if (args_size != 1)
      return badcommand();
    
    return my_ls();
  }
  else
    return badcommand();
}

int help() {

  char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n";

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


int my_mkdir(char *dirname) {
  return mkdir(dirname, 0755);
}

int my_cd(char *dirname) {
  int err_code = chdir(dirname);

  return err_code != 0 ? badCommandFrom("my_cd") : 0;
}

int my_touch (char* dirname) {
  char command[110];

  strcpy(command,"touch ");
  strcat(command, dirname);
  // technically unsafe to do system calls directly, to be tested
   fflush(stdout);
   return system(command);
}

int my_cat (char* filename) {
    FILE *fobj;
    char buffer[9999]; // hope that no line is over 9999 characters :) 

    fobj = fopen(filename, "r");

    if (fobj == NULL) {
      return badCommandFrom("my_cat");
    }

    while (fgets(buffer, sizeof(buffer), fobj) != NULL) {
        printf("%s", buffer);
    }
    
    fclose(fobj);
    return 0;
}

int my_ls(){
  fflush(stdout);
  return system("ls .");
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
