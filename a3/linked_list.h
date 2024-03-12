#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct NODE {
  void *data;
  struct NODE *next;
  struct NODE *prev;
} NODE;

typedef struct LINKED_LIST {
  int size;
  NODE *dummy_head;
  NODE *dummy_tail;
  int (*equal)(const void *a, const void *b);
} LINKED_LIST;

int add_tail(void *data, LINKED_LIST *list);
int add_head(void *data, LINKED_LIST *list);
void *pop_head(LINKED_LIST *list);
void *pop_tail(LINKED_LIST *list);
void *get_by_index(LINKED_LIST *list, int index);
void *get_by_value(LINKED_LIST *list, void *elem);
void *remove_elem(void *data_to_del, LINKED_LIST *list);
void swap(NODE *node1, NODE *node2);
int has_next(NODE **elem);
void *next(NODE **elem);
void list_clear(LINKED_LIST *list);
void list_init(LINKED_LIST **list, int (*equal)(const void *a, const void *b));

#endif