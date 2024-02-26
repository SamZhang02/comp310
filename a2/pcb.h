#ifndef PCB_H
#define PCB_H
#include <stdbool.h>
#include <stdio.h>

/*
 * Struct:  PCB
 * --------------------
 * pid: process(task) id
 * PC: program counter, stores the index of line that the task is executing
 * start: the first line in shell memory that belongs to this task
 * end: the last line in shell memory that belongs to this task
 * job_length_score: for EXEC AGING use only, stores the job length score
 */

typedef struct {
  bool priority;
  int pid;
  int curr_page; // current page being read
  int curr_line; // current line in the page being read
  int *pagetable;
  int num_pages;
  int job_length_score;
  FILE *fp;           // we store the file pointer to continuously read from it
  long file_position; // keep track of the current file position being read
  bool file_is_done;
} PCB;

typedef int *pagetable;

int generatePID();
PCB *makePCB(int pid, int *pagetable, int num_pages, FILE *fp);
bool fetch_a_page(PCB *self);
#endif
