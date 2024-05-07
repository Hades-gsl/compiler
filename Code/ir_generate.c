#include <stdio.h>

#include "hash.h"
#include "list.h"
#include "mbtree.h"

extern HashTable* ht;

static size_t label_count = 0;
static size_t temp_count = 0;

#define getLabelNo (void*)(++label_count)
#define getTempNo (void*)(++temp_count)

static List* translateExtDefList(MBTreeNode* node);
static List* translateExtDef(MBTreeNode* node);
static List* translateExtDecList(MBTreeNode* node);
static List* translateVarDec(MBTreeNode* node, Operand* place);
static List* translateCompSt(MBTreeNode* node);
static List* translateDefList(MBTreeNode* node);
static List* translateDef(MBTreeNode* node);
static List* translateDecList(MBTreeNode* node);
static List* translateDec(MBTreeNode* node);
static List* translateStmtList(MBTreeNode* node);

static List* translateFunDec(MBTreeNode* node);
static List* translateArgs(MBTreeNode* node, List* argList);
static List* translateStmt(MBTreeNode* node);
static List* translateCond(MBTreeNode* node, Operand* label_true,
                           Operand* label_false);
static List* translateExp(MBTreeNode* node, Operand* place);

static char* getID(MBTreeNode* node);
static int getINT(MBTreeNode* node);

static void displayIRCodeList(List* ir, FILE* out);

// entry point for the IR generation
void IRGenerate(MBTreeNode* node, FILE* fout) {
  if (node == NULL) {
    return;
  }

  assert(getMBTreeNodeType(node) == _Program);

  // Program -> ExtDefList
  List* ir = translateExtDefList(getMBTreeNodeFirstChild(node));

  displayIRCodeList(ir, fout);
}

// process ExtDefList node
static List* translateExtDefList(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _ExtDefList);
  List* ir = NULL;

  MBTreeNode* child = getMBTreeNodeFirstChild(node);
  if (getMBTreeNodeType(child) == _Empty) {
    // ExtDefList -> Empty
    ir = newList(NULL, NULL, NULL);
  } else {
    // ExtDefList -> ExtDef ExtDefList
    ir = translateExtDef(child);
    List* ir2 = translateExtDefList(getMBTreeNodeNextSibling(child));
    listJoin(ir, ir2);
    freeList(ir2);
  }
  return ir;
}

// process ExtDef node
static List* translateExtDef(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _ExtDef);
  List* ir = NULL;

  MBTreeNode* child = getMBTreeNodeFirstChild(node);  // Specifier
  assert(getMBTreeNodeType(child) == _Specifier);

  child = getMBTreeNodeNextSibling(child);
  if (getMBTreeNodeType(child) == _ExtDecList) {
    // ExtDef -> Specifier ExtDecList SEMI
    ir = translateExtDecList(child);
  } else if (getMBTreeNodeType(child) == _SEMI) {
    // ExtDef -> Specifier SEMI
    // do nothing
  } else if (getMBTreeNodeType(child) == _FunDec) {
    // ExtDef -> Specifier FunDec CompSt
    ir = translateFunDec(child);
    List* ir2 = translateCompSt(getMBTreeNodeNextSibling(child));
    listJoin(ir, ir2);
    freeList(ir2);
  } else {
    // error
    assert(0);
  }

  return ir;
}

// process ExtDecList node
static List* translateExtDecList(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _ExtDecList);
  List* ir = NULL;

  MBTreeNode* child = getMBTreeNodeFirstChild(node);  // VarDec
  Operand* place = newOperand(OP_TEMP, getTempNo, NULL);
  ir = translateVarDec(child, place);

  child = getMBTreeNodeNextSibling(child);
  if (child == NULL) {
    // ExtDecList -> VarDec
    // do nothing
  } else {
    // ExtDecList -> VarDec COMMA ExtDecList
    List* ir2 = translateExtDecList(getMBTreeNodeNextSibling(child));
    listJoin(ir, ir2);
    freeList(ir2);
  }

  return ir;
}

