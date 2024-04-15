#ifndef DATA_H
#define DATA_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*-------------------lexical analysis and syntax analysis-------------------*/
// the type of a node
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
} Node_type;

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

// the value of a node
typedef union {
  int val_int;
  float val_float;
  char* val_str;
} Val;

#define VAL_EMPTY \
  (Val) { .val_str = NULL }
#define VAL_INT(x) \
  (Val) { .val_int = atoi(x) }
#define VAL_FLOAT(x) \
  (Val) { .val_float = atof(x) }

// the data structure of a node
typedef struct {
  Node_type type;
  Val value;
  unsigned int lineno;
} Data;

Val val_str(const char* s);

static inline int is_non_terminal(const Node_type type) {
  return type >= _Program && type < _Empty;
}

/*-----------------------------semantic analysis-----------------------------*/
typedef struct Type Type;
typedef struct FieldList FieldList;

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

// the list of fields in a structure or the list of parameters in a function
struct FieldList {
  char* name;
  Type* type;
  FieldList* next;
};

void freeType(Type* t);
void freeFieldList(FieldList* fl);

int typeEqual(Type* a, Type* b);
int fieldListEqual(FieldList* a, FieldList* b);

Type* newTypeBasic(int basic);
Type* newTypeStructure(char* name, FieldList* structure);
Type* newTypeArray(Type* element, int size);
Type* newTypeFunction(Type* returnType, FieldList* params);
FieldList* newFieldList(char* name, Type* type, FieldList* next);

Type* copyTypeBasic(Type* t);
Type* copyTypeStructure(Type* t);
Type* copyTypeArray(Type* t);
Type* copyTypeFunction(Type* t);
FieldList* copyFieldList(FieldList* fl);

// function pointers for copying types
static Type* (*copyType[])(Type*) = {copyTypeBasic, copyTypeArray,
                                     copyTypeStructure, copyTypeFunction};

// Define error types
typedef enum {
  UND_VAR = 1,          // Undefined Variable
  UND_FUNC,             // Undefined Function
  RED_VAR,              // Redefined Variable
  RED_FUNC,             // Redefined Function
  TYP_MIS_ASS,          // Type Mismatch in Assignment
  LVAL_REQ,             // Lvalue Required
  OP_MIS,               // Operand Mismatch
  RET_TYP_MIS,          // Return Type Mismatch
  PAR_MIS,              // Parameter Mismatch
  NON_ARR_SUB,          // Non-Array Subscript
  NON_FUNC_CALL,        // Non-Function Call
  NON_INT_SUB,          // Non-Integer Subscript
  NON_STR_MEM,          // Non-Structure Member
  UND_STR_MEM,          // Undefined Structure Member
  RED_STR_MEM_OR_INIT,  // Redefined Structure Member
  STR_NAME_CON,         // Name Conflict
  UND_STR               // Undefined Structure
} ErrorType;

// Error message array
static const char* error_msg[] = {
    NULL,  // Placeholder for index 0, no error message
    "Variable used without being defined.",
    "Function called without being defined.",
    "Variable redefined or has the same name as a previously defined "
    "structure.",
    "Function redefined (i.e., the same function name defined more than once).",
    "Type mismatch in assignment expression.",
    "Lvalue required as left operand of assignment.",
    "Operand type mismatch or operand type does not match the operator.",
    "Return statement type does not match the function's defined return type.",
    "Number or type of arguments does not match the function's parameters.",
    "Subscript operator used with a non-array variable.",
    "Function call operator used with a non-function variable.",
    "Non-integer subscript in array access operator.",
    "Member access operator used with a non-structure variable.",
    "Access to an undefined structure member.",
    "Structure member redefined within the same structure or initialized at "
    "definition.",
    "Structure name conflicts with a previously defined structure or variable "
    "name.",
    "Undefined structure used to define a variable."};

#endif  // DATA_H