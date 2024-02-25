#include "kernel.h"
#include "framestore.h"
#include "interpreter.h"
#include "pcb.h"
#include "ready_queue.h"
#include "shell.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool active = false;
bool debug = false;
bool in_background = false;

int process_initialize(char *filename) {
  FILE *fp;

  fp = fopen(filename, "rt");

  if (fp == NULL) {
    return FILE_DOES_NOT_EXIST;
  }

  int pid = generatePID();

  int error_code = load_file(&fp, filename, pid);
  if (error_code != 0) {
    fclose(fp);
    return FILE_ERROR;
  }

  pagetable pagetable = get_page_table(pid);
  int num_pages = get_num_pages(pid);

  PCB *newPCB = makePCB(pid, pagetable, num_pages, 100, fp);
  QueueNode *node = malloc(sizeof(QueueNode));
  node->pcb = newPCB;

  ready_queue_add_to_tail(node);

  return 0;
}

// From starter code:
// Note that "You can assume that the # option will only be used in batch
// mode." So we know that the input is a file, we can directly load the file
// into ram
int shell_process_initialize() {
  int error_code = 0;

  int pid = generatePID();
  error_code = load_file(&stdin, "_SHELL", pid);
  if (error_code != 0) {
    return error_code;
  }

  pagetable pagetable = get_page_table(pid);
  int num_pages = get_num_pages(pid);

  PCB *newPCB = makePCB(pid, pagetable, num_pages, 100, stdin);

  newPCB->priority = true;
  QueueNode *node = malloc(sizeof(QueueNode));
  node->pcb = newPCB;

  ready_queue_add_to_head(node);

  freopen("/dev/tty", "r", stdin);
  return 0;
}

bool execute_process(QueueNode *node, int quanta) {
  char *line = NULL;
  PCB *pcb = node->pcb;

  for (int i = 0; i < quanta; i++) {
    bool page_fault = pcb->curr_page >= pcb->num_pages;

    if (page_fault) {
      bool page_was_fetched = fetch_a_page(pcb);

      if (page_was_fetched == false) {
        terminate_process(node);
        return true;
      } else {
        return false;
      }
    }

    int page_num = pcb->pagetable[pcb->curr_page];
    int line_num = pcb->curr_line;

    line = get_line(page_num, line_num);

    in_background = true;

    if (pcb->priority) {
      pcb->priority = false;
    }

    // if the line is none, that means the program is done
    if (strcmp(line, "none") == 0) {
      terminate_process(node);
      in_background = false;
      return true;
    }

    parseInput(line);
    in_background = false;

    if (pcb->curr_line == 2) {
      pcb->curr_line = 0;
      pcb->curr_page++;
    } else {
      pcb->curr_line++;
    }
  }

  return false;
}

void *scheduler_FCFS() {
  QueueNode *cur;

  while (true) {
    if (is_ready_empty()) {
      if (active)
        continue;
      else
        break;
    }

    cur = ready_queue_pop_head();
    bool process_done = execute_process(cur, MAX_INT);
    if (!process_done) {
      ready_queue_add_to_tail(cur);
    }
  }

  return 0;
}

void *scheduler_SJF() {
  QueueNode *cur;
  while (true) {
    if (is_ready_empty()) {
      if (active)
        continue;
      else
        break;
    }
    cur = ready_queue_pop_shortest_job();
    execute_process(cur, MAX_INT);
  }
  return 0;
}

void *scheduler_AGING_alternative() {
  QueueNode *cur;
  while (true) {
    if (is_ready_empty()) {
      if (active)
        continue;
      else
        break;
    }
    cur = ready_queue_pop_shortest_job();
    ready_queue_decrement_job_length_score();
    if (!execute_process(cur, 1)) {
      ready_queue_add_to_head(cur);
    }
  }
  return 0;
}

void *scheduler_AGING() {
  QueueNode *cur;
  int shortest;
  sort_ready_queue();
  while (true) {
    if (is_ready_empty()) {
      if (active)
        continue;
      else
        break;
    }
    cur = ready_queue_pop_head();
    shortest = ready_queue_get_shortest_job_score();
    if (shortest < cur->pcb->job_length_score) {
      ready_queue_promote(shortest);
      ready_queue_add_to_tail(cur);
      cur = ready_queue_pop_head();
    }
    ready_queue_decrement_job_length_score();
    if (!execute_process(cur, 1)) {
      ready_queue_add_to_head(cur);
    }
  }
  return 0;
}

void *scheduler_RR(void *arg) {
  int quanta = ((int *)arg)[0];
  QueueNode *cur;
  while (true) {
    if (is_ready_empty()) {
      if (active)
        continue;
      else
        break;
    }

    cur = ready_queue_pop_head();
    bool process_done = execute_process(cur, quanta);
    if (!process_done) {
      ready_queue_add_to_tail(cur);
    }
  }
  return 0;
}

int schedule_by_policy(char *policy) { //, bool mt){
  if (strcmp(policy, "FCFS") != 0 && strcmp(policy, "SJF") != 0 &&
      strcmp(policy, "RR") != 0 && strcmp(policy, "AGING") != 0 &&
      strcmp(policy, "RR30") != 0) {
    return SCHEDULING_ERROR;
  }
  if (active)
    return 0;
  if (in_background)
    return 0;
  int arg[1];
  if (strcmp("FCFS", policy) == 0) {
    scheduler_FCFS();
  } else if (strcmp("SJF", policy) == 0) {
    scheduler_SJF();
  } else if (strcmp("RR", policy) == 0) {
    arg[0] = 2;
    scheduler_RR((void *)arg);
  } else if (strcmp("AGING", policy) == 0) {
    scheduler_AGING();
  } else if (strcmp("RR30", policy) == 0) {
    arg[0] = 30;
    scheduler_RR((void *)arg);
  }
  return 0;
}
