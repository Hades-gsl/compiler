#include "mbtree.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

MBTreeNode* newMBTreeNode(void* data) {
  MBTreeNode* node = (MBTreeNode*)malloc(sizeof(MBTreeNode));
  *node = (MBTreeNode){.data = data, .firstChild = NULL, .nextSibling = NULL};
  return node;
}

void addMBTreeNode(MBTreeNode* parent, ...) {
  assert(parent != NULL);

  va_list children;

  va_start(children, parent);

  MBTreeNode* child;
  while ((child = va_arg(children, MBTreeNode*)) != NULL) {
    child->nextSibling = parent->firstChild;
    parent->firstChild = child;
  }

  va_end(children);
}

void removeMBTreeNode(MBTreeNode* parent, MBTreeNode* child) {
  if (parent == NULL || child == NULL) return;

  MBTreeNode* node = parent->firstChild;
  if (node == child) {
    parent->firstChild = child->nextSibling;
  } else {
    while (node->nextSibling != child) {
      node = node->nextSibling;
    }
    node->nextSibling = child->nextSibling;
  }

  freeMBTreeNode(child);
}

void freeMBTreeNode(MBTreeNode* node) {
  if (node == NULL) return;

  freeMBTreeNode(node->firstChild);
  freeMBTreeNode(node->nextSibling);
  free(node->data);
  free(node);
}
