#ifndef KERNEL
#define KERNEL
#include "pcb.h"
int process_initialize(char *filename);
int schedule_by_policy(char* policy); //, bool mt);
int shell_process_initialize();
void ready_queue_destory();
#endif