#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "hash.h"
#include "mbtree.h"
#include "syntax.tab.h"

#define FUNC_PTR_CAST(f) ((unsigned int (*)(const void*))f)

MBTreeNode* root = NULL;
HashTable* ht = NULL;
int has_error = 0;
int translateEnabled = 1;

extern int yyparse();
extern int yyrestart(FILE*);
extern void semanticAnalysis(MBTreeNode* node);
extern List* IRGenerate(MBTreeNode* node);
extern void MIPS32Generate(List* irList, FILE* fout);
extern char* strdup(const char*);
extern int yydebug;

int keyCompare(void* privDataPtr, const void* a, const void* b) {
  return !strcmp(a, b);
}
void keyDestructor(void* privDataPtr, void* key) { free(key); }
void valDestructor(void* privDataPtr, void* val) { freeType(val); }
void* keyDup(void* privDataPtr, const void* key) { return strdup(key); }
void* valDup(void* privDataPtr, const void* val) {
  if (val == NULL) return NULL;
  Type* v = (Type*)val;
  return copyType[v->kind](v);
}

static void init() {
  HtType* type = malloc(sizeof(HtType));
  *type = (HtType){.hashFunction = FUNC_PTR_CAST(htGenHashFunction),
                   .keyDup = keyDup,
                   .valDup = valDup,
                   .keyCompare = keyCompare,
                   .keyDestructor = keyDestructor,
                   .valDestructor = valDestructor};

  ht = htCreate(type, NULL);
  assert(ht);

  // yydebug = 1;
}

int main(int argc, char** argv) {
  if (argc <= 1) return 1;

  init();

  FILE* f = fopen(argv[1], "r");
  if (!f) {
    perror(argv[1]);
    return 1;
  }

  FILE* fout = NULL;

  yyrestart(f);
  yyparse();

  fclose(f);

  if (!has_error) {
    // displayMBTreeNode(root, 0);
    semanticAnalysis(root);
    if (translateEnabled) {
      if (argc <= 2) {
        fout = stdout;
      } else {
        fout = fopen(argv[2], "w");
        if (!fout) {
          perror(argv[2]);
          return 1;
        }
      }

      List* ir = IRGenerate(root);

      if (argc > 3) {
        FILE* irout = fopen(argv[3], "w");
        displayIRCodeList(ir, irout);
        fclose(irout);
      }
      // displayIRCodeList(ir, fout);

      MIPS32Generate(ir, fout);

      if (argc > 2) fclose(fout);
    } else {
      fprintf(stderr,
              "Cannot translate: Code contains variables of multi-dimensional"
              "array type or parameters of array type.\n");
    }
  }

  return 0;
}
