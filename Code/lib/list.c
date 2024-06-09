#include "list.h"

#include <assert.h>
#include <stdlib.h>

List *newList(void *(*dup)(void *), void (*free)(void *),
              int (*match)(void *, void *)) {
  List *list = malloc(sizeof(List));
  if (list == NULL) return NULL;
  list->head = list->tail = NULL;
  list->dup = dup;
  list->free = free;
  list->match = match;
  list->len = 0;
  return list;
}

void freeList(List *list) {
  ListNode *current = list->head;
  while (current != NULL) {
    ListNode *next = current->next;
    freeListNode(list, current);
    free(current);
    current = next;
  }
  free(list);
}

void listAddNodeHead(List *list, void *value) {
  ListNode *node = malloc(sizeof(ListNode));
  if (node == NULL) return;
  node->value = dupListNode(list, value);
  if (list->len == 0) {
    list->head = list->tail = node;
    node->prev = node->next = NULL;
  } else {
    node->prev = NULL;
    node->next = list->head;
    list->head->prev = node;
    list->head = node;
  }
  list->len++;
}

void listAddNodeTail(List *list, void *value) {
  ListNode *node = malloc(sizeof(ListNode));
  if (node == NULL) return;
  node->value = dupListNode(list, value);
  if (list->len == 0) {
    list->head = list->tail = node;
    node->prev = node->next = NULL;
  } else {
    node->prev = list->tail;
    node->next = NULL;
    list->tail->next = node;
    list->tail = node;
  }
  list->len++;
}

void listJoin(List *l, List *o) {
  assert(l != NULL && o != NULL);

  if (o->len == 0) return;
  if (l->len == 0) {
    l->head = o->head;
    l->tail = o->tail;
  } else {
    l->tail->next = o->head;
    o->head->prev = l->tail;
    l->tail = o->tail;
  }
  l->len += o->len;

  o->head = o->tail = NULL;
  o->len = 0;
}

ListIter *listGetIterator(List *list, int direction) {
  ListIter *iter = malloc(sizeof(ListIter));
  if (iter == NULL) return NULL;
  iter->direction = direction;
  iter->next = (direction == ITER_HEAD) ? list->head : list->tail;
  return iter;
}

ListNode *listNext(ListIter *iter) {
  ListNode *current = iter->next;
  if (current != NULL) {
    iter->next = (iter->direction == ITER_HEAD) ? current->next : current->prev;
  }
  return current;
}

void freeListIterator(ListIter *iter) { free(iter); }