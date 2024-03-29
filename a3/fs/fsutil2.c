#include "fsutil2.h"
#include "../interpreter.h"
#include "../linked_list.h"
#include "bitmap.h"
#include "block.h"
#include "cache.h"
#include "debug.h"
#include "directory.h"
#include "file.h"
#include "filesys.h"
#include "free-map.h"
#include "fsutil.h"
#include "inode.h"
#include "off_t.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -------- copy_in ---------
void partial_write_message(int bytes_written, int bufsize) {
  printf("Warning: could only write %d out of %d bytes (reached end of file\n",
         bytes_written, bufsize);
}

int get_overhead(int size) {
  /* Compute the overhead (number of extra data blocks used to store inodes and
   * adjacents)*/
  if (size <= DIRECT_BLOCKS_COUNT * BLOCK_SECTOR_SIZE) {
    return BLOCK_SECTOR_SIZE;
  }

  if (size <= DIRECT_BLOCKS_COUNT * BLOCK_SECTOR_SIZE +
                  INDIRECT_BLOCKS_PER_SECTOR * BLOCK_SECTOR_SIZE) {
    return 2 * BLOCK_SECTOR_SIZE;
  }
  double remainder = size - (DIRECT_BLOCKS_COUNT * BLOCK_SECTOR_SIZE +
                             INDIRECT_BLOCKS_PER_SECTOR * BLOCK_SECTOR_SIZE);
  double rem_data_blocks = ceil(remainder / BLOCK_SECTOR_SIZE);
  double pt_blocks_required =
      ceil(rem_data_blocks / INDIRECT_BLOCKS_PER_SECTOR);

  // space for inode + indirect block + double indirect block + di
  // indirectblocks
  return (3 + ceil(pt_blocks_required)) * BLOCK_SECTOR_SIZE;
}

size_t get_write_size(size_t bufsize, size_t free_space) {

  size_t write_size;

  // many logical braches to compute the write size
  if (free_space == 1) {
    // we only have space for an inode, no writing file content
    write_size = 0;

    return write_size;
  }

  if (bufsize >= free_space) {
    // the file is larger than the free space, write the free space minus its
    // overhead
    write_size = free_space;
    write_size -= get_overhead(write_size);

    return write_size;
  }

  int file_overhead = get_overhead(bufsize);
  int spaces_left_for_overhead = free_space - bufsize;

  if (file_overhead <= spaces_left_for_overhead) {
    // we have enough space to write both the entire file and its overhead
    write_size = bufsize;
    return write_size;
  }

  switch (spaces_left_for_overhead) {
  case 1:
    // only have enough space to store the direct blocks, so we can only
    // write into those
    write_size = DIRECT_BLOCKS_COUNT * BLOCK_SECTOR_SIZE;
    break;
  case 2:
  case 3:
    // only have enough space to store the direct blocks and the single
    // indirect blocks, so we can only write into those
    write_size = DIRECT_BLOCKS_COUNT * BLOCK_SECTOR_SIZE +
                 INDIRECT_BLOCKS_PER_SECTOR * BLOCK_SECTOR_SIZE;
    break;
  default:
    // we store the direct blocks + the single indirect blocks + as many
    // blocks of points from the double indirect blocks as possible
    spaces_left_for_overhead -= 3;
    write_size = DIRECT_BLOCKS_COUNT * BLOCK_SECTOR_SIZE +
                 INDIRECT_BLOCKS_PER_SECTOR * BLOCK_SECTOR_SIZE +
                 spaces_left_for_overhead * BLOCK_SECTOR_SIZE;
    break;
  }

  return write_size;
}

int copy_in(char *fname) {
  FILE *fp = fopen(fname, "r");
  int bufsize = 0;
  char *buffer = {};

  if (fp == NULL) {
    return FILE_DOES_NOT_EXIST;
  }

  fseek(fp, 0, SEEK_END);
  bufsize = ftell(fp) + 1;
  fseek(fp, 0, SEEK_SET);

  buffer = malloc((bufsize) * sizeof(char));
  memset(buffer, '\0', (bufsize) * sizeof(char));

  char c;
  int buffer_i = 0;
  while ((c = getc(fp)) != EOF && buffer_i < bufsize) {
    buffer[buffer_i++] = c;
  }

  buffer[buffer_i] = '\0';

  int num_free_sectors = fsutil_freespace();
  if (num_free_sectors == 0) {
    return FILE_WRITE_ERROR;
  }

  size_t free_space = num_free_sectors * BLOCK_SECTOR_SIZE;

  size_t write_size = get_write_size(bufsize, free_space);

  if (!fsutil_create(fname, write_size)) {
    return FILE_CREATION_ERROR;
  }

  int bytes_written = fsutil_write(fname, buffer, write_size);

  free(buffer);

  if (bytes_written == -1) {
    return FILE_WRITE_ERROR;
  }

  if (bytes_written != bufsize) {
    partial_write_message(bytes_written, bufsize + 1);
  }

  return 0;
}

