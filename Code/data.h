#ifndef DATA_H
#define DATA_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "mbtree.h"

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

#define getMBTreeNodeType(node) (((Data*)node->data)->type)
#define getMBTreeNodeValue(node) (((Data*)node->data)->value)
#define getMBTreeNodeLineNo(node) (((Data*)node->data)->lineno)

Val val_str(const char* s);
MBTreeNode* newMBTreeNodeData(Val val, Node_type type, unsigned int lineno);
// print the tree
void displayMBTreeNode(const MBTreeNode* node, unsigned indent);

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
      // the size of the array in memory, in bytes
      size_t memSize;
    } array;

    struct {
      char* name;
      FieldList* structure;
      // the size of the structure in memory, in bytes
      size_t memSize;
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

/*--------------------------------ir generate--------------------------------*/

// basic type memory size of a type, in bytes
#define BASIC_MEM_SIZE 4

// get the size of a type in memory, in bytes
// if the memSize field is 0, calculate it and store it in the field
size_t getMemSize(Type* t);

typedef struct Operand {
  enum {
    OP_VARIABLE,  // variable
    OP_CONSTANT,  // constant
    OP_ADDRESS,   // address
    OP_TEMP,      // temporary variable
    OP_LABEL,     // label
    OP_FUNCTION   // function
  } kind;
  union {
    char* var_name;
    intptr_t constant;
    char* base_name;
    size_t temp_no;
    size_t label_no;
    char* func_name;
  };
  Type* type;
} Operand;

Operand* newOperand(int kind, void* val, Type* type);
void operandTmp2Addr(Operand* op);
char* operand2str(Operand* op);

typedef struct IRCode {
  enum {
    IR_LABEL,      // label l :
    IR_FUNCTION,   // function f :
    IR_ASSIGN,     // x := y
    IR_ADD,        // x := y + z
    IR_SUB,        // x := y - z
    IR_MUL,        // x := y * z
    IR_DIV,        // x := y / z
    IR_GET_ADDR,   // x := &y
    IR_GET_VALUE,  // x := *y
    IR_SET_VALUE,  // *x := y
    IR_GOTO,       // goto l
    IR_IF_GOTO,    // if x [relop] y goto l
    IR_RETURN,     // return x
    IR_DEC,        // dec x [size]
    IR_ARG,        // arg x
    IR_CALL,       // x := call f
    IR_PARAM,      // param x
    IR_READ,       // read x
    IR_WRITE       // write x
  } kind;
  union {
    struct {
      Operand* op;
    };  // IR_LABEL, IR_FUNCTION, IR_GOTO, IR_RETURN, IR_ARG, IR_PARAM, IR_READ,
        // IR_WRITE
    struct {
      Operand* left;
      Operand* right;
    };  // IR_ASSIGN, IR_GET_ADDR, IR_GET_VALUE, IR_SET_VALUE, IR_CALL
    struct {
      Operand* result;
      Operand* op1;
      Operand* op2;
    };  // IR_ADD, IR_SUB, IR_MUL, IR_DIV
    struct {
      Operand* operand;
      int size;
    };  // IR_DEC
    struct {
      Operand* op_l;
      char* relop;
      Operand* op_r;
      Operand* label;
    };  // IR_IF_GOTO
  };
} IRCode;

IRCode* newIRCode(int kind, ...);
void printIRCode(FILE* fout, IRCode* ir);
void displayIRCodeList(List* ir, FILE* out);

/*------------------------------mips32 generate------------------------------*/

typedef struct Variable {
  Operand* op;
  int offset;
  int reg;
} Variable;

Variable* newVariable(Operand* op, int offset, int reg);

typedef struct Register {
  enum {
    REG_ZERO,  // zero
    REG_AT,    // at
    REG_V0,    // v0
    REG_V1,    // v1
    REG_A0,    // a0
    REG_A1,    // a1
    REG_A2,    // a2
    REG_A3,    // a3
    REG_T0,    // t0
    REG_T1,    // t1
    REG_T2,    // t2
    REG_T3,    // t3
    REG_T4,    // t4
    REG_T5,    // t5
    REG_T6,    // t6
    REG_T7,    // t7
    REG_S0,    // s0
    REG_S1,    // s1
    REG_S2,    // s2
    REG_S3,    // s3
    REG_S4,    // s4
    REG_S5,    // s5
    REG_S6,    // s6
    REG_S7,    // s7
    REG_T8,    // t8
    REG_T9,    // t9
    REG_K0,    // k0
    REG_K1,    // k1
    REG_GP,    // gp
    REG_SP,    // sp
    REG_FP,    // fp
    REG_RA     // ra
  } kind;
  int used;
  Variable* var;
} Register;

static const char* register_names[] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
    "$t0",   "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
    "$s0",   "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
    "$t8",   "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"};

#endif  // DATA_H