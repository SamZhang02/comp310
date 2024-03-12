#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include "block.h"
#include "list.h"
#include "off_t.h"
#include <stdbool.h>

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

#define DIRECT_BLOCKS_COUNT 123
#define INDIRECT_BLOCKS_PER_SECTOR 128

struct bitmap;

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk {
  /** Data sectors */
  block_sector_t direct_blocks[DIRECT_BLOCKS_COUNT];
  block_sector_t indirect_block;
  block_sector_t doubly_indirect_block;

  bool is_dir;
  offset_t length; /* File size in bytes. */
  unsigned magic;  /* Magic number. */
};

/* In-memory inode. */
struct inode {
  struct list_elem elem;  /* Element in inode list. */
  block_sector_t sector;  /* Sector number of disk location. */
  int open_cnt;           /* Number of openers. */
  bool removed;           /* True if deleted, false otherwise. */
  int deny_write_cnt;     /* 0: writes ok, >0: deny writes. */
  struct inode_disk data; /* Inode content. */
};

void inode_init(void);
bool inode_create(block_sector_t, offset_t, bool);
struct inode *inode_open(block_sector_t);
struct inode *inode_reopen(struct inode *);
block_sector_t inode_get_inumber(const struct inode *);
void inode_close(struct inode *);
void inode_remove(struct inode *);
offset_t inode_read_at(struct inode *, void *, offset_t size, offset_t offset);
offset_t inode_write_at(struct inode *, const void *, offset_t size,
                        offset_t offset);
void inode_deny_write(struct inode *);
void inode_allow_write(struct inode *);
offset_t inode_length(const struct inode *);
bool inode_is_directory(const struct inode *);
bool inode_is_removed(const struct inode *);
size_t bytes_to_sectors(offset_t size);

block_sector_t *get_inode_data_sectors(struct inode *);

#endif /* fs/inode.h */
