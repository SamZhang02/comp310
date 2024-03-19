#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fs/block.h"
#include "fs/filesys.h"
#include "fs/fsutil.h"
#include "fs/fsutil2.h"
#include "interpreter.h"
#include "kernel.h"
#include "shell.h"
#include "shellmemory.h"

int new_name_count = 1;

int help();
int quit();
int isAlphaNumStr(char *str);
int set(char *var, char *value);
int echo(char *var);
int print(char *var);
int run(const char *script, char *cwd);
int exec(char *scripts[], int size, const char *policy, char *cwd);

char *error_msgs[] = {
    "file does not exist",
    "no space left in shell memory",
    "ready queue is full",
    "scheduling policy error",
    "too many tokens",
    "too few tokens",
    "non-alphanumeric token",
    "unknown name",
    "rm",
    "file could not be created",
    "bad filesystem",
    "file could not be written (maybe no space?)",
    "file could not be read",
};

int handle_error(enum Error error_code) {
  printf("Bad command: %s\n", error_msgs[error_code]);
  return error_code;
}

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size, char *cwd) {
  if (args_size < 1)
    return handle_error(TOO_FEW_TOKENS);

  for (int i = 0; i < args_size; i++) { // strip spaces new line etc
    command_args[i][strcspn(command_args[i], "\r\n")] = 0;
  }

  if (strcmp(command_args[0], "help") == 0) { // help
    if (args_size > 1)
      return handle_error(TOO_MANY_TOKENS);
    return help();
  } else if (strcmp(command_args[0], "quit") == 0 ||
             strcmp(command_args[0], "exit") == 0) { // quit
    if (args_size > 1)
      return handle_error(TOO_MANY_TOKENS);

    char command[1069]; // cwd size + length of the command part
    snprintf(command, sizeof(command), "rm -rf %s/backing_store", cwd);
    system(command);

    filesys_done();

    return quit();
  } else if (strcmp(command_args[0], "set") == 0) { // set
    if (args_size < 3)
      return handle_error(TOO_FEW_TOKENS);

    // check if the var is alphanumeric
    if (!isAlphaNumStr(command_args[1])) {
      return handle_error(NON_ALPHANUMERIC_TOKEN);
    }

    // the following code snippet is the implementation for the enhanced set
    // command step 1: initialize a string buffer to store the VAR's step 2:
    // iterate through the user input, and append each of the VAR's to the end
    // of the buffer initialize the string buffer, the size is 505 since there
    // are at most 5 tokens, and each of them is shorter than 100 characters
    // there are also spaces between the VAR's, so we add 5 extra slots
    char *buffer = (char *)calloc(505, 1);
    // the VAR's begin at index 2
    for (int i = 2; i < args_size; i++) {
      // check if each str is alphanumeric
      if (!isAlphaNumStr(command_args[i])) {
        return handle_error(NON_ALPHANUMERIC_TOKEN);
      }
      // append the current VAR to the end of the buffer
      strcat(buffer, command_args[i]);
      // if the current VAR is the last one, don't append a space after
      // otherwise append the space
      if (!(i == args_size - 1)) {
        strcat(buffer, " ");
      }
    }
    // call the set function
    // set would clear the buffer
    int ret_val = set(command_args[1], buffer);
    free(buffer);
    return ret_val;
  } else if (strcmp(command_args[0], "print") == 0) { // print
    if (args_size < 2)
      return handle_error(TOO_FEW_TOKENS);
    if (args_size > 2)
      return handle_error(TOO_MANY_TOKENS);
    return print(command_args[1]);
  } else if (strcmp(command_args[0], "run") == 0) { // run
    if (args_size < 2)
      return handle_error(TOO_FEW_TOKENS);
    if (args_size > 2)
      return handle_error(TOO_MANY_TOKENS);

    char cmd[100] = "cp ./";
    char new_name[100] = "./backing_store/";
    char old_name[100] = {'\0'};
    strcat(old_name, command_args[1]);
    strcat(cmd, command_args[1]);
    strcat(cmd, " ");
    for (int j = 0; j < new_name_count; j++) {
      strcat(old_name, "a");
    }
    strcat(new_name, old_name);
    strcat(cmd, new_name);
    system(cmd);
    new_name_count++;
    int ret = run(strdup(new_name), cwd);

    char command[1069]; // cwd size + length of the command part
    snprintf(command, sizeof(command), "rm -rf %s/backing_store", cwd);
    system(command);

    char command2[1069]; // cwd size + length of the command part
    snprintf(command2, sizeof(command2), "mkdir %s/backing_store", cwd);
    system(command2);

    return ret;
  } else if (strcmp(command_args[0], "echo") == 0) { // echo
    if (args_size > 2)
      return handle_error(TOO_MANY_TOKENS);
    return echo(command_args[1]);
  } else if (strcmp(command_args[0], "exec") == 0) { // exec
    if (args_size <= 1)
      return handle_error(TOO_FEW_TOKENS);
    if (args_size > 5)
      return handle_error(TOO_MANY_TOKENS);

    char *scripts[args_size];
    for (int i = 1; i < args_size; i++) {
      char cmd[100] = "cp ./";
      char new_name[100] = "./backing_store/";
      char old_name[100] = {'\0'};
      strcat(old_name, command_args[i]);
      strcat(cmd, command_args[i]);
      strcat(cmd, " ");
      for (int j = 0; j < new_name_count; j++) {
        strcat(old_name, "a");
      }
      strcat(new_name, old_name);
      strcat(cmd, new_name);
      system(cmd);
      new_name_count++;
      scripts[i - 1] = strdup(new_name);
    }
    int ret = exec(scripts, args_size - 1, "RR", cwd);

    char command[1069]; // cwd size + length of the command part
    snprintf(command, sizeof(command), "rm -rf %s/backing_store", cwd);
    system(command);

    char command2[1069]; // cwd size + length of the command part
    snprintf(command2, sizeof(command2), "mkdir %s/backing_store", cwd);
    system(command2);

    return ret;
  } else if (strcmp(command_args[0], "resetmem") == 0) { // resetmem
    reset_var_zone();
    return 0;
  }

  // FS
  else if (strcmp(command_args[0], "ls") == 0) { // ls
    if (args_size > 1)
      return handle_error(TOO_MANY_TOKENS);
    int status = fsutil_ls(NULL);
    if (status == 1)
      return handle_error(FILESYSTEM_ERROR);
    return 0;
  } else if (strcmp(command_args[0], "cat") == 0) { // cat
    if (args_size != 2)
      return handle_error(TOO_MANY_TOKENS);
    int status = fsutil_cat(command_args[1]);
    if (status == 1)
      return handle_error(FILE_DOES_NOT_EXIST);
    return 0;
  } else if (strcmp(command_args[0], "rm") == 0) { // rm
    if (args_size != 2)
      return handle_error(TOO_MANY_TOKENS);
    int status = fsutil_rm(command_args[1]);
    if (status == 0)
      return handle_error(ERROR_RM);
    return status;
  } else if (strcmp(command_args[0], "create") == 0) { // rm
    if (args_size != 3)
      return handle_error(TOO_MANY_TOKENS);
    int size = atoi(command_args[2]);
    int status = fsutil_create(command_args[1], size);
    if (status == 0)
      return handle_error(FILE_CREATION_ERROR);
    return 0;
  } else if (strcmp(command_args[0], "write") == 0) { // rm
    if (args_size < 3)
      return handle_error(TOO_FEW_TOKENS);
    int size = 0;
    for (int i = 2; i < args_size; i++) {
      size += strlen(command_args[i]);
    }
    size += (args_size - 1);
    char *buf = malloc(size * sizeof(char));
    memset(buf, 0, size);
    int current_ind = 0;
    for (int i = 2; i < args_size; i++) {
      strcpy(buf + current_ind, command_args[i]);
      current_ind += strlen(command_args[i]);
      strcpy(buf + current_ind, " ");
      current_ind += 1;
    }
    buf[current_ind - 1] = '\0';
    int bytes_written = fsutil_write(command_args[1], buf, size);
    free(buf);
    if (bytes_written == -1) {
      return handle_error(FILE_WRITE_ERROR);
    } else if (bytes_written != size) {
      printf("Warning: could only write %d out of %d bytes (reached end of "
             "file)\n",
             bytes_written, size);
    }
    return 0;
  } else if (strcmp(command_args[0], "find_file") == 0) { // rm
    if (args_size < 2)
      return handle_error(TOO_FEW_TOKENS);
    int size = 0;
    for (int i = 1; i < args_size; i++) {
      size += strlen(command_args[i]);
    }
    size += args_size;
    char *buf = malloc(size * sizeof(char));
    memset(buf, 0, size);
    int current_ind = 0;
    for (int i = 1; i < args_size; i++) {
      strcpy(buf + current_ind, command_args[i]);
      current_ind += strlen(command_args[i]);
      strcpy(buf + current_ind, " ");
      current_ind += 1;
    }
    buf[current_ind - 1] = '\0';

    find_file(buf);
    free(buf);
    return 0;
  } else if (strcmp(command_args[0], "read") == 0) { // rm
    if (args_size != 3)
      return handle_error(TOO_MANY_TOKENS);
    int size = atoi(command_args[2]);
    char *buffer = malloc((size + 1) * sizeof(char));
    memset(buffer, 0, (size + 1));
    int bytes_read = fsutil_read(command_args[1], buffer, size);
    if (bytes_read == -1) {
      free(buffer);
      return handle_error(FILE_READ_ERROR);
    }
    printf("%s\n", buffer);
    free(buffer);
    return bytes_read == size;
  } else if (strcmp(command_args[0], "copy_in") == 0) {
    if (args_size != 2)
      return handle_error(TOO_MANY_TOKENS);

    int status = copy_in(command_args[1]);
    if (status != 0)
      return handle_error(status);
    return 0;
  } else if (strcmp(command_args[0], "copy_out") == 0) {
    if (args_size != 2)
      return handle_error(TOO_MANY_TOKENS);

    int status = copy_out(command_args[1]);
    if (status != 0)
      return handle_error(status);
    return 0;
  } else if (strcmp(command_args[0], "size") == 0) { // rm
    if (args_size != 2)
      return handle_error(TOO_MANY_TOKENS);
    int length = fsutil_size(command_args[1]);
    if (length == -1) {
      return handle_error(FILE_DOES_NOT_EXIST);
    }
    printf("File length: %d\n", length);
    return 0;
  } else if (strcmp(command_args[0], "seek") == 0) { // rm
    if (args_size != 3)
      return handle_error(TOO_MANY_TOKENS);
    int offset = atoi(command_args[2]);
    int status = fsutil_seek(command_args[1], offset);
    if (status == -1) {
      return handle_error(FILE_DOES_NOT_EXIST);
    }
    return 0;
  } else if (strcmp(command_args[0], "freespace") == 0) { // rm
    if (args_size != 1)
      return handle_error(TOO_MANY_TOKENS);
    int free_space = fsutil_freespace();
    printf("Num free sectors: %d (%d total bytes)\n", free_space,
           free_space * BLOCK_SECTOR_SIZE);
    return 0;
  } else if (strcmp(command_args[0], "fragmentation_degree") == 0) { // rm
    if (args_size != 1)
      return handle_error(TOO_MANY_TOKENS);
    fragmentation_degree();
    return 0;
  } else if (strcmp(command_args[0], "defragment") == 0) { // rm
    if (args_size != 1)
      return handle_error(TOO_MANY_TOKENS);
    defragment();
    return 0;
  } else if (strcmp(command_args[0], "recover") == 0) { // rm
    if (args_size != 2)
      return handle_error(TOO_MANY_TOKENS);
    int flag = atoi(command_args[1]);
    recover(flag);
    return 0;
  } else {
    return handle_error(BAD_COMMAND);
  }
}

