#include "file.h"
#include "debug.h"
#include "inode.h"
#include "list.h"
#include "string.h"
#include <stdbool.h>
#include <stdlib.h>

struct list file_table;
struct file_table_entry {
  char *fname;           /* file descriptor. */
  struct file *f;        /* pointer to open file. */
  struct list_elem elem; /* used by the process to store its open files. */
};

void init_file_table() { llist_init(&file_table); }

void free_file_table() {
  while (!list_empty(&file_table)) {
    struct list_elem *e = list_pop_front(&file_table);
    struct file_table_entry *entry =
        list_entry(e, struct file_table_entry, elem);
    free(entry->fname);
    file_close(entry->f);
    list_remove(&entry->elem);
    free(entry);
  }
}

void add_to_file_table(struct file *file, char *fname) {
  if (get_file_by_fname(fname) != NULL)
    return;

  struct file_table_entry *entry = malloc(sizeof(struct file_table_entry));
  entry->fname = strdup(fname);
  entry->f = file;
  list_push_back(&file_table, &entry->elem);
}

bool remove_from_file_table(char *fname) {
  struct list_elem *e = list_begin(&file_table);
  struct file_table_entry *entry;
  for (; e != list_end(&file_table); e = list_next(e)) {
    entry = list_entry(e, struct file_table_entry, elem);
    if (strcmp(entry->fname, fname) == 0) {
      free(entry->fname);
      file_close(entry->f);
      list_remove(&entry->elem);
      free(entry);
      return true;
    }
  }
  return false;
}

/* return file object crossponding to given filename */
struct file *get_file_by_fname(char *fname) {
  struct list_elem *e = list_begin(&file_table);
  struct file_table_entry *entry;
  for (; e != list_end(&file_table); e = list_next(e)) {
    entry = list_entry(e, struct file_table_entry, elem);
    if (strcmp(entry->fname, fname) == 0)
      return entry->f;
  }

  return NULL;
}

/* Opens a file for the given INODE, of which it takes ownership,
   and returns the new file.  Returns a null pointer if an
   allocation fails or if INODE is null. */
struct file *file_open(struct inode *inode) {
  struct file *file = calloc(1, sizeof *file);
  if (inode != NULL && file != NULL) {
    file->inode = inode;
    file->pos = 0;
    file->deny_write = false;
    return file;
  } else {
    inode_close(inode);
    free(file);
    return NULL;
  }
}

/* Opens and returns a new file for the same inode as FILE.
   Returns a null pointer if unsuccessful. */
struct file *file_reopen(struct file *file) {
  return file_open(inode_reopen(file->inode));
}

/* Closes FILE. */
void file_close(struct file *file) {
  if (file != NULL) {
    file_allow_write(file);
    inode_close(file->inode);
    free(file);
  }
}

/* Returns the inode encapsulated by FILE. */
struct inode *file_get_inode(struct file *file) { return file->inode; }

/* Reads SIZE bytes from FILE into BUFFER,
   starting at the file's current position.
   Returns the number of bytes actually read,
   which may be less than SIZE if end of file is reached.
   Advances FILE's position by the number of bytes read. */
offset_t file_read(struct file *file, void *buffer, offset_t size) {
  offset_t bytes_read = inode_read_at(file->inode, buffer, size, file->pos);
  file->pos += bytes_read;
  return bytes_read;
}

/* Reads SIZE bytes from FILE into BUFFER,
   starting at offset FILE_OFS in the file.
   Returns the number of bytes actually read,
   which may be less than SIZE if end of file is reached.
   The file's current position is unaffected. */
offset_t file_read_at(struct file *file, void *buffer, offset_t size,
                      offset_t file_ofs) {
  return inode_read_at(file->inode, buffer, size, file_ofs);
}

/* Writes SIZE bytes from BUFFER into FILE,
   starting at the file's current position.
   Returns the number of bytes actually written,
   which may be less than SIZE if end of file is reached.
   (Normally we'd grow the file in that case, but file growth is
   not yet implemented.)
   Advances FILE's position by the number of bytes read.
   Sets the final character of the buffer to the null terminator. */
offset_t file_write(struct file *file, const void *buffer, offset_t size) {
  ((char *)buffer)[size - 1] = '\0';
  offset_t bytes_written = inode_write_at(file->inode, buffer, size, file->pos);
  file->pos += bytes_written - 2;
  return bytes_written;
}

/* Writes SIZE bytes from BUFFER into FILE,
   starting at offset FILE_OFS in the file.
   Returns the number of bytes actually written,
   which may be less than SIZE if end of file is reached.
   (Normally we'd grow the file in that case, but file growth is
   not yet implemented.)
   The file's current position is unaffected. */
offset_t file_write_at(struct file *file, const void *buffer, offset_t size,
                       offset_t file_ofs) {
  return inode_write_at(file->inode, buffer, size, file_ofs);
}

/* Prevents write operations on FILE's underlying inode
   until file_allow_write() is called or FILE is closed. */
void file_deny_write(struct file *file) {
  ASSERT(file != NULL);
  if (!file->deny_write) {
    file->deny_write = true;
    inode_deny_write(file->inode);
  }
}

/* Re-enables write operations on FILE's underlying inode.
   (Writes might still be denied by some other file that has the
   same inode open.) */
void file_allow_write(struct file *file) {
  ASSERT(file != NULL);
  if (file->deny_write) {
    file->deny_write = false;
    inode_allow_write(file->inode);
  }
}

/* Returns the size of FILE in bytes. */
offset_t file_length(struct file *file) {
  ASSERT(file != NULL);
  return inode_length(file->inode);
}

/* Sets the current position in FILE to NEW_POS bytes from the
   start of the file. */
void file_seek(struct file *file, offset_t new_pos) {
  ASSERT(file != NULL);
  ASSERT(new_pos >= 0);
  file->pos = new_pos;
}

/* Returns the current position in FILE as a byte offset from the
   start of the file. */
offset_t file_tell(struct file *file) {
  ASSERT(file != NULL);
  return file->pos;
}
