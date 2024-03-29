#include <assert.h>
#include <stdio.h>

#include "hash.h"
#include "mbtree.h"
#include "syntax.tab.h"

#define FUNC_PTR_CAST(f) ((unsigned int (*)(const void*))f)

MBTreeNode* root = NULL;
MBTreeNode* empty = NULL;
int has_error = 0;
HashTable* ht = NULL;

extern int yyparse();
extern int yyrestart(FILE*);
extern int yydebug;

int keyCompare(void* privDataPtr, const void* a, const void* b) {
  return !strcmp(a, b);
}

void keyDestructor(void* privDataPtr, void* key) { free(key); }

void valDestructor(void* privDataPtr, void* val) { freeFieldList(val); }

void init() {
  empty = newMBTreeNode(VAL_EMPTY, _Empty, -1);
  assert(empty);

  HtType* type = malloc(sizeof(HtType));
  *type = (HtType){.hashFunction = FUNC_PTR_CAST(htGenHashFunction),
                   .keyDup = NULL,
                   .valDup = NULL,
                   .keyCompare = keyCompare,
                   .keyDestructor = keyDestructor,
                   .valDestructor = valDestructor};

  ht = htCreate(type, NULL);
  assert(ht);

  // yydebug = 1;
}

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

  if (!has_error) displayMBTreeNode(root, 0);

  return 0;
}
