#ifndef KERNEL
#define KERNEL
#include "pcb.h"
int process_initialize(char *filename);
int schedule_by_policy(char *policy); //, bool mt);
void ready_queue_destory();
int initialize_multiple_process(char *filename1, char *filename2,
                                char *filename3);
#endif
