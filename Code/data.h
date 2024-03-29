#ifndef DATA_H
#define DATA_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct Type Type;
typedef struct FieldList FieldList;

struct Type {
  enum { BASIC, ARRAY, STRUCTURE } kind;
  union {
    enum { INT_, FLOAT_ } basic;
    struct {
      Type* element;
      int size;
    } array;
    FieldList* structure;
  };
};

// the list of fields in a structure
struct FieldList {
  char* name;
  Type* type;
  FieldList* next;
};

static void freeFieldList(FieldList* fl) {
  if (fl) {
    free(fl->name);
    free(fl->type);
    freeFieldList(fl->next);
    free(fl);
  }
}

#endif  // DATA_H