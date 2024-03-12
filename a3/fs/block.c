#include "block.h"
#include "debug.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A block device. */
struct block {
  char name[16];       /* Block device name. */
  char fname[100];     // actual file on your real hard drive (e.g., test.dsk)
  block_sector_t size; /* Size in sectors. */

  const struct block_operations *ops; /* Driver operations. */
  void *aux;                          /* Extra data owned by driver. */

  unsigned long long read_cnt;  /* Number of sectors read. */
  unsigned long long write_cnt; /* Number of sectors written. */
};

struct block *hard_drive;

struct block *block_get_hd() { return hard_drive; }

void block_set_hd(struct block *block) { hard_drive = block; }

/* Verifies that SECTOR is a valid offset within BLOCK.
   Panics if not. */
static void check_sector(struct block *block, block_sector_t sector) {
  if (sector >= block->size) {
    /* We do not use ASSERT because we want to panic here
       regardless of whether NDEBUG is defined. */
    PANIC("Access past end of device %s (sector=%" PRDSNu ", "
          "size=%" PRDSNu ")\n",
          block_name(block), sector, block->size);
  }
}

/* Reads sector SECTOR from BLOCK into BUFFER, which must
   have room for BLOCK_SECTOR_SIZE bytes. */
void block_read(struct block *block, block_sector_t sector, void *buffer) {
  ASSERT(block != NULL);
  check_sector(block, sector);
  block->ops->read(block->aux, sector, buffer);
  block->read_cnt++;
}

/* Write sector SECTOR to BLOCK from BUFFER, which must contain
   BLOCK_SECTOR_SIZE bytes.  Returns after the block device has
   acknowledged receiving the data. */
void block_write(struct block *block, block_sector_t sector,
                 const void *buffer) {
  check_sector(block, sector);
  block->ops->write(block->aux, sector, buffer);
  block->write_cnt++;
}

/* Returns the number of sectors in BLOCK. */
block_sector_t block_size(struct block *block) { return block->size; }

/* Returns BLOCK's name (e.g. "hda"). */
const char *block_name(struct block *block) { return block->name; }

/* Registers a new block device with the given NAME.
The block device's SIZE in sectors and its TYPE must
   be provided, as well as the it operation functions OPS, which
   will be passed AUX in each function call. */
struct block *block_register(const char *name, const char *fname,
                             block_sector_t size,
                             const struct block_operations *ops, void *aux) {
  struct block *block = malloc(sizeof *block);
  if (block == NULL)
    PANIC("Failed to allocate memory for block device descriptor");

  strncpy(block->name, name, sizeof block->name);
  block->size = size;
  block->ops = ops;
  block->aux = aux;
  block->read_cnt = 0;
  block->write_cnt = 0;
  strncpy(block->fname, fname, sizeof block->fname);

  printf("%s: %'" PRDSNu " sectors (", block->name, block->size);
  print_human_readable_size((uint64_t)block->size * BLOCK_SECTOR_SIZE);
  printf(")\n");

  block_set_hd(block);

  return block;
}
