#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "kernel.h"
#include "linked_list.h"
#include "pcb.h"

LINKED_LIST *QUEUE;
LINKED_LIST *LRU_CACHE;
LINKED_LIST *ALL_PCB;
int PID_counter = 1;

int queue_done();

/**
 * @brief initializes ready queue, LRU cache and all added PCB's in the kernel
 *
 */
void kernel_setup() {
  QUEUE = (LINKED_LIST *)malloc(sizeof(LINKED_LIST));
  list_init(&QUEUE, &PCB_equal);
  LRU_CACHE = (LINKED_LIST *)malloc(sizeof(LINKED_LIST));
  list_init(&LRU_CACHE, &PAGE_equal);
  ALL_PCB = (LINKED_LIST *)malloc(sizeof(LINKED_LIST));
  list_init(&ALL_PCB, &PCB_equal);
}

/**
 * @brief makes a process to run 1 line; if a page fault occurs,
 * loads 1 page from the backing store and re-inserts the process to the head of
 * the ready queue; updates the LRU cache
 *
 * @param pcb
 * @param page_index
 * @param start_pos
 * @param program_counter
 * @param size
 * @param valid_bit
 * @return int 1: page fault; 0: successful
 */
int run_proc(PCB *pcb, int page_index, int *start_pos, int *program_counter,
             int *size, int *valid_bit, char *cwd) {
  int ret = cpu_run(start_pos, program_counter, size, valid_bit, cwd);
  if (ret == 1) {
    if (load_PAGE(pcb, page_index) == 1) {
      PAGE *victim_page = (PAGE *)pop_tail(LRU_CACHE);
      NODE *curr = ALL_PCB->dummy_head->next;
      while (curr != QUEUE->dummy_tail &&
             (((PCB *)curr->data)->pid != victim_page->page_pid)) {
        curr = curr->next;
      }
      if (!curr) {
        printf("cannot find PCB to be deleted\n");
        exit(99);
      }
      PAGE_evict((PCB *)curr->data, victim_page->page_index);
      load_PAGE(pcb, page_index);
    }
    add_head((void *)pcb->page_table[page_index], LRU_CACHE);
    add_tail((void *)pcb, QUEUE);
    return 1;
  }
  add_head(remove_elem((void *)pcb->page_table[page_index], LRU_CACHE),
           LRU_CACHE);
  return 0;
}

/**
 * @brief Creates a new process; updates the ready queue and LRU cache
 *
 * @param script
 * @return int 1: unable to append a PCB to the ready queue; 0: successful
 */
int new_proc(const char *script) {
  PCB *new_PCB = PCB_init(PID_counter++, script);
  add_tail(new_PCB, ALL_PCB);

  if (add_tail((void *)new_PCB, QUEUE) != 0) {
    return 1;
  } else {
    int page_limit =
        2 > new_PCB->page_table_size ? new_PCB->page_table_size : 2;
    for (int i = 0; i < page_limit; i++) {
      add_head((void *)new_PCB->page_table[i], LRU_CACHE);
    }

    return 0;
  }
}

/**
 * @brief FIFO scheduling policy
 *
 */
int run_proc_FIFO(char *cwd) {
  // execute each PCB in the ready queue one by one
  while (QUEUE->size > 0) {
    PCB *curr = (PCB *)pop_head(QUEUE);
    while (!(proc_done(&curr->size))) {
      int i = 0, j = 0;
      while (curr->page_table[i]->valid_bit[j] == 0) {
        if (j < 2) {
          j++;
          continue;
        } else {
          i++, j = 0;
        }
      }
      run_proc(curr, i, &curr->page_table[i]->index[j],
               &curr->program_counter[i], &curr->size,
               &curr->page_table[i]->valid_bit[j], cwd);
    }
    free_PCB(curr);
  }
  return 0;
}

/**
 * @brief RR scheduling policy
 *
 */
int run_proc_RR(char *cwd) {
  // execute each PCB in the ready queue one by one
  while (!queue_done()) {
    PCB *curr = (PCB *)(pop_head(QUEUE));
    int i, ret;
    // if the size of the current PCB is greater than 1, run 2 lines
    // then insert it back to the tail of the ready queue
    if (curr->size > 1) {
      for (i = 0; i < curr->page_table_size; i++) {
        if (curr->program_counter[i] != 3) {
          break;
        }
      }
      ret = run_proc(
          curr, i, &curr->page_table[i]->index[curr->program_counter[i]],
          &curr->program_counter[i], &curr->size,
          &curr->page_table[i]->valid_bit[curr->program_counter[i]], cwd);
      if (ret == 1) {
        continue;
      }
      for (i = 0; i < curr->page_table_size; i++) {
        if (curr->program_counter[i] != 3) {
          break;
        }
      }
      ret = run_proc(
          curr, i, &curr->page_table[i]->index[curr->program_counter[i]],
          &curr->program_counter[i], &curr->size,
          &curr->page_table[i]->valid_bit[curr->program_counter[i]], cwd);
      if (ret == 1) {
        continue;
      }
      add_tail((void *)curr, QUEUE);
      continue;
    }
    // else only run the current PCB once, then free it, or directly free it if
    // size == 0
    else if (curr->size == 1) {
      for (i = 0; i < curr->page_table_size; i++) {
        if (curr->program_counter[i] != 3) {
          break;
        }
      }
      ret = run_proc(
          curr, i, &curr->page_table[i]->index[curr->program_counter[i]],
          &curr->program_counter[i], &curr->size,
          &curr->page_table[i]->valid_bit[curr->program_counter[i]], cwd);
      if (ret == 1) {
        continue;
      }
    }
  }

  NODE *curr = ALL_PCB->dummy_head->next;
  while (has_next(&curr)) {
    free_PCB((PCB *)next(&curr));
  }
  list_clear(QUEUE);
  list_clear(LRU_CACHE);
  list_clear(ALL_PCB);
  return 0;
}

/**
 * @brief check if every PCB in the ready queue is done
 *
 * @return int
 */
int queue_done() {
  NODE *curr = QUEUE->dummy_head->next;
  while (has_next(&curr)) {
    if (!PCB_done((PCB *)next(&curr))) {
      return 0;
    }
  }
  return 1;
}