#ifndef SHELL_H
#define SHELL_H

extern char backing_store_path[1024];

int parseInput(char *ui);
int removeBackingStore();
#endif
