#ifndef FILESYS_FILE_H
#define FILESYS_FILE_H

#include "off_t.h"
#include <stdbool.h>

struct inode;

/* An open file. */
struct file {
  struct inode *inode; /* File's inode. */
  offset_t pos;        /* Current position. */
  bool deny_write;     /* Has file_deny_write() been called? */
};

void init_file_table();
void add_to_file_table(struct file *, char *);
struct file *get_file_by_fname(char *fname);
bool remove_from_file_table(char *fname);
void free_file_table();

/* Opening and closing files. */
struct file *file_open(struct inode *);
struct file *file_reopen(struct file *);
void file_close(struct file *);
struct inode *file_get_inode(struct file *);

/* Reading and writing. */
offset_t file_read(struct file *, void *, offset_t);
offset_t file_read_at(struct file *, void *, offset_t size, offset_t start);
offset_t file_write(struct file *, const void *, offset_t);
offset_t file_write_at(struct file *, const void *, offset_t size,
                       offset_t start);

/* Preventing writes. */
void file_deny_write(struct file *);
void file_allow_write(struct file *);

/* File position. */
void file_seek(struct file *, offset_t);
offset_t file_tell(struct file *);
offset_t file_length(struct file *);

#endif /* fs/file.h */
