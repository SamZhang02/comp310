#ifndef INTERPRETER_H
#define INTERPRETER_H

enum Error {
  PLACEHOLDER, // placeholder so an error never has a status code of 0
  FILE_DOES_NOT_EXIST,
  NO_MEM_SPACE,
  READY_QUEUE_FULL,
  SCHEDULING_ERROR,
  TOO_MANY_TOKENS,
  TOO_FEW_TOKENS,
  NON_ALPHANUMERIC_TOKEN,
  BAD_COMMAND,
  ERROR_RM,
  FILE_CREATION_ERROR,
  FILESYSTEM_ERROR,
  FILE_WRITE_ERROR,
  FILE_READ_ERROR,
  EXTERNAL_FILE_READ_ERROR
};

int handle_error(enum Error error_code);
int interpreter(char *command_args[], int args_size, char *cwd);
int help();

#endif
