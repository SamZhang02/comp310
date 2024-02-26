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

  int error_code = load_file(&fp, pid);
  if (error_code != 0) {
    fclose(fp);
    return FILE_ERROR;
  }

  pagetable pagetable = get_page_table(pid);
  int num_pages = get_num_pages(pid);

  PCB *newPCB = makePCB(pid, pagetable, num_pages, fp);
  QueueNode *node = malloc(sizeof(QueueNode));
  node->pcb = newPCB;

  ready_queue_add_to_tail(node);

  return 0;
}

int initialize_multiple_process(char *filename1, char *filename2,
                                char *filename3) {

  char *files[] = {filename1, filename2, filename3};

  FILE *fp1;
  FILE *fp2;
  FILE *fp3;

  int pid1 = -1;
  int pid2 = -1;
  int pid3 = -1;

  if (filename1 != NULL) {
    fp1 = fopen(filename1, "rt");
    pid1 = generatePID();
  }

  if (filename2 != NULL) {
    fp2 = fopen(filename2, "rt");
    pid2 = generatePID();
  }

  if (filename3 != NULL) {
    fp3 = fopen(filename3, "rt");
    pid3 = generatePID();
  }

  int error_code = load_multiple_files(&fp1, &fp2, &fp3, pid1, pid2, pid3);

  if (error_code != 0) {
    if (filename1 != NULL) {
      fclose(fp1);
    }

    if (filename2 != NULL) {
      fclose(fp2);
    }

    if (filename3 != NULL) {
      fclose(fp3);
    }

    return FILE_ERROR;
  }

  if (filename1 != NULL) {
    pagetable pagetable = get_page_table(pid1);
    int num_pages = get_num_pages(pid1);

    PCB *newPCB = makePCB(pid1, pagetable, num_pages, fp1);
    QueueNode *node = malloc(sizeof(QueueNode));
    node->pcb = newPCB;

    ready_queue_add_to_tail(node);
  }

  if (filename2 != NULL) {
    pagetable pagetable = get_page_table(pid2);
    int num_pages = get_num_pages(pid2);

    PCB *newPCB = makePCB(pid2, pagetable, num_pages, fp2);
    QueueNode *node = malloc(sizeof(QueueNode));
    node->pcb = newPCB;

    ready_queue_add_to_tail(node);
  }

  if (filename3 != NULL) {
    pagetable pagetable = get_page_table(pid3);
    int num_pages = get_num_pages(pid3);

    PCB *newPCB = makePCB(pid3, pagetable, num_pages, fp3);
    QueueNode *node = malloc(sizeof(QueueNode));
    node->pcb = newPCB;

    ready_queue_add_to_tail(node);
  }

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
