#include <assert.h>
#include <stdio.h>

#include "mbtree.h"
#include "syntax.tab.h"

MBTreeNode* root = NULL;

extern int yyparse();
extern int yyrestart(FILE*);
extern void yyerror(const char* s);

void yyerror(const char* s) { fprintf(stderr, "Syntax error: %s\n", s); }

int main(int argc, char** argv) {
  if (argc <= 1) return 1;
  FILE* f = fopen(argv[1], "r");
  if (!f) {
    perror(argv[1]);
    return 1;
  }

  yyrestart(f);
  yyparse();

  fclose(f);

  return 0;
}
