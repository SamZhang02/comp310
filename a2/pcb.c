#include "pcb.h"
#include <stdlib.h>

// global pid counter
int pid_counter = 1;

int generatePID() { return pid_counter++; }

PCB *makePCB(int *pagetable, int job_length_score) {
  PCB *newPCB = malloc(sizeof(PCB));
  if (newPCB == NULL) {
    return NULL;
  }

  newPCB->pid = generatePID();
  newPCB->job_length_score = job_length_score;
  newPCB->priority = false;
  newPCB->pagetable = pagetable;
  return newPCB;
}
