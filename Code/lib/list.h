#ifndef LIST_H
#define LIST_H

typedef struct ListNode {
  struct ListNode *prev;
  struct ListNode *next;
  void *value;
} ListNode;

typedef struct ListIter {
  ListNode *next;
  int direction;
} ListIter;

typedef struct List {
  ListNode *head;
  ListNode *tail;
  void *(*dup)(void *ptr);
  void (*free)(void *ptr);
  int (*match)(void *ptr, void *key);
  unsigned long len;
} List;

#define ITER_HEAD 0
#define ITER_TAIL 1

#define freeListNode(list, node)                 \
  do {                                           \
    if ((list)->free) (list)->free(node->value); \
  } while (0)
#define dupListNode(list, value) ((list->dup) ? list->dup(value) : value)
#define matchListNode(list, ptr, key) \
  ((list->match) ? list->match(ptr, key) : ptr == key)

List *newList(void *(*dup)(void *), void (*free)(void *),
              int (*match)(void *, void *));
void freeList(List *list);
void listAddNodeHead(List *list, void *value);
void listAddNodeTail(List *list, void *value);
void listJoin(List *l, List *o);

ListIter *listGetIterator(List *list, int direction);
ListNode *listNext(ListIter *iter);
void freeListIterator(ListIter *iter);

#endif  // LIST_H