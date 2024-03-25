#include "fsutil2.h"
#include "../interpreter.h"
#include "../linked_list.h"
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
  FILE *fp = fopen(fname, "r");
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

  // TODO : if it doesnt fit it should create a smaller one instead
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
  struct dir *dir;
  char name[NAME_MAX + 1];

  dir = dir_open_root();

  while (dir_readdir(dir, name)) {
    struct file *f = filesys_open(name);
    int bufsize = file_length(f);
    char *buffer = malloc(bufsize);

    file_seek(f, 0);
    file_read(f, buffer, bufsize);
    file_seek(f, 0);

    bool pattern_is_in_file = strstr(buffer, pattern);

    if (pattern_is_in_file) {
      printf("%s\n", name);
    }

    free(buffer);
    file_close(f);
  }

  dir_close(dir);
}

void fragmentation_degree() {
  int num_fragmentable = 0;
  int num_fragmented = 0;

  struct dir *dir;
  char name[NAME_MAX + 1];
  dir = dir_open_root();

  while (dir_readdir(dir, name)) {
    struct file *f = filesys_open(name);
    offset_t file_length = f->inode->data.length;
    size_t num_sectors = bytes_to_sectors(file_length);

    if (num_sectors > 1) {
      num_fragmentable++;
    }

    block_sector_t *data_sectors = get_inode_data_sectors(f->inode);
    for (int i = 0; i < num_sectors - 1; i++) {
      int block_distance = data_sectors[i + 1] - data_sectors[i];
      if (block_distance > 3) {
        num_fragmented++;
      }
    }
  }

  printf("%f\n", num_fragmentable != 0
                     ? num_fragmented / (double)num_fragmentable
                     : 0.0);

  dir_close(dir);
}

struct file_metadata {
  char *name;
  char *content;
  size_t size;
};

int defragment() {
  LINKED_LIST *existing_files = (LINKED_LIST *)malloc(sizeof(LINKED_LIST));
  list_init(&existing_files, NULL);

  struct dir *dir;
  char name[NAME_MAX + 1];
  dir = dir_open_root();

  while (dir_readdir(dir, name)) {
    // load content into a list (filename, buffer, size)
    struct file_metadata *file_data = malloc(sizeof(struct file_metadata));

    struct file *f = filesys_open(name);
    offset_t f_length = file_length(f);

    char *file_content = malloc(f_length);

    file_seek(f, 0);
    file_read(f, file_content, f_length);
    file_seek(f, 0);

    file_data->name = strdup(name);
    file_data->content = strdup(file_content);
    file_data->size = f_length;

    free(file_content);

    add_tail(file_data, existing_files);

    fsutil_rm(name);
  }

  for (struct NODE *curr = existing_files->dummy_head->next;
       curr != NULL && has_next(&curr); curr = curr->next) {

    struct file_metadata *file_data = curr->data;
    fsutil_write(file_data->name, file_data->content, file_data->size);

    free(file_data->name);
    free(file_data->content);
  }

  list_clear(existing_files);

  return 0;
}

// recover deleted inodes
void recover_0() {}

// recover all non-empty sectors
void recover_1() {}

// data past end of file.
void recover_2() {}

void recover(int flag) {
  if (flag == 0) {
    recover_0();
  } else if (flag == 1) {
    recover_1();
  } else if (flag == 2) {
    recover_2();
  }
}