// -------- copy_out ---------

int copy_out(char *fname) {
  if (!fsutil_file_exists(fname)) {
    return FILE_DOES_NOT_EXIST;
  }

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
    return FILE_READ_ERROR;
  }

  fprintf(fp, "%s", buffer);
  fclose(fp);
  free(buffer);

  return 0;
}

// -------- find_file ---------

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

// -------- fragmentation_degree ---------

void report_fragmentation_degree(int num_fragmentable, int num_fragmented,
                                 double pct) {
  printf("Num fragmentable files: %d\nNum fragmented files: %d\nFragmentation "
         "pct: %f\n",
         num_fragmentable, num_fragmented, pct);
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
        break;
      }
    }
  }

  double pct =
      num_fragmentable != 0 ? num_fragmented / (double)num_fragmentable : 0.0;

  report_fragmentation_degree(num_fragmentable, num_fragmented, pct);

  dir_close(dir);
}

// -------- defragment ---------

struct file_data {
  char *name;
  char *content;
  size_t size;
};

int defragment() {
  LINKED_LIST *existing_files = malloc(sizeof(LINKED_LIST));
  list_init(&existing_files, NULL);

  struct dir *dir;
  char name[NAME_MAX + 1];
  dir = dir_open_root();

  while (dir_readdir(dir, name)) {
    // load content into a list (filename, buffer, size)
    struct file_data *file_data = malloc(sizeof(struct file_data));

    struct file *f = filesys_open(name);
    offset_t f_length = file_length(f);

    char *file_content = malloc((f_length) * sizeof(char));

    file_seek(f, 0);
    file_read(f, file_content, f_length);
    file_seek(f, 0);

    file_data->name = strdup(name);
    file_data->content = file_content;
    file_data->size = f_length;

    add_tail(file_data, existing_files);

    fsutil_rm(name);
  }

  for (struct NODE *curr = existing_files->dummy_head->next;
       curr != NULL && curr != existing_files->dummy_tail; curr = curr->next) {

    struct file_data *file_data = curr->data;
    fsutil_create(file_data->name, file_data->size);
    fsutil_write(file_data->name, file_data->content, file_data->size);
  }

  list_clear(existing_files);

  return 0;
}

// -------- recover ---------

bool sector_is_inode(block_sector_t sector) {
  struct inode *inode_at_sector = inode_open(sector);
  return inode_at_sector->data.magic == INODE_MAGIC;
}

// recover deleted inodes
void recover_0() {

  for (block_sector_t sector_i = 0; sector_i < bitmap_size(free_map);
       sector_i++) {

    bool bit_is_free = (bitmap_test(free_map, sector_i) == false);

    if (!bit_is_free || !sector_is_inode(sector_i)) {
      continue;
    }

    struct inode *inode_to_recover = inode_open(sector_i);

    // --- set bitmap as occupied ---
    block_sector_t *direct_sectors = get_inode_data_sectors(inode_to_recover);
    int num_sectors_in_inode = bytes_to_sectors(inode_length(inode_to_recover));

    bitmap_set(free_map, sector_i, true);
    bitmap_set_multiple(free_map, direct_sectors[0], num_sectors_in_inode,
                        true);
    bitmap_set(free_map, inode_to_recover->data.doubly_indirect_block, true);
    bitmap_set(free_map, inode_to_recover->data.indirect_block, true);

    // ---- create file -----
    char file_name[256];
    sprintf(file_name, "recovered0-%d", sector_i);

    struct dir *root = dir_open_root();
    dir_add(root, file_name, sector_i, false);

    dir_close(root);
  }
}

