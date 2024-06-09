#include "data.h"

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

extern char* strdup(const char*);

/*-------------------lexical analysis and syntax analysis-------------------*/
Val val_str(const char* s) { return (Val){.val_str = strdup(s)}; }

MBTreeNode* newMBTreeNodeData(Val val, Node_type type, unsigned int lineno) {
  Data* data = (Data*)malloc(sizeof(Data));
  *data = (Data){.type = type, .value = val, .lineno = lineno};
  return newMBTreeNode(data);
}

void displayMBTreeNode(const MBTreeNode* node, unsigned indent) {
  if (node == NULL || getMBTreeNodeType(node) == _Empty) return;
  if (node->firstChild != NULL && getMBTreeNodeType(node->firstChild) == _Empty)
    return;

  for (int i = 0; i < indent; i++) {
    printf(" ");
  }
  if (is_non_terminal(getMBTreeNodeType(node))) {
    printf("%s (%u)\n", type_strs[getMBTreeNodeType(node)],
           getMBTreeNodeLineNo(node));
  } else {
    printf("%s", type_strs[getMBTreeNodeType(node)]);
    switch (getMBTreeNodeType(node)) {
      case _ID:
      case _TYPE:
        printf(": %s\n", getMBTreeNodeValue(node).val_str);
        break;
      case _INT:
        printf(": %d\n", getMBTreeNodeValue(node).val_int);
        break;
      case _FLOAT:
        printf(": %f\n", getMBTreeNodeValue(node).val_float);
        break;
      default:
        printf("\n");
        break;
    }
  }

  for (MBTreeNode* child = node->firstChild; child != NULL;
       child = child->nextSibling) {
    displayMBTreeNode(child, indent + 2);
  }
}

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
              .structure.structure = copyFieldList(structure),
              .structure.memSize = 0};
  return t;
}

Type* newTypeArray(Type* element, int size) {
  Type* t = malloc(sizeof(Type));
  *t = (Type){.kind = ARRAY,
              .array.size = size,
              .array.element = NULL,
              .array.memSize = 0};
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
  return -1;
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
             .structure.structure = copyFieldList(t->structure.structure),
             .structure.memSize = t->structure.memSize};

  return newType;
}

