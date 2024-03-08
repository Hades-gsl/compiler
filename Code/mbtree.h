#ifndef MBTREE_H
#define MBTREE_H

#include <stdlib.h>

#include "data.h"

typedef struct MBTreeNode {
  Date* data;
  struct MBTreeNode* firstChild;
  struct MBTreeNode* nextSibling;
} MBTreeNode;

// create a new node
MBTreeNode* newMBTreeNodeData(Date* Data);
MBTreeNode* newMBTreeNode(Val val, Node_type type, unsigned int lineno);

// add children to the parent node, the last argument must be NULL
// e.g. addMBTreeNode(parent, child1, child2, child3, NULL);
// the order of the children is from right to left and the first child will be
// the last child
void addMBTreeNode(MBTreeNode* parent, ...);

// remove a child from the parent node
void removeMBTreeNode(MBTreeNode* parent, MBTreeNode* child);

// will free the tree node recursively
void freeMBTreeNode(MBTreeNode* node);

// print the tree
void displayMBTreeNode(MBTreeNode* node);

#endif  // MBTREE_H