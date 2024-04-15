#include "data.h"

#include <assert.h>
#include <string.h>

extern char* strdup(const char*);

/*-------------------lexical analysis and syntax analysis-------------------*/
Val val_str(const char* s) { return (Val){.val_str = strdup(s)}; }

/*-----------------------------semantic analysis-----------------------------*/
Type* newTypeBasic(int basic) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = BASIC, .basic = basic};
  return t;
}

Type* newTypeStructure(char* name, FieldList* structure) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = STRUCTURE,
              .structure.name = strdup(name),
              .structure.structure = copyFieldList(structure)};
  return t;
}

Type* newTypeArray(Type* element, int size) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = ARRAY, .array.size = size, .array.element = NULL};
  if (element) {
    t->array.element = copyType[element->kind](element);
  }
  return t;
}

Type* newTypeFunction(Type* returnType, FieldList* params) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = FUNCTION,
              .function.params = copyFieldList(params),
              .function.returnType = NULL};
  if (returnType) {
    t->function.returnType = copyType[returnType->kind](returnType);
  }
  return t;
}

int typeEqual(Type* a, Type* b) {
  /*If either 'a' or 'b' is NULL, it indicates an error such as an undefined
   * struct 'a'. To prevent cascading errors, we treat the comparison as equal
   * and return 1.*/
  if (a == NULL || b == NULL) {
    return 1;
  } else if (a->kind != b->kind) {
    return 0;
  } else {
    switch (a->kind) {
      case BASIC:
        return a->basic == b->basic;
        // we don't need to compare the size of the array
      case ARRAY:
        return typeEqual(a->array.element, b->array.element);
        // we don't need to compare the name of the structure
      case STRUCTURE:
        return fieldListEqual(a->structure.structure, b->structure.structure);
      case FUNCTION:
        return typeEqual(a->function.returnType, b->function.returnType) &&
               fieldListEqual(a->function.params, b->function.params);
    }
  }
}

void freeType(Type* t) {
  if (t) {
    if (t->kind == ARRAY) {
      freeType(t->array.element);
    } else if (t->kind == STRUCTURE) {
      free(t->structure.name);
      freeFieldList(t->structure.structure);
    } else if (t->kind == FUNCTION) {
      freeType(t->function.returnType);
      freeFieldList(t->function.params);
    }

    free(t);
  }
}

FieldList* newFieldList(char* name, Type* type, FieldList* next) {
  FieldList* fl = malloc(sizeof(FieldList));
  *fl = (FieldList){
      .name = strdup(name), .next = copyFieldList(next), .type = NULL};
  if (type) {
    fl->type = copyType[type->kind](type);
  }
  return fl;
}

int fieldListEqual(FieldList* a, FieldList* b) {
  if (a == NULL && b == NULL) {
    return 1;
  } else if (a == NULL || b == NULL) {
    return 0;
  } else {
    return typeEqual(a->type, b->type) && fieldListEqual(a->next, b->next);
  }
}

void freeFieldList(FieldList* fl) {
  if (fl) {
    free(fl->name);
    freeType(fl->type);
    freeFieldList(fl->next);
    free(fl);
  }
}

Type* copyTypeBasic(Type* t) {
  if (t == NULL) {
    return NULL;
  }

  assert(t->kind == BASIC);

  Type* newType = malloc(sizeof(Type));
  *newType = (Type){.kind = BASIC, .basic = t->basic};

  return newType;
}

Type* copyTypeStructure(Type* t) {
  if (t == NULL) {
    return NULL;
  }

  assert(t->kind == STRUCTURE);

  Type* newType = malloc(sizeof(Type));
  *newType =
      (Type){.kind = STRUCTURE,
             .structure.name = strdup(t->structure.name),
             .structure.structure = copyFieldList(t->structure.structure)};

  return newType;
}

Type* copyTypeArray(Type* t) {
  if (t == NULL) {
    return NULL;
  }

  assert(t->kind == ARRAY);

  Type* newType = malloc(sizeof(Type));
  *newType =
      (Type){.kind = ARRAY, .array.size = t->array.size, .array.element = NULL};
  if (t->array.element) {
    newType->array.element = copyType[t->array.element->kind](t->array.element);
  }

  return newType;
}

Type* copyTypeFunction(Type* t) {
  if (t == NULL) {
    return NULL;
  }

  assert(t->kind == FUNCTION);

  Type* newType = malloc(sizeof(Type));
  *newType = (Type){.kind = FUNCTION,
                    .function.params = copyFieldList(t->function.params),
                    .function.returnType = NULL};
  if (t->function.returnType) {
    newType->function.returnType =
        copyType[t->function.returnType->kind](t->function.returnType);
  }

  return newType;
}

FieldList* copyFieldList(FieldList* fl) {
  if (fl == NULL) {
    return NULL;
  }

  FieldList* newFieldList = malloc(sizeof(FieldList));
  *newFieldList = (FieldList){
      .name = strdup(fl->name), .next = copyFieldList(fl->next), .type = NULL};
  if (fl->type) {
    newFieldList->type = copyType[fl->type->kind](fl->type);
  }

  return newFieldList;
}
