#include <stdio.h>

#include "hash.h"
#include "mbtree.h"

#define BASIC_TYPE_INT 0
#define BASIC_TYPE_FLOAT 1

#define IS_LVALUE 1
#define NOT_LVALUE 0

extern HashTable* ht;

static Type* retType = NULL;
static int structDep = 0;

// High-level Definitions
static void saExtDefList(MBTreeNode* node);
static void saExtDef(MBTreeNode* node);
static void saExtDecList(MBTreeNode* node, Type* type);
// Specifiers
static Type* saSpecifier(MBTreeNode* node);
static Type* saStructSpecifier(MBTreeNode* node);
static char* saTag(MBTreeNode* node);
static char* saOptTag(MBTreeNode* node);
// Local Definitions
static FieldList* saDefList(MBTreeNode* node);
static FieldList* saDef(MBTreeNode* node);
static FieldList* saDecList(MBTreeNode* node, Type* type);
static FieldList* saDec(MBTreeNode* node, Type* type);
// Declarators
static FieldList* saVarDec(MBTreeNode* node, Type* type);
static void saFunDec(MBTreeNode* node, Type* type);
static FieldList* saVarList(MBTreeNode* node);
static FieldList* saParamDec(MBTreeNode* node);
// Statements
static void saCompSt(MBTreeNode* node);
static void saStmtList(MBTreeNode* node);
static void saStmt(MBTreeNode* node);
// Expressions
static Type* saExp(MBTreeNode* node, int isLvalue);
static FieldList* saArgs(MBTreeNode* node);
// Tokens
static Type* saTYPE(MBTreeNode* node);
static char* saID(MBTreeNode* node);
static int saINT(MBTreeNode* node);

static void print_error_massage(int code, int line);

// semantic analysis entry
void semanticAnalysis(MBTreeNode* node) {
  if (node == NULL) return;

  assert(node->data->type == _Program && node->nextSibling == NULL);

  // Program -> ExtDefList
  saExtDefList(node->firstChild);
}

// analyse the ExtDefList node
static void saExtDefList(MBTreeNode* node) {
  if (node == NULL) return;

  assert(node->data->type == _ExtDefList);

  MBTreeNode* child = node->firstChild;
  if (child->data->type == _Empty) {
    // ExtDefList -> empty
    return;
  }

  // ExtDefList -> ExtDef ExtDefList
  saExtDef(child);
  saExtDefList(child->nextSibling);
}

// analyse the ExtDef node
static void saExtDef(MBTreeNode* node) {
  if (node == NULL) return;

  assert(node->data->type == _ExtDef);

  MBTreeNode* specifier = node->firstChild;  // Specifier
  Type* type = saSpecifier(specifier);

  MBTreeNode* next = specifier->nextSibling;
  switch (next->data->type) {
    case _SEMI:
      // ExtDef -> Specifier SEMI
      break;
    case _FunDec:
      // ExtDef -> Specifier FunDec CompSt
      saFunDec(next, type);
      saCompSt(next->nextSibling);
      break;
    case _ExtDecList:
      // ExtDef -> Specifier ExtDecList SEMI
      saExtDecList(next, type);
      break;
    default:
      // error
      assert(0);
  }
}

// analyse the Specifier node
static Type* saSpecifier(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _Specifier);

  MBTreeNode* child = node->firstChild;
  Type* type = NULL;

  if (child->data->type == _TYPE) {
    // Specifier -> TYPE
    type = saTYPE(child);
  } else if (child->data->type == _StructSpecifier) {
    // Specifier -> StructSpecifier
    type = saStructSpecifier(child);
  } else {
    // error
    assert(0);
  }

  return type;
}

// analyse the TYPE node
static Type* saTYPE(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _TYPE);

  Type* type = NULL;

  // TYPE -> int | float
  if (!strcmp(node->data->value.val_str, "int")) {
    type = newTypeBasic(BASIC_TYPE_INT);
  } else if (!strcmp(node->data->value.val_str, "float")) {
    type = newTypeBasic(BASIC_TYPE_FLOAT);
  } else {
    // error
    assert(0);
  }

  return type;
}

