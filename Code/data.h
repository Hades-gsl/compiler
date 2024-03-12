#ifndef DATA_H
#define DATA_H

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
  _Args
} Node_type;//* 枚举抽象语法树节点类型 *//

typedef union {
  int val_int;
  float val_float;
  char* val_str;
} Val; //* 联合类型 整、浮、字符串指针 *//

#define VAL_EMPTY \
  (Val) { .val_str = NULL }
#define VAL_INT(x) \
  (Val) { .val_int = atoi(x) }
#define VAL_FLOAT(x) \
  (Val) { .val_float = atof(x) }
#define VAL_STR(x) \
  (Val) { .val_string = (x) }  //* 宏用于创建Val类型的值 *//

typedef struct {
  Node_type type;
  Val value;
  unsigned int lineno;
} Date;//* AST_Node 节点词法单元名、属性值、行号 *//

#endif  // DATA_H