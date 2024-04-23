#ifndef MBTREE_H
#define MBTREE_H

#include <stdlib.h>

#include "data.h"

typedef struct MBTreeNode {
  Data* data;
  struct MBTreeNode* firstChild;
  struct MBTreeNode* nextSibling;
} MBTreeNode;

#define getMBTreeNodeData(node) (node->data)
#define getMBTreeNodeFirstChild(node) (node->firstChild)
#define getMBTreeNodeNextSibling(node) (node->nextSibling)
#define getMBTreeNodeType(node) (node->data->type)
#define getMBTreeNodeVal(node) (node->data->val)
#define getMBTreeNodeLineNo(node) (node->data->lineno)

// create a new node
MBTreeNode* newMBTreeNodeData(Data* Data);
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
void displayMBTreeNode(const MBTreeNode* node, unsigned indent);

#endif  // MBTREE_H