// analyse the StructSpecifier node
static Type* saStructSpecifier(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _StructSpecifier);

  MBTreeNode* child = node->firstChild;
  assert(child->data->type == _STRUCT);

  Type* type = NULL;

  child = child->nextSibling;
  if (child->data->type == _OptTag) {
    // StructSpecifier -> STRUCT OptTag LC DefList RC
    char* tag = saOptTag(child);

    MBTreeNode* next = child->nextSibling;  // LC
    assert(next->data->type == _LC);
    structDep++;

    next = next->nextSibling;  // DefList
    FieldList* fl = saDefList(next);

    next = next->nextSibling;  // RC
    assert(next->data->type == _RC);
    structDep--;

    type = newTypeStructure(tag, fl);
    // if the tag is not empty, insert the structure type into the symbol table
    if (tag[0] != '\0') {
      if (htReplace(ht, tag, type) == 0) {
        print_error_massage(STR_NAME_CON, node->data->lineno);
      }
    }

  } else if (child->data->type == _Tag) {
    // StructSpecifier -> STRUCT Tag
    char* tag = saTag(child);
    HashEntry* he = htFind(ht, tag);
    if (he == NULL) {
      print_error_massage(UND_STR, node->data->lineno);
    } else {
      type = htGetEntryVal(he);
    }
  } else {
    // error
    assert(0);
  }

  return type;
}

// analyse the OptTag node
static char* saTag(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _Tag);

  // Tag -> ID
  return saID(node->firstChild);
}

// analyse the OptTag node
static char* saOptTag(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _OptTag);

  MBTreeNode* child = node->firstChild;
  if (child->data->type == _Empty) {
    // OptTag -> empty
    return "";
  }

  // OptTag -> ID
  return saID(child);
}

// analyse the ID node
static char* saID(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _ID);

  char* name = malloc(strlen(node->data->value.val_str) + 1);
  strcpy(name, node->data->value.val_str);

  return name;
}

// analyse the DefList node
static FieldList* saDefList(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _DefList);

  MBTreeNode* child = node->firstChild;
  if (child->data->type == _Empty) {
    // DefList -> empty
    return NULL;
  }

  // DefList -> Def DefList
  FieldList* def = saDef(child);
  FieldList* defList = saDefList(child->nextSibling);

  // concatenate the two linked lists
  FieldList* p = def;
  while (p->next != NULL) {
    p = p->next;
  }
  p->next = defList;

  return def;
}

// analyse the Def node
static FieldList* saDef(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _Def);

  // Def -> Specifier DecList SEMI
  MBTreeNode* specifier = node->firstChild;
  Type* type = saSpecifier(specifier);

  MBTreeNode* decList = specifier->nextSibling;

  assert(decList->nextSibling->data->type == _SEMI);

  return saDecList(decList, type);
}

// analyse the DecList node
static FieldList* saDecList(MBTreeNode* node, Type* type) {
  if (node == NULL) return NULL;

  assert(node->data->type == _DecList);

  MBTreeNode* child = node->firstChild;  // Dec
  FieldList* dec = saDec(child, type);

  MBTreeNode* next = child->nextSibling;
  if (next == NULL) {
    // DecList -> Dec
    return dec;
  }

  assert(next->data->type == _COMMA);

  // DecList -> Dec COMMA DecList
  FieldList* decList = saDecList(next->nextSibling, type);
  assert(dec->next == NULL);
  dec->next = decList;

  return dec;
}

// analyse the Dec node
static FieldList* saDec(MBTreeNode* node, Type* type) {
  if (node == NULL) return NULL;

  assert(node->data->type == _Dec);

  MBTreeNode* child = node->firstChild;  // VarDec
  FieldList* varDec = saVarDec(child, type);
  if (htReplace(ht, varDec->name, varDec->type) == 0) {
    if (structDep > 0) {
      print_error_massage(RED_STR_MEM_OR_INIT, node->data->lineno);
    } else {
      print_error_massage(RED_VAR, node->data->lineno);
    }
  }

  child = child->nextSibling;
  if (child == NULL) {
    // Dec -> VarDec
  } else if (child->data->type == _ASSIGNOP) {
    // Dec -> VarDec ASSIGNOP Exp
    if (structDep > 0) {
      print_error_massage(RED_STR_MEM_OR_INIT, node->data->lineno);
    }

    Type* t = saExp(child->nextSibling, NOT_LVALUE);
    if (!typeEqual(varDec->type, t)) {
      print_error_massage(TYP_MIS_ASS, node->data->lineno);
    }

  } else {
    // error
    assert(0);
  }
  return varDec;
}

