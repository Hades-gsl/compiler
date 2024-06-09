#ifndef MBTREE_H
#define MBTREE_H

#include <stdlib.h>

typedef struct MBTreeNode {
  void* data;
  struct MBTreeNode* firstChild;
  struct MBTreeNode* nextSibling;
} MBTreeNode;

#define getMBTreeNodeData(node) (node->data)
#define getMBTreeNodeFirstChild(node) (node->firstChild)
#define getMBTreeNodeNextSibling(node) (node->nextSibling)

// create a new node
MBTreeNode* newMBTreeNode(void* data);

// add children to the parent node, the last argument must be NULL
// e.g. addMBTreeNode(parent, child1, child2, child3, NULL);
// the order of the children is from right to left and the first child will be
// the last child
void addMBTreeNode(MBTreeNode* parent, ...);

// remove a child from the parent node
void removeMBTreeNode(MBTreeNode* parent, MBTreeNode* child);

// will free the tree node recursively
void freeMBTreeNode(MBTreeNode* node);

#endif  // MBTREE_H