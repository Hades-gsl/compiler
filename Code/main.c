#include <assert.h>
#include <stdio.h>

#include "mbtree.h"
#include "syntax.tab.h"

MBTreeNode* root = NULL;
MBTreeNode* empty;
int has_error = 0;

extern int yyparse();
extern int yyrestart(FILE*);
extern int yydebug;

int main(int argc, char** argv) {
  empty = newMBTreeNode(VAL_EMPTY, _Empty, -1);
  // yydebug = 1;

  if (argc <= 1) return 1;
  FILE* f = fopen(argv[1], "r");
  if (!f) {
    perror(argv[1]);
    return 1;
  }

  yyrestart(f);
  yyparse();

  fclose(f);

  if (!has_error) displayMBTreeNode(root, 0);

  return 0;
}
