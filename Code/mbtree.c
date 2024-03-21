#include "mbtree.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

MBTreeNode* newMBTreeNodeData(Data* data) {
  MBTreeNode* node = (MBTreeNode*)malloc(sizeof(MBTreeNode));
  *node = (MBTreeNode){.data = data, .firstChild = NULL, .nextSibling = NULL};
  return node;
}

MBTreeNode* newMBTreeNode(Val val, Node_type type, unsigned int lineno) {
  Data* data = (Data*)malloc(sizeof(Data));
  *data = (Data){.type = type, .value = val, .lineno = lineno};
  return newMBTreeNodeData(data);
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

void displayMBTreeNode(const MBTreeNode* node, unsigned indent) {
  if (node == NULL || node->data->type == _Empty) return;
  if (node->firstChild != NULL && node->firstChild->data->type == _Empty)
    return;

  for (int i = 0; i < indent; i++) {
    printf(" ");
  }
  if (is_non_terminal(node->data->type)) {
    printf("%s (%u)\n", type_strs[node->data->type], node->data->lineno);
  } else {
    printf("%s", type_strs[node->data->type]);
    switch (node->data->type) {
      case _ID:
      case _TYPE:
        printf(": %s\n", node->data->value.val_str);
        break;
      case_INT:
        printf(": %d\n", node->data->value.val_int);
        break;
      case _FLOAT:
        printf(": %f\n", node->data->value.val_float);
        break;
      default:
        printf("\n");
        break;
    }
  }

  for (MBTreeNode* child = node->firstChild; child != NULL;
       child = child->nextSibling) {
    displayMBTreeNode(child, indent + 2);
  }
}
