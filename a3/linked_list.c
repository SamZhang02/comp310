#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"

NODE *get_node_by_index(LINKED_LIST *list, int index);
NODE *get_node_by_value(LINKED_LIST *list, void *target_data);

/**
 * @brief check if the current node has a next node
 *
 * @param elem
 * @return int
 */
int has_next(NODE **elem) {
  if ((*elem)->next != NULL) {
    return 1;
  }
  return 0;
}

/**
 * @brief move the current node pointer to its next node, and return the current
 * node
 *
 * @param node
 * @return NODE*
 */
NODE *next_node(NODE **node) {
  NODE *ret = *node;
  *node = (*node)->next;
  return ret;
}

/**
 * @brief executes "next_node", returns the data stored in the current node
 *
 * @param elem
 * @return void*
 */
void *next(NODE **elem) {
  NODE *ret = next_node(elem);
  return ret->data;
}

/**
 * @brief reset the linked list to the initial state
 *
 * @param list
 */
void list_clear(LINKED_LIST *list) {
  list->size = 0;
  list->dummy_head->next = list->dummy_tail;
  list->dummy_tail->prev = list->dummy_head;
}

/**
 * @brief append an element to the end of a linked list
 *
 * @param data
 * @param list
 * @return int
 */
int add_tail(void *data, LINKED_LIST *list) {
  NODE *new_node = (NODE *)malloc(sizeof(NODE));
  NODE *old_tail = list->dummy_tail->prev;
  new_node->data = data;

  old_tail->next = new_node;
  new_node->prev = old_tail;
  new_node->next = list->dummy_tail;
  list->dummy_tail->prev = new_node;

  list->size++;

  return 0;
}

/**
 * @brief append an element to the beginning of a linked list
 *
 * @param data
 * @param list
 * @return int
 */
int add_head(void *data, LINKED_LIST *list) {
  NODE *new_node = (NODE *)malloc(sizeof(NODE));
  NODE *old_head = list->dummy_head->next;
  new_node->data = data;

  old_head->prev = new_node;
  new_node->next = old_head;
  new_node->prev = list->dummy_head;
  list->dummy_head->next = new_node;

  list->size++;

  return 0;
}

/**
 * @brief removes an element from the beginning of the linked list and returns
 * the data
 *
 * @param list
 * @return void*
 */
void *pop_head(LINKED_LIST *list) {
  if (list->size <= 0) {
    printf("Trying to pop head element from an empty linked list\n");
    exit(99);
  }
  NODE *to_be_deleted = list->dummy_head->next;
  to_be_deleted->prev->next = to_be_deleted->next;
  to_be_deleted->next->prev = to_be_deleted->prev;

  list->size--;

  return to_be_deleted->data;
}

/**
 * @brief Get the node by index object
 *
 * @param list
 * @param index
 * @return NODE*
 */
NODE *get_node_by_index(LINKED_LIST *list, int index) {
  // check if index is valid
  if (index >= list->size || index < 0) {
    printf("Invalid index to get from a linked list\n");
    return NULL;
  }

  NODE *curr = list->dummy_head->next;
  for (int i = 0; i < index; i++) {
    curr = curr->next;
  }
  return curr;
}

/**
 * @brief Get the node by value object
 *
 * @param list
 * @param target_data
 * @return NODE*
 */
NODE *get_node_by_value(LINKED_LIST *list, void *target_data) {
  NODE *curr = list->dummy_head->next;
  while (curr && !list->equal(target_data, curr->data)) {
    curr = curr->next;
  }
  return curr;
}

/**
 * @brief Get the data by index object
 *
 * @param list
 * @param index
 * @return void*
 */
void *get_by_index(LINKED_LIST *list, int index) {
  NODE *target_node = get_node_by_index(list, index);
  return target_node->data;
}

/**
 * @brief Get the data by value object
 *
 * @param list
 * @param elem
 * @return void*
 */
void *get_by_value(LINKED_LIST *list, void *elem) {
  NODE *target_node = get_node_by_value(list, elem);
  return target_node->data;
}

/**
 * @brief removes a given element from a linked list
 *
 * @param data_to_del
 * @param list
 * @return void*
 */
void *remove_elem(void *data_to_del, LINKED_LIST *list) {
  NODE *to_be_deleted = get_node_by_value(list, data_to_del);
  to_be_deleted->prev->next = to_be_deleted->next;
  to_be_deleted->next->prev = to_be_deleted->prev;

  list->size--;

  return to_be_deleted->data;
}

/**
 * @brief swap two elements in a linked list
 *
 * @param node1
 * @param node2
 */
void swap(NODE *node1, NODE *node2) {
  void *tmp = node1->data;
  node1->data = node2->data;
  node2->data = tmp;
}

/**
 * @brief removes an element from the end of a linked list, returns the data
 *
 * @param list
 * @return void*
 */
void *pop_tail(LINKED_LIST *list) {
  if (list->size <= 0) {
    printf("Trying to pop tail element from an empty linked list\n");
    exit(99);
  }
  NODE *to_be_deleted = list->dummy_tail->prev;
  to_be_deleted->prev->next = to_be_deleted->next;
  to_be_deleted->next->prev = to_be_deleted->prev;

  list->size--;

  return to_be_deleted->data;
}

/**
 * @brief initializes an empty linked list
 *
 * @param list
 * @param equal
 */
void list_init(LINKED_LIST **list, int (*equal)(const void *a, const void *b)) {
  NODE *dummy_head = (NODE *)malloc(sizeof(NODE));
  NODE *dummy_tail = (NODE *)malloc(sizeof(NODE));
  dummy_tail->next = NULL;
  (*list)->size = 0;
  (*list)->dummy_head = dummy_head;
  dummy_head->next = dummy_tail;
  dummy_tail->prev = dummy_head;
  (*list)->dummy_tail = dummy_tail;
  (*list)->equal = equal;
}