Type* copyTypeArray(Type* t) {
  if (t == NULL) {
    return NULL;
  }

  assert(t->kind == ARRAY);

  Type* newType = malloc(sizeof(Type));
  *newType = (Type){.kind = ARRAY,
                    .array.size = t->array.size,
                    .array.element = NULL,
                    .array.memSize = t->array.memSize};
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

/*--------------------------------ir generate--------------------------------*/
size_t getMemSize(Type* t) {
  if (t == NULL || t->kind == FUNCTION) {
    return 0;
  }

  switch (t->kind) {
    case BASIC:
      return BASIC_MEM_SIZE;
    case ARRAY:
      if (t->array.memSize == 0) {
        t->array.memSize = t->array.size * getMemSize(t->array.element);
      }
      return t->array.memSize;
    case STRUCTURE: {
      if (t->structure.memSize == 0) {
        FieldList* fl = t->structure.structure;
        while (fl) {
          t->structure.memSize += getMemSize(fl->type);
          fl = fl->next;
        }
      }
      return t->structure.memSize;
    }
    default: {
      // we should never reach here
      assert(0);
    }
  }
}

Operand* newOperand(int kind, void* val, Type* type) {
  Operand* op = malloc(sizeof(Operand));
  *op = (Operand){.kind = kind, .type = type};
  switch (kind) {
    case OP_CONSTANT:
      op->constant = (intptr_t)val;
      break;
    case OP_TEMP:
      op->temp_no = (size_t)val;
      break;
    case OP_LABEL:
      op->label_no = (size_t)val;
      break;
    case OP_FUNCTION:
      op->func_name = (char*)val;
      break;
    case OP_VARIABLE:
      op->var_name = (char*)val;
      break;
    case OP_ADDRESS:
      op->base_name = (char*)val;
      break;
    default:
      // we should never reach here
      assert(0);
      break;
  }
  return op;
}

char* operand2str(Operand* op) {
  char* str = malloc(32);
  switch (op->kind) {
    case OP_CONSTANT:
      sprintf(str, "#%" PRIdPTR, op->constant);
      break;
    case OP_TEMP:
      sprintf(str, "t%zu", op->temp_no);
      break;
    case OP_LABEL:
      sprintf(str, "l%zu", op->label_no);
      break;
    case OP_FUNCTION:
      sprintf(str, "%s", op->func_name);
      break;
    case OP_VARIABLE:
      sprintf(str, "%s", op->var_name);
      break;
    case OP_ADDRESS:
      sprintf(str, "%s", op->base_name);
      break;
    default:
      // we should never reach here
      assert(0);
      break;
  }
  return str;
}

void operandTmp2Addr(Operand* op) {
  assert(op->kind == OP_TEMP);

  op->kind = OP_ADDRESS;
  char* name = malloc(32);
  sprintf(name, "t%zu", op->temp_no);
  op->base_name = name;
}

IRCode* newIRCode(int kind, ...) {
  va_list ap;
  va_start(ap, kind);

  IRCode* ir = malloc(sizeof(IRCode));
  ir->kind = kind;

  switch (kind) {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
      ir->op = va_arg(ap, Operand*);
      break;
    case IR_ASSIGN:
    case IR_GET_ADDR:
    case IR_GET_VALUE:
    case IR_SET_VALUE:
    case IR_CALL:
      ir->left = va_arg(ap, Operand*);
      ir->right = va_arg(ap, Operand*);
      break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
      ir->result = va_arg(ap, Operand*);
      ir->op1 = va_arg(ap, Operand*);
      ir->op2 = va_arg(ap, Operand*);
      break;
    case IR_DEC:
      ir->operand = va_arg(ap, Operand*);
      ir->size = va_arg(ap, int);
      break;
    case IR_IF_GOTO:
      ir->op_l = va_arg(ap, Operand*);
      ir->relop = va_arg(ap, char*);
      ir->op_r = va_arg(ap, Operand*);
      ir->label = va_arg(ap, Operand*);
      break;
    default:
      // we should never reach here
      assert(0);
      break;
  }

  va_end(ap);
  return ir;
}

void printIRCode(FILE* fout, IRCode* ir) {
  static char* ir_template[] = {
      "LABEL %s :\n",    "\nFUNCTION %s :\n", "%s := %s\n",
      "%s := %s + %s\n", "%s := %s - %s\n",   "%s := %s * %s\n",
      "%s := %s / %s\n", "%s := &%s\n",       "%s := *%s\n",
      "*%s := %s\n",     "GOTO %s\n",         "IF %s %s %s GOTO %s\n",
      "RETURN %s\n",     "DEC %s %d\n",       "ARG %s\n",
      "%s := CALL %s\n", "PARAM %s\n",        "READ %s\n",
      "WRITE %s\n",
  };

  char *s1 = NULL, *s2 = NULL, *s3 = NULL;

  switch (ir->kind) {
    case IR_LABEL:
    case IR_FUNCTION:
    case IR_GOTO:
    case IR_RETURN:
    case IR_ARG:
    case IR_PARAM:
    case IR_READ:
    case IR_WRITE:
      s1 = operand2str(ir->op);
      fprintf(fout, ir_template[ir->kind], s1);
      break;
    case IR_ASSIGN:
    case IR_GET_ADDR:
    case IR_GET_VALUE:
    case IR_SET_VALUE:
    case IR_CALL:
      s1 = operand2str(ir->left);
      s2 = operand2str(ir->right);
      fprintf(fout, ir_template[ir->kind], s1, s2);
      break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
      s1 = operand2str(ir->result);
      s2 = operand2str(ir->op1);
      s3 = operand2str(ir->op2);
      fprintf(fout, ir_template[ir->kind], s1, s2, s3);
      break;
    case IR_DEC:
      s1 = operand2str(ir->operand);
      fprintf(fout, ir_template[ir->kind], s1, ir->size);
      break;
    case IR_IF_GOTO:
      s1 = operand2str(ir->op_l);
      s2 = operand2str(ir->op_r);
      s3 = operand2str(ir->label);
      fprintf(fout, ir_template[ir->kind], s1, ir->relop, s2, s3);
      break;
    default:
      // we should never reach here
      assert(0);
      break;
  }

  if (s1) {
    free(s1);
  }
  if (s2) {
    free(s2);
  }
  if (s3) {
    free(s3);
  }
}

void displayIRCodeList(List* ir, FILE* out) {
  if (ir == NULL) {
    return;
  }

  ListIter* iter = listGetIterator(ir, ITER_HEAD);
  for (ListNode* node = listNext(iter); node; node = listNext(iter)) {
    printIRCode(out, node->value);
  }
  freeListIterator(iter);
}

/*------------------------------mips32 generate------------------------------*/

Variable* newVariable(Operand* op, int offset, int reg) {
  Variable* var = malloc(sizeof(Variable));
  *var = (Variable){.op = op, .offset = offset, .reg = reg};
  return var;
}