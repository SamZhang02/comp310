#ifndef DEVICES_BLOCK_H
#define DEVICES_BLOCK_H

#include <inttypes.h>
#include <stddef.h>

/* Size of a block device sector in bytes.
   All IDE disks use this sector size, as do most USB and SCSI
   disks. */
#define BLOCK_SECTOR_SIZE 512

/* Index of a block device sector.
   Good enough for devices up to 2 TB. */
typedef uint32_t block_sector_t;

/* Format specifier for printf(), e.g.:
   printf ("sector=%"PRDSNu"\n", sector); */
#define PRDSNu PRIu32

/* Higher-level interface for file systems, etc. */

struct block;

/* Finding block devices. */
struct block *block_get_hd();
void block_set_hd(struct block *);

/* Block device operations. */
block_sector_t block_size(struct block *);
void block_read(struct block *, block_sector_t, void *);
void block_write(struct block *, block_sector_t, const void *);
const char *block_name(struct block *);

/* Lower-level interface to block device drivers. */

struct block_operations {
  void (*read)(void *aux, block_sector_t, void *buffer);
  void (*write)(void *aux, block_sector_t, const void *buffer);
};

struct block *block_register(const char *name, const char *fname,
                             block_sector_t size,
                             const struct block_operations *, void *aux);

#endif /* fs/block.h */
