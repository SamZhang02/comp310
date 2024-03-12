#ifndef DEVICES_PARTITION_H
#define DEVICES_PARTITION_H

#define PACKED __attribute__((packed))

#include "block.h"

struct block;

/* A partition of a block device. */
struct partition {
  struct block *block;  /* Underlying block device. */
  block_sector_t start; /* First sector within device. */
};

/* Format of a partition table entry.  See [Partitions]. */
struct partition_table_entry {
  uint8_t bootable;     /* 0x00=not bootable, 0x80=bootable. */
  uint8_t start_chs[3]; /* Encoded starting cylinder, head, sector. */
  uint8_t type;         /* Partition type (see partition_type_name). */
  uint8_t end_chs[3];   /* Encoded ending cylinder, head, sector. */
  uint32_t offset;      /* Start sector offset from partition table. */
  uint32_t size;        /* Number of sectors. */
} PACKED;

/* Partition table sector. */
struct partition_table {
  uint8_t loader[446]; /* Loader, in top-level partition table. */
  struct partition_table_entry partitions[4]; /* Table entries. */
  uint16_t signature;                         /* Should be 0xaa55. */
} PACKED;

void partition_scan(struct block *, char *);

#endif /* fs/partition.h */