int help() {

  char help_string[] = "COMMAND			DESCRIPTION\n \
help			      Displays all the commands\n \
quit			      Exits / terminates the shell with “Bye!”\n \
set VAR STRING		      Assigns a value to shell memory\n \
print VAR		      Displays the STRING assigned to VAR\n \
echo VAR                     Displays VAR if VAR doesn't begin with '$', otherwise displays the value assigned to VAR\n \
ls                        Displays the folders and files in the current directory in ascending order according to ASCII values \n \
run SCRIPT.TXT		      Executes the file SCRIPT.TXT\n \
exec [SCRIPT.TXT]     Executes up to 3 files using the round-robin scheduling policy\n \
resetmem                     Deletes the content of the variable store\n";
  printf("%s\n", help_string);
  return 0;
}

int quit() {
  printf("%s\n", "Bye!");
  return -1;
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
  printf("%s\n", mem_get_value(var, 'p'));
  return 0;
}

// implementation for the echo command
// two usages:
// 1. when the second token doesn't start begin with '$', display it directly
// 2. if the second token begins with '$', display its corresponding value in
// the memory or
//    an empty line if it's not found in the memory
//
// step 1: check the first character of the second token
// step 2: calls "printf" if it's not '$', otherwise calls "mem_get_value" to
// check if it's stored in the memory, display it if yes, otherwise display an
// empty line
int echo(char *var) {
  // fetch the first character
  char fst_letter = var[0];
  if (fst_letter != '$') {
    // check if the string is alphanumeric
    if (!isAlphaNumStr(var)) {
      return handle_error(NON_ALPHANUMERIC_TOKEN);
    }
    printf("%s\n", var);
  } else {
    // check if it's an alphanumeric string
    for (char *p = var + 1; *p != '\0'; p++) {
      if (!isAlphaNumStr(p)) {
        return handle_error(NON_ALPHANUMERIC_TOKEN);
      }
    }

    // the parameter 'e' will make "mem_get_value" return an empty
    // string if the value is not found in the memory
    printf("%s\n", mem_get_value(var + 1, 'e'));
  }
  return 0;
}

