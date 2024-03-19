#ifndef FILESYS_DIRECTORY_H
#define FILESYS_DIRECTORY_H

#include "block.h"
#include "off_t.h"
#include <stdbool.h>
#include <stddef.h>

/* Maximum length of a file name component. */
#define NAME_MAX 30

struct inode;

/* A directory. */
struct dir {
  struct inode *inode; /* Backing store. */
  offset_t pos;        /* Current position. */
  int open_cnt;           /* Number of openers. */
};

/* A single directory entry. */
struct dir_entry {
  block_sector_t inode_sector; /* Sector number of header. */
  char name[NAME_MAX + 1];     /* Null terminated file name. */
  bool in_use;                 /* In use or free? */
};

extern struct dir *cwd;

/* Directory and Path manipulation utilities. */
void split_path_filename(const char *path, char *directory, char *filename);

/* Opening and closing directories. */
bool dir_create(block_sector_t sector, size_t entry_cnt);
struct dir *dir_open(struct inode *);
struct dir *dir_open_root(void);
struct dir *dir_open_path(const char *);
struct dir *dir_reopen(struct dir *);
void dir_close(struct dir *);
struct inode *dir_get_inode(struct dir *);

/* Reading and writing. */
bool dir_is_empty(const struct dir *);
bool dir_lookup(const struct dir *, const char *name, struct inode **);
bool dir_add(struct dir *, const char *name, block_sector_t, bool is_dir);
bool dir_remove(struct dir *, const char *name);
bool dir_readdir(struct dir *, char name[NAME_MAX + 1]);
block_sector_t dir_readdir_inode(struct dir *, char name[NAME_MAX + 1]);

#endif /* fs/directory.h */
