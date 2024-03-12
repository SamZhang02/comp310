
#include "partition.h"
#include "block.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct block_operations partition_operations;

static void read_partition_table(struct block *, block_sector_t sector,
                                 block_sector_t primary_extended_sector,
                                 int *part_nr, char *fname);
static void found_partition(struct block *, uint8_t type, block_sector_t start,
                            block_sector_t size, int part_nr, char *fname);

/* Scans BLOCK for partitions of interest to Pintos. */
void partition_scan(struct block *block, char *fname) {
  int part_nr = 0;
  read_partition_table(block, 0, 0, &part_nr, fname);
  if (part_nr == 0)
    printf("%s: Device contains no partitions\n", block_name(block));
}

/* Reads the partition table in the given SECTOR of BLOCK and
   scans it for partitions of interest to Pintos.

   If SECTOR is 0, so that this is the top-level partition table
   on BLOCK, then PRIMARY_EXTENDED_SECTOR is not meaningful;
   otherwise, it should designate the sector of the top-level
   extended partition table that was traversed to arrive at
   SECTOR, for use in finding logical partitions (see the large
   comment below).

   PART_NR points to the number of non-empty primary or logical
   partitions already encountered on BLOCK.  It is incremented as
   partitions are found. */
static void read_partition_table(struct block *block, block_sector_t sector,
                                 block_sector_t primary_extended_sector,
                                 int *part_nr, char *fname) {
  struct partition_table *pt;
  size_t i;

  /* Check SECTOR validity. */
  if (sector >= block_size(block)) {
    printf("%s: Partition table at sector %" PRDSNu " past end of device.\n",
           block_name(block), sector);
    return;
  }

  /* Read sector. */
  ASSERT(sizeof *pt == BLOCK_SECTOR_SIZE);
  pt = malloc(sizeof *pt);
  if (pt == NULL)
    PANIC("Failed to allocate memory for partition table.");
  block_read(block, 0, pt);

  /* Check signature. */
  if (pt->signature != 0xaa55) {
    if (primary_extended_sector == 0)
      printf("%s: Invalid partition table signature\n", block_name(block));
    else
      printf("%s: Invalid extended partition table in sector %" PRDSNu "\n",
             block_name(block), sector);
    free(pt);
    return;
  }

  /* Parse partitions. */
  for (i = 0; i < sizeof pt->partitions / sizeof *pt->partitions; i++) {
    struct partition_table_entry *e = &pt->partitions[i];

    if (e->size == 0 || e->type == 0) {
      /* Ignore empty partition. */
    } else if (e->type == 0x05     /* Extended partition. */
               || e->type == 0x0f  /* Windows 98 extended partition. */
               || e->type == 0x85  /* Linux extended partition. */
               || e->type == 0xc5) /* DR-DOS extended partition. */
    {
      printf("%s: Extended partition in sector %" PRDSNu "\n",
             block_name(block), sector);

      /* The interpretation of the offset field for extended
         partitions is bizarre.  When the extended partition
         table entry is in the master boot record, that is,
         the device's primary partition table in sector 0, then
         the offset is an absolute sector number.  Otherwise,
         no matter how deep the partition table we're reading
         is nested, the offset is relative to the start of
         the extended partition that the MBR points to. */
      if (sector == 0)
        read_partition_table(block, e->offset, e->offset, part_nr, fname);
      else
        read_partition_table(block, e->offset + primary_extended_sector,
                             primary_extended_sector, part_nr, fname);
    } else {
      ++*part_nr;

      found_partition(block, e->type, e->offset + sector, e->size, *part_nr,
                      fname);
    }
  }

  free(pt);
}

/* We have found a primary or logical partition of the given TYPE
   on BLOCK, starting at sector START and continuing for SIZE
   sectors, which we are giving the partition number PART_NR.
   Check whether this is a partition of interest to Pintos, and
   if so then add it to the proper element of partitions[]. */
static void found_partition(struct block *block, uint8_t part_type,
                            block_sector_t start, block_sector_t size,
                            int part_nr, char *fname) {
  if (start >= block_size(block))
    printf("%s%d: Partition starts past end of device (sector %" PRDSNu ")\n",
           block_name(block), part_nr, start);
  else if (start + size < start || start + size > block_size(block))
    printf("%s%d: Partition end (%" PRDSNu ") past end of device (%" PRDSNu
           ")\n",
           block_name(block), part_nr, start + size, block_size(block));
  else {
    struct partition *p;
    char name[16];

    p = malloc(sizeof *p);
    if (p == NULL)
      PANIC("Failed to allocate memory for partition descriptor");
    p->block = block;
    p->start = start;

    snprintf(name, sizeof name, "%s%d", block_name(block), part_nr);

    block_register(name, fname, size, &partition_operations, p);
  }
}

/* Reads sector SECTOR from partition P into BUFFER, which must
   have room for BLOCK_SECTOR_SIZE bytes. */
static void partition_read(void *p_, block_sector_t sector, void *buffer) {
  struct partition *p = p_;
  block_read(p->block, p->start + sector + 1, buffer);
}

/* Write sector SECTOR to partition P from BUFFER, which must
   contain BLOCK_SECTOR_SIZE bytes.  Returns after the block has
   acknowledged receiving the data. */
static void partition_write(void *p_, block_sector_t sector,
                            const void *buffer) {
  struct partition *p = p_;
  block_write(p->block, p->start + sector + 1, buffer);
}

static struct block_operations partition_operations = {partition_read,
                                                       partition_write};