// process VarDec node
static List* translateVarDec(MBTreeNode* node, Operand* place) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _VarDec);
  List* ir = newList(NULL, NULL, NULL);

  MBTreeNode* child = getMBTreeNodeFirstChild(node);
  if (getMBTreeNodeType(child) == _ID) {
    // VarDec -> ID
    char* id = getID(child);
    Type* t = htFind(ht, id)->val;
    assert(t != NULL);

    place->kind = OP_VARIABLE;
    place->var_name = id;
    place->type = t;

    if (t->kind == ARRAY) {
      listAddNodeTail(ir, newIRCode(IR_DEC, place, t->array.memSize));
    } else if (t->kind == STRUCTURE) {
      listAddNodeTail(ir, newIRCode(IR_DEC, place, t->structure.memSize));
    }
  } else if (getMBTreeNodeType(child) == _VarDec) {
    // VarDec -> VarDec LB INT RB
    freeList(ir);
    ir = translateVarDec(child, place);
  } else {
    // error
    assert(0);
  }

  return ir;
}

// process ID node
static char* getID(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _ID);

  return getMBTreeNodeValue(node).val_str;
}

// process INT node
static int getINT(MBTreeNode* node) {
  if (node == NULL) {
    return 0;
  }

  assert(getMBTreeNodeType(node) == _INT);

  return getMBTreeNodeValue(node).val_int;
}

// process FunDec node
static List* translateFunDec(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _FunDec);
  List* ir = newList(NULL, NULL, NULL);

  // FunDec -> ID LP VarList RP
  MBTreeNode* child = getMBTreeNodeFirstChild(node);  // ID
  char* id = getID(child);

  Type* t = htFind(ht, id)->val;
  assert(t != NULL);

  Operand* op = newOperand(OP_FUNCTION, id, t);
  listAddNodeTail(ir, newIRCode(IR_FUNCTION, op));

  for (FieldList* fl = t->function.params; fl; fl = fl->next) {
    Operand* op = newOperand(OP_VARIABLE, fl->name, fl->type);
    listAddNodeTail(ir, newIRCode(IR_PARAM, op));
  }

  return ir;
}

// process CompSt node
static List* translateCompSt(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _CompSt);
  List* ir = NULL;

  // CompSt -> LC DefList StmtList RC
  MBTreeNode* child = getMBTreeNodeFirstChild(node);  // LC
  assert(getMBTreeNodeType(child) == _LC);

  child = getMBTreeNodeNextSibling(child);  // DefList
  ir = translateDefList(child);

  child = getMBTreeNodeNextSibling(child);  // StmtList
  List* ir2 = translateStmtList(child);
  listJoin(ir, ir2);
  freeList(ir2);

  return ir;
}

// process DefList node
static List* translateDefList(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _DefList);
  List* ir = NULL;

  MBTreeNode* child = getMBTreeNodeFirstChild(node);
  if (getMBTreeNodeType(child) == _Def) {
    // DefList -> Def DefList
    ir = translateDef(child);
    List* ir2 = translateDefList(getMBTreeNodeNextSibling(child));
    listJoin(ir, ir2);
    freeList(ir2);
  } else if (getMBTreeNodeType(child) == _Empty) {
    // DefList -> Empty
    ir = newList(NULL, NULL, NULL);
  } else {
    // error
    assert(0);
  }

  return ir;
}

// process Def node
static List* translateDef(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _Def);
  List* ir = NULL;

  // Def -> Specifier DecList SEMI
  MBTreeNode* child = getMBTreeNodeFirstChild(node);  // Specifier
  child = getMBTreeNodeNextSibling(child);            // DecList
  ir = translateDecList(child);

  return ir;
}

// process DecList node
static List* translateDecList(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _DecList);
  List* ir = NULL;

  // DecList -> Dec
  MBTreeNode* child = getMBTreeNodeFirstChild(node);  // Dec
  ir = translateDec(child);

  child = getMBTreeNodeNextSibling(child);
  if (child != NULL) {
    // DecList -> Dec COMMA DecList
    assert(getMBTreeNodeType(child) == _COMMA);
    List* ir2 = translateDecList(getMBTreeNodeNextSibling(child));
    listJoin(ir, ir2);
    freeList(ir2);
  }

  return ir;
}

