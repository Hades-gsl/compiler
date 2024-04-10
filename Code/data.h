#ifndef DATA_H
#define DATA_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*-------------------lexical analysis and syntax analysis-------------------*/
typedef enum {
  _INT,
  _FLOAT,
  _ID,
  _SEMI,
  _COMMA,
  _ASSIGNOP,
  _RELOP,
  _PLUS,
  _MINUS,
  _STAR,
  _DIV,
  _AND,
  _OR,
  _DOT,
  _NOT,
  _TYPE,
  _LP,
  _RP,
  _LB,
  _RB,
  _LC,
  _RC,
  _STRUCT,
  _RETURN,
  _IF,
  _ELSE,
  _WHILE,
  _Program,
  _ExtDefList,
  _ExtDef,
  _ExtDecList,
  _Specifier,
  _StructSpecifier,
  _OptTag,
  _Tag,
  _VarDec,
  _FunDec,
  _VarList,
  _ParamDec,
  _CompSt,
  _StmtList,
  _Stmt,
  _DefList,
  _Def,
  _DecList,
  _Dec,
  _Exp,
  _Args,
  _Empty,
} Node_type;  //* 枚举抽象语法树节点类型 *//

const static char* type_strs[] = {
    "INT",        "FLOAT",      "ID",
    "SEMI",       "COMMA",      "ASSIGNOP",
    "RELOP",      "PLUS",       "MINUS",
    "STAR",       "DIV",        "AND",
    "OR",         "DOT",        "NOT",
    "TYPE",       "LP",         "RP",
    "LB",         "RB",         "LC",
    "RC",         "STRUCT",     "RETURN",
    "IF",         "ELSE",       "WHILE",
    "Program",    "ExtDefList", "ExtDef",
    "ExtDecList", "Specifier",  "StructSpecifier",
    "OptTag",     "Tag",        "VarDec",
    "FunDec",     "VarList",    "ParamDec",
    "CompSt",     "StmtList",   "Stmt",
    "DefList",    "Def",        "DecList",
    "Dec",        "Exp",        "Args",
};

typedef union {
  int val_int;
  float val_float;
  char* val_str;
} Val;  //* 联合类型 整、浮、字符串指针 *//

#define VAL_EMPTY \
  (Val) { .val_str = NULL }
#define VAL_INT(x) \
  (Val) { .val_int = atoi(x) }
#define VAL_FLOAT(x) \
  (Val) { .val_float = atof(x) }

typedef struct {
  Node_type type;
  Val value;
  unsigned int lineno;
} Data;  //* AST_Node 节点词法单元名、属性值、行号 *//

static inline int is_non_terminal(const Node_type type) {
  return type >= _Program && type < _Empty;
}

static Val val_str(const char* s) {
  size_t len = strlen(s) + 1;  // +1 for the null terminator
  char* cp = malloc(len);      // Allocate memory for the string
  if (cp) {
    strncpy(cp, s, len);  // Copy the string, including the null terminator
    return (Val){.val_str = cp};
  } else {
    // Handle memory allocation failure
    assert(0);
    return (Val){.val_str = NULL};
  }
}

/*-----------------------------semantic analysis-----------------------------*/
typedef struct Type Type;
typedef struct FieldList FieldList;

static void freeType(Type* t);
static void freeFieldList(FieldList* fl);
static int typeEqual(Type* a, Type* b);
static int fieldListEqual(FieldList* a, FieldList* b);

// the type of a symbol
struct Type {
  enum { BASIC, ARRAY, STRUCTURE, FUNCTION } kind;
  union {
    enum { INT_, FLOAT_ } basic;

    struct {
      Type* element;
      int size;
    } array;

    struct {
      char* name;
      FieldList* structure;
    } structure;

    struct {
      Type* returnType;
      FieldList* params;
    } function;
  };
};

static inline Type* newTypeBasic(int basic) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = BASIC, .basic = basic};
  return t;
}

static inline Type* newTypeStructure(char* name, FieldList* structure) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = STRUCTURE,
              .structure.name = name,
              .structure.structure = structure};
  return t;
}

static inline Type* newTypeArray(Type* element, int size) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = ARRAY, .array.element = element, .array.size = size};
  return t;
}

static inline Type* newTypeFunction(Type* returnType, FieldList* params) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = FUNCTION,
              .function.returnType = returnType,
              .function.params = params};
  return t;
}

static int typeEqual(Type* a, Type* b) {
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
      case ARRAY:
        return a->array.size == b->array.size &&
               typeEqual(a->array.element, b->array.element);
      case STRUCTURE:
        return fieldListEqual(a->structure.structure, b->structure.structure);
      case FUNCTION:
        return typeEqual(a->function.returnType, b->function.returnType) &&
               fieldListEqual(a->function.params, b->function.params);
    }
  }
}

static void freeType(Type* t) {
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

// the list of fields in a structure or the list of parameters in a function
struct FieldList {
  char* name;
  Type* type;
  FieldList* next;
};

static inline FieldList* newFieldList(char* name, Type* type, FieldList* next) {
  FieldList* fl = malloc(sizeof(FieldList));
  *fl = (FieldList){.name = name, .type = type, .next = next};
  return fl;
}

static int fieldListEqual(FieldList* a, FieldList* b) {
  if (a == NULL && b == NULL) {
    return 1;
  } else if (a == NULL || b == NULL) {
    return 0;
  } else {
    return typeEqual(a->type, b->type) && fieldListEqual(a->next, b->next);
  }
}

static void freeFieldList(FieldList* fl) {
  if (fl) {
    free(fl->name);
    freeType(fl->type);
    freeFieldList(fl->next);
    free(fl);
  }
}

// Define error types
typedef enum {
  UND_VAR = 1,    // Undefined Variable
  UND_FUNC,       // Undefined Function
  RED_VAR,        // Redefined Variable
  RED_FUNC,       // Redefined Function
  TYP_MIS_ASS,    // Type Mismatch in Assignment
  LVAL_REQ,       // Lvalue Required
  OP_MIS,         // Operand Mismatch
  RET_TYP_MIS,    // Return Type Mismatch
  PAR_MIS,        // Parameter Mismatch
  NON_ARR_SUB,    // Non-Array Subscript
  NON_FUNC_CALL,  // Non-Function Call
  NON_INT_SUB,    // Non-Integer Subscript
  NON_STR_MEM,    // Non-Structure Member
  UND_STR_MEM,    // Undefined Structure Member
  RED_STR_MEM,    // Redefined Structure Member
  NAME_CON,       // Name Conflict
  UND_STR         // Undefined Structure
} ErrorType;

// Error message array
const char* error_msg[] = {NULL,  // Placeholder for index 0, no error message
                           "Undeclared variable.",
                           "Undefined function.",
                           "Redefinition of variable or struct name conflict.",
                           "Redefinition of function.",
                           "Type mismatch in assignment.",
                           "Lvalue required.",
                           "Operand type mismatch.",
                           "Return type mismatch.",
                           "Parameter mismatch.",
                           "Non-array subscript used.",
                           "Non-function call attempt.",
                           "Non-integer array subscript.",
                           "Non-structure member access.",
                           "Undefined structure member access.",
                           "Redefined structure member.",
                           "Name conflict with struct or variable.",
                           "Undefined structure usage."};

#endif  // DATA_H