// recover all non-empty sectors
void recover_1() {
  for (block_sector_t sector = 4; sector < bitmap_size(free_map); sector++) {
    // NOTE: Apparently we do not check for whether the bit is free (public
    // test 10)

    if (sector_is_inode(sector)) {
      continue;
    }

    char buffer[BLOCK_SECTOR_SIZE];
    buffer_cache_read(sector, buffer);

    bool buffer_is_empty = true;
    for (int i = 0; i < BLOCK_SECTOR_SIZE; i++) {
      if (buffer[i] != '\0') {
        buffer_is_empty = false;
        break;
      };
    }

    if (buffer_is_empty) {
      continue;
    }

    char file_name[256];
    sprintf(file_name, "recovered1-%d.txt", sector);

    fsutil_write(file_name, buffer, strlen(buffer));

    FILE *fp = fopen(file_name, "w");

    fprintf(fp, "%s", buffer);
    fclose(fp);
  }
}

// data past end of file.
void recover_2() {
  struct dir *dir;
  char name[NAME_MAX + 1];

  dir = dir_open_root();

  while (dir_readdir(dir, name)) {
    struct file *f = filesys_open(name);
    struct inode *f_inode = f->inode;

    block_sector_t *sector = get_inode_data_sectors(f_inode);
    int num_sectors_in_inode = bytes_to_sectors(inode_length(f_inode));
    block_sector_t last_sector = sector[num_sectors_in_inode - 1];

    // offset so we don't read the actual file's data, but only what's past it
    // in the block
    offset_t offset = (inode_length(f_inode) % BLOCK_SECTOR_SIZE);

    char buffer[BLOCK_SECTOR_SIZE];

    buffer_cache_read(last_sector, buffer);

    char file_name[256];
    sprintf(file_name, "recovered2-%s.txt", name);

    bool file_has_leftover_data = false;
    for (int i = offset; i < BLOCK_SECTOR_SIZE; i++) {
      if (buffer[i] != '\0') {
        file_has_leftover_data = true;
        break;
      }
    }

    if (!file_has_leftover_data) {
      continue;
    }

    FILE *fp = fopen(file_name, "w");

    for (int i = offset; i < BLOCK_SECTOR_SIZE; i++) {
      if (buffer[i] == '\0') {
        continue;
      }

      fputc(buffer[i], fp);
    }

    fclose(fp);
    file_close(f);
  }

  dir_close(dir);
}
int extract_hidden_data_from_metadata(const struct inode *inode, char *hidden_data) {
    // Assuming 'hidden_data' is sufficiently large and the inode structure contains a field 'extra_data'
    // that might hold hidden information. This is a fictional example to illustrate the concept.
    // In reality, you would need to analyze the specific inode structure for your filesystem.

    int found_hidden_data = -1;
    
    // Example: Check if the inode has a non-standard timestamp indicating hidden data.
    // This is purely illustrative and would depend on actual inode fields and how data might be hidden.
    if (inode->timestamp > SOME_ARBITRARY_VALUE) {
        // Suppose we determine that hidden data is indicated by a specific timestamp value.
        // The actual extraction method would depend on where and how data is hidden in the inode.
        
        // For example, if hidden data is stored in a reserved field:
        memcpy(hidden_data, inode->extra_data, 10000);
        found_hidden_data = 1;
    }

    // Another example: Check if 'extra_data' field, normally unused, contains non-zero data.
    if (!found_hidden_data && memcmp(inode->extra_data, "\0", SIZE_OF_EXTRA_DATA) != 0) {
        // Assume extra_data contains hidden data if it's not all zeros.
        memcpy(hidden_data, inode->extra_data, 10000);
        found_hidden_data = 1;
    }

    // Add other conditions as necessary depending on how data might be hidden within the inode.

    return found_hidden_data;
}
void recover_4() {
    // Iterate through each file in the filesystem.
    struct dir *dir = dir_open_root();
    char name[NAME_MAX + 1];
    while (dir_readdir(dir, name)) {
        struct file *f = filesys_open(name);
        if (!f) continue; // Skip if file cannot be opened.
        
        char hidden_data[10000];
        if (extract_hidden_data_from_metadata(f->inode, hidden_data)) {
            char file_name[256];
            sprintf(file_name, "recovered4-%s.txt", name);
            fsutil_write(file_name, hidden_data, 10000);
        }
        
        file_close(f); // Close the file.
    }
    dir_close(dir); // Close the directory.
}


void recover(int flag) {
  if (flag == 0) {
    recover_0();
  } else if (flag == 1) {
    recover_1();
  } else if (flag == 2) {
    recover_2();
  } else if (flag == 3) {
    recover_4();
  } else if (flag == 4) {
    recover_4();
  } else if (flag == 5) {
    recover_4();
  }
}