// process Dec node
static List* translateDec(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _Dec);
  List* ir = NULL;

  MBTreeNode* child = getMBTreeNodeFirstChild(node);  // VarDec
  Operand* place = newOperand(OP_TEMP, getTempNo, NULL);
  ir = translateVarDec(child, place);

  child = getMBTreeNodeNextSibling(child);
  if (child == NULL) {
    // Dec -> VarDec
    ir = newList(NULL, NULL, NULL);
  } else {
    // Dec -> VarDec ASSIGNOP Exp
    assert(getMBTreeNodeType(child) == _ASSIGNOP);
    Operand* tmp = newOperand(OP_TEMP, getTempNo, NULL);
    List* ir2 = translateExp(getMBTreeNodeNextSibling(child), tmp);
    listJoin(ir, ir2);
    freeList(ir2);

    listAddNodeTail(ir, newIRCode(IR_ASSIGN, place, tmp));
  }

  return ir;
}

// process Exp node
static List* translateExp(MBTreeNode* node, Operand* place) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _Exp);
  List* ir = NULL;

  MBTreeNode* child = getMBTreeNodeFirstChild(node);
  switch (getMBTreeNodeType(child)) {
    case _Exp: {
      MBTreeNode* child2 = getMBTreeNodeNextSibling(child);
      switch
        getMBTreeNodeType(child2) {
          case _ASSIGNOP: {
            // Exp -> Exp1 ASSIGNOP Exp2
            Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
            ir = translateExp(getMBTreeNodeNextSibling(child2), t1);
            List* ir2 = translateExp(child, place);
            listJoin(ir, ir2);
            freeList(ir2);

            if (place->kind != OP_ADDRESS && t1->kind != OP_ADDRESS) {
              listAddNodeTail(ir, newIRCode(IR_ASSIGN, place, t1));
            } else if (place->kind == OP_ADDRESS) {
              listAddNodeTail(ir, newIRCode(IR_SET_VALUE, place, t1));
            } else if (t1->kind == OP_ADDRESS) {
              listAddNodeTail(ir, newIRCode(IR_GET_VALUE, place, t1));
            } else {
              // error
              assert(0);
            }
            break;
          }
          case _AND:
          case _OR:
          case _RELOP: {
            // Exp -> Exp AND Exp
            // Exp -> Exp OR Exp
            // Exp -> Exp RELOP Exp
            Operand* label_true = newOperand(OP_LABEL, getLabelNo, NULL);
            Operand* label_false = newOperand(OP_LABEL, getLabelNo, NULL);
            ir = newList(NULL, NULL, NULL);
            listAddNodeTail(ir, newIRCode(IR_ASSIGN, place,
                                          newOperand(OP_CONSTANT, 0, NULL)));
            List* ir2 = translateCond(node, label_true, label_false);
            listJoin(ir, ir2);
            freeList(ir2);
            listAddNodeTail(ir, newIRCode(IR_LABEL, label_true));
            listAddNodeTail(ir,
                            newIRCode(IR_ASSIGN, place,
                                      newOperand(OP_CONSTANT, (void*)1, NULL)));
            listAddNodeTail(ir, newIRCode(IR_LABEL, label_false));
            break;
          }
          case _PLUS:
          case _MINUS:
          case _STAR:
          case _DIV: {
            // Exp -> Exp PLUS Exp
            int kinds[] = {IR_ADD, IR_SUB, IR_MUL, IR_DIV};
            Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
            Operand* t2 = newOperand(OP_TEMP, getTempNo, NULL);
            ir = translateExp(child, t1);
            List* ir2 = translateExp(getMBTreeNodeNextSibling(child2), t2);
            listJoin(ir, ir2);
            freeList(ir2);

            int ir_kind = kinds[getMBTreeNodeType(child2) - _PLUS];
            listAddNodeTail(ir, newIRCode(ir_kind, place, t1, t2));
            break;
          }
          case _DOT: {
            // Exp -> Exp DOT ID
            Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
            ir = translateExp(child, t1);
            if (t1->kind == OP_VARIABLE) {
              t1->kind = OP_ADDRESS;
              listAddNodeTail(ir, newIRCode(IR_GET_ADDR, t1, t1));
            }

            char* id = getID(getMBTreeNodeNextSibling(child2));
            intptr_t offset = 0;
            Type* t = NULL;
            for (FieldList* fl = t1->type->structure.structure; fl;
                 fl = fl->next) {
              if (strcmp(fl->name, id) == 0) {
                t = fl->type;
                break;
              }
              offset += getMemSize(fl->type);
            }

            operandTmp2Addr(place);
            place->type = t;
            listAddNodeTail(
                ir, newIRCode(IR_ADD, place, t1,
                              newOperand(OP_CONSTANT, (void*)offset, NULL)));
            break;
          }
          case _LB: {
            // Exp -> Exp LB Exp RB
            Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
            Operand* t2 = newOperand(OP_TEMP, getTempNo, NULL);
            ir = translateExp(child, t1);
            List* ir2 = translateExp(getMBTreeNodeNextSibling(child2), t2);
            listJoin(ir, ir2);
            freeList(ir2);

            if (t1->kind == OP_VARIABLE) {
              t1->kind = OP_ADDRESS;
              listAddNodeTail(ir, newIRCode(IR_GET_ADDR, t1, t1));
            }

            operandTmp2Addr(place);
            place->type = t1->type->array.element;
            listAddNodeTail(
                ir,
                newIRCode(IR_MUL, t2, t2,
                          newOperand(OP_CONSTANT,
                                     (void*)getMemSize(place->type), NULL)));
            listAddNodeTail(ir, newIRCode(IR_ADD, place, t1, t2));
            break;
          }
          default:
            // error
            assert(0);
            break;
        }
      break;
    }
    case _LP: {
      // Exp -> LP Exp RP
      ir = translateExp(getMBTreeNodeNextSibling(child), place);
      break;
    }
    case _MINUS: {
      // Exp -> MINUS Exp
      Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
      ir = translateExp(getMBTreeNodeNextSibling(child), t1);
      Operand* t2 = newOperand(OP_CONSTANT, 0, NULL);
      listAddNodeTail(ir, newIRCode(IR_SUB, place, t2, t1));
      break;
    }
    case _NOT: {
      Operand* label_true = newOperand(OP_LABEL, getLabelNo, NULL);
      Operand* label_false = newOperand(OP_LABEL, getLabelNo, NULL);
      ir = newList(NULL, NULL, NULL);
      listAddNodeTail(
          ir, newIRCode(IR_ASSIGN, place, newOperand(OP_CONSTANT, 0, NULL)));
      List* ir2 = translateCond(node, label_true, label_false);
      listJoin(ir, ir2);
      freeList(ir2);
      listAddNodeTail(ir, newIRCode(IR_LABEL, label_true));
      listAddNodeTail(ir, newIRCode(IR_ASSIGN, place,
                                    newOperand(OP_CONSTANT, (void*)1, NULL)));
      listAddNodeTail(ir, newIRCode(IR_LABEL, label_false));
      break;
    }
    case _ID: {
      char* id = getID(child);
      Type* t = htFind(ht, id)->val;
      assert(t != NULL);

      child = getMBTreeNodeNextSibling(child);
      if (child == NULL) {
        // Exp -> ID
        place->type = t;
        place->var_name = id;
        place->kind = OP_VARIABLE;

        ir = newList(NULL, NULL, NULL);
      } else if (getMBTreeNodeType(child) == _LP) {
        child = getMBTreeNodeNextSibling(child);
        if (getMBTreeNodeType(child) == _RP) {
          // Exp -> ID LP RP
          Operand* op = newOperand(OP_FUNCTION, id, t);
          ir = newList(NULL, NULL, NULL);

          if (strcmp(id, "read") == 0) {
            listAddNodeTail(ir, newIRCode(IR_READ, place));
          } else {
            listAddNodeTail(ir, newIRCode(IR_CALL, place, op));
          }
        } else if (getMBTreeNodeType(child) == _Args) {
          // Exp -> ID LP Args RP
          List* argList = newList(NULL, NULL, NULL);
          ir = translateArgs(child, argList);
          ListIter* iter = listGetIterator(argList, ITER_TAIL);

          if (strcmp(id, "write") == 0) {
            Operand* arg = listNext(iter)->value;
            listAddNodeTail(ir, newIRCode(IR_WRITE, arg));
            place->kind = OP_CONSTANT;
            place->constant = 0;
          } else {
            ListNode* arg;
            while (arg = listNext(iter)) {
              listAddNodeTail(ir, newIRCode(IR_ARG, arg->value));
            }
            listAddNodeTail(
                ir, newIRCode(IR_CALL, place, newOperand(OP_FUNCTION, id, t)));
          }
        } else {
          // error
          assert(0);
        }
      } else {
        // error
        assert(0);
      }
      break;
    }
    case _INT: {
      // Exp -> INT
      ir = newList(NULL, NULL, NULL);
      place->kind = OP_CONSTANT;
      place->constant = getINT(child);
      break;
    }
    case _FLOAT: {
      // Exp -> FLOAT
      // not supported
      assert(0);
      break;
    }
    default:
      // error
      assert(0);
      break;
  }

  return ir;
}

