#ifndef READY_QUEUE_H
#define READY_QUEUE_H
#include "pcb.h"
#define MAX_INT 2147483646
typedef struct QueueNode{
    PCB *pcb;
    struct QueueNode *next;
} QueueNode;
void ready_queue_destory();
void ready_queue_add_to_tail(QueueNode *node);
void print_ready_queue();
void terminate_process(QueueNode *node);
bool is_ready_empty();
QueueNode *ready_queue_pop_shortest_job();
void ready_queue_head_to_tail();
void ready_queue_add_to_head(QueueNode *node);
QueueNode *ready_queue_pop_head();
void ready_queue_decrement_job_length_score();
void sort_ready_queue();
int ready_queue_get_shortest_job_score();
void ready_queue_promote(int score);
#endif