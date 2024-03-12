#include "ide.h"
#include "block.h"
#include "debug.h"
#include "partition.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

/* The code in this file is an interface to an ATA (IDE)
   controller.  It attempts to comply to [ATA-3]. */

/* An ATA device. */
struct ata_disk {
  char name[8];            /* Name, e.g. "hda". */
  struct channel *channel; /* Channel that disk is attached to. */
  int dev_no;              /* Device 0 or 1 for master or slave. */
  bool is_ata;             /* Is device an ATA disk? */
  char *fname;
  int fd;
};

/* An ATA channel (aka controller).
   Each channel can control up to two disks. */
struct channel {
  char name[8];               /* Name, e.g. "ide0". */
  struct ata_disk devices[2]; /* The devices on this channel. */
};

/* We support the two "legacy" ATA channels found in a standard PC. */
#define CHANNEL_CNT 2
static struct channel channels[CHANNEL_CNT];

static struct block_operations ide_operations;

static void identify_ata_device(struct ata_disk *);

/* Initialize the disk subsystem and detect disks. */
void ide_init(char *hd) {
  struct channel *c = &channels[0];
  struct ata_disk *d = &c->devices[0];
  int tmp = 'a' + 0 * 2 + 0;
  snprintf(d->name, sizeof d->name, "hd%c", tmp);
  d->channel = c;
  d->dev_no = 0;
  d->is_ata = true;
  d->fname = hd;
  d->fd = open(hd, O_RDWR);
  if (d->fd == 1)
    PANIC("FD -1");
  identify_ata_device(&c->devices[0]);
}

/* Sends an IDENTIFY DEVICE command to disk D and reads the
   response.  Registers the disk with the block device layer. */
static void identify_ata_device(struct ata_disk *d) {
  block_sector_t capacity;
  struct block *block;

  ASSERT(d->is_ata);

  /* Calculate capacity.
     Read model name and serial number. */
  struct stat st;
  stat(d->fname, &st);
  capacity = st.st_size / BLOCK_SECTOR_SIZE;

  // Register.
  block = block_register(d->name, d->fname, capacity, &ide_operations, d);
  partition_scan(block, d->fname);
}

/* Reads sector SEC_NO from disk D into BUFFER, which must have
   room for BLOCK_SECTOR_SIZE bytes. */
static void ide_read(void *d_, block_sector_t sec_no, void *buffer) {

  struct ata_disk *d = d_;
  lseek(d->fd, sec_no * BLOCK_SECTOR_SIZE, SEEK_SET);
  read(d->fd, buffer, BLOCK_SECTOR_SIZE);
}

/* Write sector SEC_NO to disk D from BUFFER, which must contain
   BLOCK_SECTOR_SIZE bytes. */
static void ide_write(void *d_, block_sector_t sec_no, const void *buffer) {
  struct ata_disk *d = d_;
  lseek(d->fd, sec_no * BLOCK_SECTOR_SIZE, SEEK_SET);
  write(d->fd, buffer, BLOCK_SECTOR_SIZE);
}

static struct block_operations ide_operations = {ide_read, ide_write};