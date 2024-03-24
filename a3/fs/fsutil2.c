#include "fsutil2.h"
#include "../interpreter.h"
#include "bitmap.h"
#include "cache.h"
#include "debug.h"
#include "directory.h"
#include "file.h"
#include "filesys.h"
#include "free-map.h"
#include "fsutil.h"
#include "inode.h"
#include "off_t.h"
#include "partition.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void partial_write_message(int bytes_written, int bufsize) {
  printf("Warning: could only write %d out of %d bytes (reached end of "
         "file)\n",
         bytes_written, bufsize);
}

int copy_in(char *fname) {
  FILE *fp = fopen(fname, "rb");
  int bufsize = 0;
  char *buffer = {};

  if (fp == NULL) {
    return FILE_DOES_NOT_EXIST;
  }

  fseek(fp, 0, SEEK_END);
  bufsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  buffer = malloc(bufsize * sizeof(char) + 1);
  memset(buffer, 0, bufsize);
  buffer[bufsize - 1] = '\0';

  char c;
  for (int buffer_i = 0; !feof(fp); buffer_i += 1) {
    c = getc(fp);
    buffer[buffer_i] = c;
  }

  if (!fsutil_create(fname, bufsize)) {
    return FILE_CREATION_ERROR;
  }

  int bytes_written = fsutil_write(fname, buffer, bufsize);

  free(buffer);

  if (bytes_written == -1) {
    return handle_error(FILE_WRITE_ERROR);
  }

  if (bytes_written != bufsize) {
    partial_write_message(bytes_written, bufsize);
  }

  return 0;
}

int copy_out(char *fname) {
  int bufsize = fsutil_size(fname); // in bytes
  char *buffer = malloc(bufsize);

  // read from the beginning,
  // reset seek upon done reading.
  fsutil_seek(fname, 0);
  int bytes_read = fsutil_read(fname, buffer, bufsize);
  if (bytes_read == -1) {
    free(buffer);
    return FILE_READ_ERROR;
  }
  fsutil_seek(fname, 0);

  FILE *fp = fopen(fname, "w");

  if (fp == NULL) {
    free(buffer);
    return EXTERNAL_FILE_READ_ERROR;
  }

  fprintf(fp, "%s", buffer);
  fclose(fp);
  free(buffer);

  return 0;
}

void find_file(char *pattern) {
  // TODO
  return;
}

void fragmentation_degree() {
  // TODO
}

int defragment() {
  // TODO
  return 0;
}

void recover(int flag) {
  if (flag == 0) { // recover deleted inodes

    // TODO
  } else if (flag == 1) { // recover all non-empty sectors

    // TODO
  } else if (flag == 2) { // data past end of file.

    // TODO
  }
}