// process Args node
static List* translateArgs(MBTreeNode* node, List* argList) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _Args);

  MBTreeNode* child = getMBTreeNodeFirstChild(node);  // Exp
  Operand* tmp = newOperand(OP_TEMP, getTempNo, NULL);
  List* ir = translateExp(child, tmp);
  listAddNodeHead(argList, tmp);

  child = getMBTreeNodeNextSibling(child);
  if (child != NULL) {
    // Args -> Exp COMMA Args
    assert(getMBTreeNodeType(child) == _COMMA);
    List* ir2 = translateArgs(getMBTreeNodeNextSibling(child), argList);
    listJoin(ir, ir2);
    freeList(ir2);
  }

  return ir;
}

static List* translateCond(MBTreeNode* node, Operand* label_true,
                           Operand* label_false) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _Exp);
  MBTreeNode* child = getMBTreeNodeFirstChild(node);

  List* ir = NULL;

  if (getMBTreeNodeType(child) == _Exp) {
    MBTreeNode* child2 = getMBTreeNodeNextSibling(child);
    switch (getMBTreeNodeType(child2)) {
      case _RELOP: {
        // Exp -> Exp RELOP Exp
        Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
        Operand* t2 = newOperand(OP_TEMP, getTempNo, NULL);
        ir = translateExp(child, t1);
        List* ir2 = translateExp(getMBTreeNodeNextSibling(child2), t2);
        listJoin(ir, ir2);
        freeList(ir2);

        listAddNodeTail(
            ir, newIRCode(IR_IF_GOTO, t1, getMBTreeNodeValue(child2).val_str,
                          t2, label_true));
        listAddNodeTail(ir, newIRCode(IR_GOTO, label_false));
        break;
      }
      case _AND: {
        // Exp -> Exp AND Exp
        Operand* label1 = newOperand(OP_LABEL, getLabelNo, NULL);
        ir = translateCond(child, label1, label_false);
        listAddNodeTail(ir, newIRCode(IR_LABEL, label1));
        List* ir2 = translateCond(getMBTreeNodeNextSibling(child2), label_true,
                                  label_false);
        listJoin(ir, ir2);
        freeList(ir2);
        break;
      }
      case _OR: {
        // Exp -> Exp OR Exp
        Operand* label1 = newOperand(OP_LABEL, getLabelNo, NULL);
        ir = translateCond(child, label_true, label1);
        listAddNodeTail(ir, newIRCode(IR_LABEL, label1));
        List* ir2 = translateCond(getMBTreeNodeNextSibling(child2), label_true,
                                  label_false);
        listJoin(ir, ir2);
        freeList(ir2);
        break;
      }
      default: {
        goto other;
      }
    }
  } else if (getMBTreeNodeType(child) == _NOT) {
    ir =
        translateCond(getMBTreeNodeNextSibling(child), label_false, label_true);
  } else {
    goto other;
  }

  return ir;