// analyse the VarDec node
static FieldList* saVarDec(MBTreeNode* node, Type* type) {
  if (node == NULL) return NULL;

  assert(node->data->type == _VarDec);

  MBTreeNode* child = node->firstChild;
  FieldList* fl = NULL;
  if (child->data->type == _ID) {
    // VarDec -> ID
    char* name = saID(child);
    fl = newFieldList(name, type, NULL);
  } else {
    // VarDec -> VarDec LB INT RB
    fl = saVarDec(child, type);

    MBTreeNode* next = child->nextSibling;
    assert(next->data->type == _LB);

    next = next->nextSibling;
    int size = saINT(next);

    // Create a new array type and insert it into the type hierarchy. The new
    // array type is positioned as the second-to-last type before the basic
    // type.
    Type* array = newTypeArray(NULL, size);
    Type* p = fl->type;
    // If the current type is a basic type, the new array type becomes the
    // outermost type.
    if (p->kind == BASIC) {
      array->array.element = p;
      fl->type = array;
    } else {
      // Traverse the type hierarchy to find the type that is immediately before
      // the basic type. Insert the new array type as the element of this type,
      // making it the second-to-last type.
      while (p->array.element->kind != BASIC) {
        p = p->array.element;
      }
      array->array.element = p->array.element;
      p->array.element = array;
    }

    next = next->nextSibling;
    assert(next->data->type == _RB);
  }
  return fl;
}

// get the int value of the INT node
static int saINT(MBTreeNode* node) {
  if (node == NULL) return 0;

  assert(node->data->type == _INT);

  return node->data->value.val_int;
}

// analyse the FunDec node
static void saFunDec(MBTreeNode* node, Type* type) {
  if (node == NULL) return;

  assert(node->data->type == _FunDec);

  // record the return type of the function
  retType = type;

  MBTreeNode* child = node->firstChild;  // ID
  char* name = saID(child);

  child = child->nextSibling;  // LP
  assert(child->data->type == _LP);

  Type* function = newTypeFunction(type, NULL);

  child = child->nextSibling;
  if (child->data->type == _VarList) {
    // FunDec -> ID LP VarList RP
    type->function.params = saVarList(child);

    assert(child->nextSibling->data->type == _RP);
  } else if (child->data->type == _RP) {
    // FunDec -> ID LP RP
  } else {
    // error
    assert(0);
  }

  if (htReplace(ht, name, function) == 0) {
    print_error_massage(RED_FUNC, node->data->lineno);
  }
}

// analyse the VarList node
static FieldList* saVarList(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _VarList);

  MBTreeNode* child = node->firstChild;  // ParamDec
  FieldList* fl = saParamDec(child);

  MBTreeNode* next = child->nextSibling;
  if (next == NULL) {
    // VarList -> ParamDec
    return fl;
  }

  assert(next->data->type == _COMMA);

  // VarList -> ParamDec COMMA VarList
  assert(fl->next == NULL);
  fl->next = saVarList(next->nextSibling);

  return fl;
}

// analyse the ParamDec node
static FieldList* saParamDec(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _ParamDec);

  // ParamDec -> Specifier VarDec
  MBTreeNode* child = node->firstChild;
  Type* type = saSpecifier(child);

  child = child->nextSibling;
  FieldList* fl = saVarDec(child, type);

  if (htReplace(ht, fl->name, fl->type) == 0) {
    print_error_massage(RED_VAR, node->data->lineno);
  }

  return fl;
}

// analyse the CompSt node
static void saCompSt(MBTreeNode* node) {
  if (node == NULL) return;

  assert(node->data->type == _CompSt);

  MBTreeNode* child = node->firstChild;  // LC
  assert(child->data->type == _LC);

  // CompSt -> LC DefList StmtList RC
  child = child->nextSibling;  // DefList
  saDefList(child);

  child = child->nextSibling;  // StmtList
  saStmtList(child);

  child = child->nextSibling;  // RC
  assert(child->data->type == _RC);
}

// analyse the StmtList node
static void saStmtList(MBTreeNode* node) {
  if (node == NULL) return;

  assert(node->data->type == _StmtList);

  MBTreeNode* child = node->firstChild;
  if (child->data->type == _Empty) {
    // StmtList -> empty
    return;
  }

  // StmtList -> Stmt StmtList
  saStmt(child);
  saStmtList(child->nextSibling);
}

