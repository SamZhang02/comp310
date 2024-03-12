#ifndef FILESYS_FSUTIL_H
#define FILESYS_FSUTIL_H

int fsutil_ls(char *);
int fsutil_cat(char *);
int fsutil_rm(char *);

int fsutil_create(const char *fname, unsigned isize);
int fsutil_write(char *file_name, const void *buffer, unsigned size);
int fsutil_read(char *file_name, void *buffer, unsigned size);
int fsutil_size(char *file_name);
int fsutil_seek(char *file_name, int offset);
void fsutil_close(char *file_name);
int fsutil_freespace();

#endif /* fs/fsutil.h */