other:
  Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
  ir = translateExp(node, t1);
  listAddNodeTail(ir, newIRCode(IR_IF_GOTO, t1, "!=",
                                newOperand(OP_CONSTANT, 0, NULL), label_true));
  listAddNodeTail(ir, newIRCode(IR_GOTO, label_false));
  return ir;
}

// process StmtList node
static List* translateStmtList(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _StmtList);
  List* ir = NULL;

  MBTreeNode* child = getMBTreeNodeFirstChild(node);
  if (getMBTreeNodeType(child) == _Empty) {
    // StmtList -> Empty
    ir = newList(NULL, NULL, NULL);
  } else {
    // StmtList -> Stmt StmtList
    ir = translateStmt(child);
    List* ir2 = translateStmtList(getMBTreeNodeNextSibling(child));
    listJoin(ir, ir2);
    freeList(ir2);
  }

  return ir;
}

// process Stmt node
static List* translateStmt(MBTreeNode* node) {
  if (node == NULL) {
    return NULL;
  }

  assert(getMBTreeNodeType(node) == _Stmt);
  List* ir = NULL;

  MBTreeNode* child = getMBTreeNodeFirstChild(node);
  switch (getMBTreeNodeType(child)) {
    case _Exp: {
      // Stmt -> Exp SEMI
      Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
      ir = translateExp(child, t1);
      break;
    }
    case _CompSt: {
      // Stmt -> CompSt
      ir = translateCompSt(child);
      break;
    }
    case _RETURN: {
      // Stmt -> RETURN Exp SEMI
      Operand* t1 = newOperand(OP_TEMP, getTempNo, NULL);
      ir = translateExp(getMBTreeNodeNextSibling(child), t1);

      listAddNodeTail(ir, newIRCode(IR_RETURN, t1));
      break;
    }
    case _IF: {
      // Stmt -> IF LP Exp RP Stmt
      // Stmt -> IF LP Exp RP Stmt ELSE Stmt
      Operand* label1 = newOperand(OP_LABEL, getLabelNo, NULL);
      Operand* label2 = newOperand(OP_LABEL, getLabelNo, NULL);

      child = getMBTreeNodeNextSibling(child);  // LP
      child = getMBTreeNodeNextSibling(child);  // Exp

      ir = translateCond(child, label1, label2);
      listAddNodeTail(ir, newIRCode(IR_LABEL, label1));

      child = getMBTreeNodeNextSibling(child);  // RP
      child = getMBTreeNodeNextSibling(child);  // Stmt

      List* ir2 = translateStmt(child);
      listJoin(ir, ir2);
      freeList(ir2);

      child = getMBTreeNodeNextSibling(child);
      if (child != NULL) {  // ELSE
        // Stmt -> IF LP Exp RP Stmt ELSE Stmt
        Operand* label3 = newOperand(OP_LABEL, getLabelNo, NULL);

        listAddNodeTail(ir, newIRCode(IR_GOTO, label3));
        listAddNodeTail(ir, newIRCode(IR_LABEL, label2));

        child = getMBTreeNodeNextSibling(child);  // Stmt
        ir2 = translateStmt(child);
        listJoin(ir, ir2);
        freeList(ir2);

        listAddNodeTail(ir, newIRCode(IR_LABEL, label3));
      } else {
        listAddNodeTail(ir, newIRCode(IR_LABEL, label2));
      }
      break;
    }
    case _WHILE: {
      // Stmt -> WHILE LP Exp RP Stmt
      Operand* label1 = newOperand(OP_LABEL, getLabelNo, NULL);
      Operand* label2 = newOperand(OP_LABEL, getLabelNo, NULL);
      Operand* label3 = newOperand(OP_LABEL, getLabelNo, NULL);

      child = getMBTreeNodeNextSibling(child);  // LP
      child = getMBTreeNodeNextSibling(child);  // Exp

      ir = translateCond(child, label2, label3);
      listAddNodeHead(ir, newIRCode(IR_LABEL, label1));
      listAddNodeTail(ir, newIRCode(IR_LABEL, label2));

      child = getMBTreeNodeNextSibling(child);  // RP
      child = getMBTreeNodeNextSibling(child);  // Stmt

      List* ir2 = translateStmt(child);
      listJoin(ir, ir2);
      freeList(ir2);

      listAddNodeTail(ir, newIRCode(IR_GOTO, label1));
      listAddNodeTail(ir, newIRCode(IR_LABEL, label3));
      break;
    }
    default:
      // error
      assert(0);
      break;
  }

  return ir;
}

static void displayIRCodeList(List* ir, FILE* out) {
  if (ir == NULL) {
    return;
  }

  ListIter* iter = listGetIterator(ir, ITER_HEAD);
  for (ListNode* node = listNext(iter); node; node = listNext(iter)) {
    printIRCode(out, node->value);
  }
  freeListIterator(iter);
}