// analyse the Stmt node
static void saStmt(MBTreeNode* node) {
  if (node == NULL) return;

  assert(node->data->type == _Stmt);

  MBTreeNode* child = node->firstChild;
  switch (child->data->type) {
    case _Exp:
      // Stmt -> Exp SEMI
      saExp(child, NOT_LVALUE);
      assert(child->nextSibling->data->type == _SEMI);
      break;
    case _CompSt:
      // Stmt -> CompSt
      saCompSt(child);
      break;
    case _RETURN: {
      // Stmt -> RETURN Exp SEMI
      child = child->nextSibling;
      Type* t = saExp(child, NOT_LVALUE);
      if (!typeEqual(retType, t)) {
        print_error_massage(RET_TYP_MIS, node->data->lineno);
      }

      assert(child->nextSibling->data->type == _SEMI);
      break;
    }
    case _IF: {
      // Stmt -> IF LP Exp RP Stmt
      child = child->nextSibling;  // LP
      assert(child->data->type == _LP);

      child = child->nextSibling;  // Exp
      Type* t = saExp(child, NOT_LVALUE);
      if (t != NULL && (t->kind != BASIC || t->basic != BASIC_TYPE_INT)) {
        print_error_massage(OP_MIS, node->data->lineno);
      }

      child = child->nextSibling;  // RP
      assert(child->data->type == _RP);

      child = child->nextSibling;  // Stmt
      saStmt(child);

      child = child->nextSibling;
      if (child != NULL) {
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        assert(child->data->type == _ELSE);
        child = child->nextSibling;
        saStmt(child);
      }
      break;
    }
    case _WHILE: {
      // Stmt -> WHILE LP Exp RP Stmt
      child = child->nextSibling;  // LP
      assert(child->data->type == _LP);

      child = child->nextSibling;  // Exp
      Type* t = saExp(child, NOT_LVALUE);
      assert(t->kind == BASIC && t->basic == BASIC_TYPE_INT);

      child = child->nextSibling;  // RP
      assert(child->data->type == _RP);

      child = child->nextSibling;  // Stmt
      saStmt(child);
      break;
    }
    default:
      // error
      assert(0);
  }
}