/**
 * @brief runs a single script
 *
 * @param script the script to be executed
 * @return int 1: failure; 0: succeed
 */
int run(const char *script, char *cwd) {
  // add the PCB to the ready queue
  if (new_proc(script) != 0) {
    return 1;
  }

  return run_proc_RR(cwd);
}

int isAlphaNumStr(char *str) {
  if (*str == '\0') {
    return 0; // empty string, false
  }
  char *tmp_var = str;
  for (; *tmp_var != '\0'; tmp_var++) {
    if (!isalnum(*tmp_var)) {
      return 0; // false
    }
  }
  return 1; // true
}

/**
 * @brief execute up to 3 scripts using the given schedulig policy
 *
 * @param scripts a list of scripts to be executed
 * @param size number of scripts
 * @param policy name of the scheduling policy
 * @return int 1: invalid number of scripts
 * refer to different "run_proc" functions in kernel.c
 */
int exec(char *scripts[], int size, const char *policy, char *cwd) {
  // check number of scripts
  if (size < 0 || size > 3) {
    printf("%s", "exec can only take between 1 to 3 scripts\n");
    return 1;
  }

  int i;
  int ret = 0;

  // add all scripts to the ready queue
  for (i = 0; i < size; i++) {
    if (new_proc(scripts[i]) != 0) {
      return 1;
    }
  }

  // execute according to the policy
  if (strcmp(policy, "FIFO") == 0) {
    ret = run_proc_FIFO(cwd);
  } else if (strcmp(policy, "RR") == 0) {
    ret = run_proc_RR(cwd);
  }

  return ret;
}