// analyse the Exp node
static Type* saExp(MBTreeNode* node, int isLvalue) {
  if (node == NULL) return NULL;

  assert(node->data->type == _Exp);

  Type* type = NULL;

  MBTreeNode* child = node->firstChild;
  switch (child->data->type) {
    case _Exp:
      Type* t1 = saExp(child, NOT_LVALUE);
      child = child->nextSibling;
      if (child->data->type == _DOT) {
        // Exp -> Exp DOT ID
        if (t1 == NULL || t1->kind != STRUCTURE) {
          print_error_massage(NON_STR_MEM, node->data->lineno);
          break;
        }

        char* name = saID(child->nextSibling);
        FieldList* fl = t1->structure.structure;
        while (fl != NULL) {
          if (!strcmp(fl->name, name)) {
            type = fl->type;
            break;
          }
          fl = fl->next;
        }

        if (type == NULL) {
          print_error_massage(UND_STR_MEM, node->data->lineno);
        }

        goto ret;
      } else if (child->data->type == _LB) {
        // Exp -> Exp LB Exp RB
        Type* t2 = saExp(child->nextSibling, NOT_LVALUE);
        if (t1 == NULL || t1->kind != ARRAY) {
          print_error_massage(NON_ARR_SUB, node->data->lineno);
          break;
        }
        if (t2 == NULL || t2->kind != BASIC || t2->basic != BASIC_TYPE_INT) {
          print_error_massage(NON_INT_SUB, node->data->lineno);
          break;
        }
        type = t1->array.element;

        goto ret;
      } else {
        // Exp -> Exp ASSIGNOP Exp
        // Exp -> Exp AND Exp
        // Exp -> Exp OR Exp
        // Exp -> Exp RELOP Exp
        // Exp -> Exp PLUS Exp
        // Exp -> Exp MINUS Exp
        // Exp -> Exp STAR Exp
        // Exp -> Exp DIV Exp
        Type* t2 = saExp(child->nextSibling, NOT_LVALUE);
        if (child->data->type == _ASSIGNOP) {
          t1 = saExp(node->firstChild, IS_LVALUE);

          if (!typeEqual(t1, t2)) {
            print_error_massage(TYP_MIS_ASS, node->data->lineno);
          }
        } else {
          if (!typeEqual(t1, t2)) {
            print_error_massage(OP_MIS, node->data->lineno);
          }
        }

        type = t1;
      }

      break;
    case _LP:
      // Exp -> LP Exp RP
      child = child->nextSibling;
      type = saExp(child, NOT_LVALUE);

      assert(child->nextSibling->data->type == _RP);
      break;
    case _MINUS:
      // Exp -> MINUS Exp
      type = saExp(child->nextSibling, NOT_LVALUE);
      if (type == NULL || type->kind != BASIC) {
        print_error_massage(OP_MIS, node->data->lineno);
      }
      break;
    case _NOT:
      // Exp -> NOT Exp
      type = saExp(child->nextSibling, NOT_LVALUE);
      if (type == NULL || type->kind != BASIC ||
          type->basic != BASIC_TYPE_INT) {
        print_error_massage(OP_MIS, node->data->lineno);
      }
      break;
    case _ID:
      // Exp -> ID LP Args RP
      // Exp -> ID LP RP
      // Exp -> ID
      char* name = saID(child);

      child = child->nextSibling;
      if (child != NULL) {
        HashEntry* he = htFind(ht, name);
        if (he == NULL) {
          print_error_massage(UND_FUNC, node->data->lineno);
          break;
        }

        Type* t = htGetEntryVal(he);

        assert(child->data->type == _LP);
        child = child->nextSibling;
        if (child->data->type == _Args) {
          // Exp -> ID LP Args RP
          FieldList* fl = saArgs(child);

          if (t == NULL || t->kind != FUNCTION) {
            print_error_massage(NON_FUNC_CALL, node->data->lineno);
            break;
          }

          if (!fieldListEqual(t->function.params, fl)) {
            print_error_massage(PAR_MIS, node->data->lineno);
          }

          assert(child->nextSibling->data->type == _RP);
        } else if (child->data->type == _RP) {
          // Exp -> ID LP RP
          if (t->function.params != NULL) {
            print_error_massage(PAR_MIS, node->data->lineno);
          }
        } else {
          // error
          assert(0);
        }
      } else {
        HashEntry* he = htFind(ht, name);
        if (he == NULL) {
          print_error_massage(UND_VAR, node->data->lineno);
        } else {
          type = htGetEntryVal(he);
        }

        goto ret;
      }
      break;
    case _INT:
      // Exp -> INT
      type = newTypeBasic(BASIC_TYPE_INT);
      break;
    case _FLOAT:
      // Exp -> FLOAT
      type = newTypeBasic(BASIC_TYPE_FLOAT);
      break;
    default:
      // error
      assert(0);
  }

  if (isLvalue) {
    print_error_massage(LVAL_REQ, node->data->lineno);
  }

ret:
  return type;
}

// analyse the Args node
static FieldList* saArgs(MBTreeNode* node) {
  if (node == NULL) return NULL;

  assert(node->data->type == _Args);

  MBTreeNode* child = node->firstChild;
  FieldList* fl = newFieldList("", saExp(child, NOT_LVALUE), NULL);

  MBTreeNode* next = child->nextSibling;
  if (next == NULL) {
    // Args -> Exp
    return fl;
  }

  assert(next->data->type == _COMMA);

  // Args -> Exp COMMA Args
  assert(fl->next == NULL);
  fl->next = saArgs(next->nextSibling);

  return fl;
}

// analyse the ExtDecList node
static void saExtDecList(MBTreeNode* node, Type* type) {
  if (node == NULL) return;

  assert(node->data->type == _ExtDecList);

  MBTreeNode* child = node->firstChild;  // VarDec
  saVarDec(child, type);

  MBTreeNode* next = child->nextSibling;
  if (next == NULL) {
    // ExtDecList -> VarDec
  } else if (next->data->type == _COMMA) {
    // ExtDecList -> VarDec COMMA ExtDecList
    saExtDecList(next->nextSibling, type);
  } else {
    // error
    assert(0);
  }
}

static void print_error_massage(int code, int line) {
  printf("Error type %d at Line %d: %s\n", code, line, error_msg[code]);
}
