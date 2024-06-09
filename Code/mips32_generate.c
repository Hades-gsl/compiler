#include "data.h"
#include "hash.h"
#include "list.h"

#define MIPS32_REG_NUM 32
#define FUNC_PTR_CAST(f) ((unsigned int (*)(const void*))f)
#define SAVE_FP_RA()                      \
  fprintf(fout, "\taddi $sp, $sp, -8\n"); \
  fprintf(fout, "\tsw $ra, 0($sp)\n");    \
  fprintf(fout, "\tsw $fp, 4($sp)\n")
#define RESTORE_FP_RA()                \
  fprintf(fout, "\tlw $ra, 0($sp)\n"); \
  fprintf(fout, "\tlw $fp, 4($sp)\n"); \
  fprintf(fout, "\taddi $sp, $sp, 8\n")

extern int keyCompare(void* privDataPtr, const void* a, const void* b);

static Register reg[MIPS32_REG_NUM];
static HashTable* varTable;
static int offset = 0;
static int param_num = 0;
static int arg_num = 0;
static HtType* type;

static void init(FILE* fout);
static void setupStackFrame(ListNode* node);
static void insertVariable(Operand* op);
static int getRegister(Variable* var, FILE* fout);
static void freeRegister(int reg_num);

static void genLabel(IRCode* ir, FILE* fout);
static void genFunction(IRCode* ir, FILE* fout);
static void genAssign(IRCode* ir, FILE* fout);
static void genAdd(IRCode* ir, FILE* fout);
static void genSub(IRCode* ir, FILE* fout);
static void genMul(IRCode* ir, FILE* fout);
static void genDiv(IRCode* ir, FILE* fout);
static void genGetAddr(IRCode* ir, FILE* fout);
static void genGetValue(IRCode* ir, FILE* fout);
static void genSetValue(IRCode* ir, FILE* fout);
static void genGoto(IRCode* ir, FILE* fout);
static void genIfGoto(IRCode* ir, FILE* fout);
static void genReturn(IRCode* ir, FILE* fout);
static void genDec(IRCode* ir, FILE* fout);
static void genArg(IRCode* ir, FILE* fout);
static void genCall(IRCode* ir, FILE* fout);
static void genParam(IRCode* ir, FILE* fout);
static void genRead(IRCode* ir, FILE* fout);
static void genWrite(IRCode* ir, FILE* fout);

static void (*mips32GenFunctions[])(IRCode*, FILE*) = {
    genLabel,   genFunction, genAssign,   genAdd,  genSub,    genMul,    genDiv,
    genGetAddr, genGetValue, genSetValue, genGoto, genIfGoto, genReturn, genDec,
    genArg,     genCall,     genParam,    genRead, genWrite};

/*
 * stack frame layout
 *
 * high address
 * +--------+
 * |  arg2  | <- $fp + 12
 * +--------+
 * |  arg1  | <- $fp + 8
 * +--------+
 * |  ret   | <- $fp + 4 (return address)
 * +--------+
 * | old $fp| <-$fp (old frame pointer)
 * +--------+
 * |  var1  | <- $fp - 4
 * +--------+
 * |  var2  | <- $fp - 8
 * +--------+
 * |  ...   |
 * |  ...   |
 * +--------+
 * |  ...   | <- $sp (stack pointer)
 * +--------+
 * low address
 *
 */

// Entry point for MIPS32 code generation
void MIPS32Generate(List* irList, FILE* fout) {
  if (irList == NULL) return;

  init(fout);

  ListIter* iter = listGetIterator(irList, ITER_HEAD);
  for (ListNode* node = listNext(iter); node != NULL; node = listNext(iter)) {
    IRCode* ir = (IRCode*)node->value;
    if (ir->kind == IR_FUNCTION) {
      setupStackFrame(node);
    }

    mips32GenFunctions[ir->kind](ir, fout);
  }
  freeListIterator(iter);
}

// add read and write functions, initialize registers and variables list
static void init(FILE* fout) {
  const char* init_code =
      ".data\n"
      "_prompt: .asciiz \"Enter an integer:\"\n"
      "_ret: .asciiz \"\\n\"\n"
      ".globl main\n"
      ".text\n"
      "\n"
      "read:\n"
      "\tli $v0, 4\n"
      "\tla $a0, _prompt\n"
      "\tsyscall\n"
      "\tli $v0, 5\n"
      "\tsyscall\n"
      "\tjr $ra\n"
      "\n"
      "write:\n"
      "\tli $v0, 1\n"
      "\tsyscall\n"
      "\tli $v0, 4\n"
      "\tla $a0, _ret\n"
      "\tsyscall\n"
      "\tmove $v0, $0\n"
      "\tjr $ra\n";

  fprintf(fout, "%s", init_code);

  for (int i = 0; i < MIPS32_REG_NUM; i++) {
    reg[i].kind = i;
    reg[i].var = NULL;
    reg[i].used = 0;
  }

  type = malloc(sizeof(HtType));
  *type = (HtType){.hashFunction = FUNC_PTR_CAST(htGenHashFunction),
                   .keyDup = NULL,
                   .valDup = NULL,
                   .keyCompare = keyCompare,
                   .keyDestructor = NULL,
                   .valDestructor = NULL};
  varTable = htCreate(type, NULL);
}

// setup stack frame for function
static void setupStackFrame(ListNode* node) {
  assert(node);

  IRCode* ir = (IRCode*)node->value;
  assert(ir && ir->kind == IR_FUNCTION);

  htRelease(varTable);
  varTable = htCreate(type, NULL);
  param_num = 0;
  offset = 0;

  for (ListNode* p = node->next; p != NULL; p = p->next) {
    IRCode* ir = (IRCode*)p->value;
    switch (ir->kind) {
      case IR_LABEL:
      case IR_FUNCTION:
      case IR_GOTO:
        break;
      case IR_PARAM:
        Variable* var = newVariable(ir->op, BASIC_MEM_SIZE * param_num + 8, -1);
        htAdd(varTable, operand2str(ir->op), var);
        param_num++;
        break;
      case IR_ARG:
      case IR_RETURN:
      case IR_READ:
      case IR_WRITE:
        insertVariable(ir->op);
        break;
      case IR_ASSIGN:
      case IR_GET_ADDR:
      case IR_GET_VALUE:
      case IR_SET_VALUE:
        insertVariable(ir->left);
        insertVariable(ir->right);
        break;
      case IR_CALL:
        insertVariable(ir->left);
        break;
      case IR_ADD:
      case IR_SUB:
      case IR_MUL:
      case IR_DIV:
        insertVariable(ir->result);
        insertVariable(ir->op1);
        insertVariable(ir->op2);
        break;
      case IR_DEC:
        offset -= (ir->size - BASIC_MEM_SIZE);
        insertVariable(ir->operand);
        break;
      case IR_IF_GOTO:
        insertVariable(ir->op_l);
        insertVariable(ir->op_r);
        break;
      default:
        // we should never reach here
        assert(0);
        break;
    }
  }
}

// insert variable into variable table
static void insertVariable(Operand* op) {
  assert(op);

  if (op->kind == OP_CONSTANT) return;

  if (htFind(varTable, operand2str(op)) != NULL) return;

  offset -= BASIC_MEM_SIZE;

  Variable* var = newVariable(op, offset, -1);
  assert(htAdd(varTable, operand2str(op), var) == HT_OK);
}

static Variable* findVariable(Operand* op) {
  assert(op);

  HashEntry* entry = htFind(varTable, operand2str(op));
  if (entry == NULL) {
    if (op->kind == OP_CONSTANT) {
      return newVariable(op, -1, op->constant);
    }
    assert(0);
    return NULL;
  }

  Variable* var = entry->val;
  assert(var);

  return var;
}

// get register, load variable into register
static int getRegister(Variable* var, FILE* fout) {
  assert(var);

  int i;

  for (i = REG_T0; i <= REG_T9; i++) {
    if (!reg[i].used) {
      reg[i].var = var;
      reg[i].used = 1;
      break;
    }
  }

  assert(i <= REG_T9);

  if (var->op->kind == OP_CONSTANT) {
    fprintf(fout, "\tli %s, %ld\n", register_names[i], var->op->constant);
  } else {
    fprintf(fout, "\tlw %s, %d($fp) # %s\n", register_names[i], var->offset,
            operand2str(var->op));
  }

  return i;
}

// free register
static void freeRegister(int reg_num) {
  assert(reg_num >= REG_T0 && reg_num <= REG_T9);

  reg[reg_num].var = NULL;
  reg[reg_num].used = 0;
}

// generate MIPS32 code for Label, e.g. l1:
static void genLabel(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_LABEL);

  fprintf(fout, "%s:\n", operand2str(ir->op));
}

// generate MIPS32 code for Function, e.g. main:
static void genFunction(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_FUNCTION);

  fprintf(fout, "\n%s:\n", operand2str(ir->op));
  fprintf(fout, "\tmove $fp, $sp\n");
  fprintf(fout, "\taddi $sp, $sp, %d\n", offset);
}

// generate MIPS32 code for Assign, e.g. x = y
static void genAssign(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_ASSIGN);

  Variable* left = findVariable(ir->left);
  Variable* right = findVariable(ir->right);

  int reg_num = getRegister(right, fout);

  fprintf(fout, "\tsw %s, %d($fp) # %s\n", register_names[reg_num],
          left->offset, operand2str(ir->left));

  freeRegister(reg_num);
}

// generate MIPS32 code for Arithmetic, e.g. x = y op z
static void genArithmetic(IRCode* ir, FILE* fout, int type) {
  assert(ir && (ir->kind == IR_ADD || ir->kind == IR_SUB ||
                ir->kind == IR_MUL || ir->kind == IR_DIV));

  Variable* result = findVariable(ir->result);
  Variable* op1 = findVariable(ir->op1);
  Variable* op2 = findVariable(ir->op2);

  int reg_num1 = getRegister(op1, fout);
  int reg_num2 = getRegister(op2, fout);

  switch (type) {
    case IR_ADD:
      fprintf(fout, "\tadd %s, %s, %s\n", register_names[reg_num1],
              register_names[reg_num1], register_names[reg_num2]);
      break;
    case IR_SUB:
      fprintf(fout, "\tsub %s, %s, %s\n", register_names[reg_num1],
              register_names[reg_num1], register_names[reg_num2]);
      break;
    case IR_MUL:
      fprintf(fout, "\tmul %s, %s, %s\n", register_names[reg_num1],
              register_names[reg_num1], register_names[reg_num2]);
      break;
    case IR_DIV:
      fprintf(fout, "\tdiv %s, %s\n", register_names[reg_num1],
              register_names[reg_num2]);
      fprintf(fout, "\tmflo %s\n", register_names[reg_num1]);
      break;
    default:
      // we should never reach here
      assert(0);
      break;
  }

  fprintf(fout, "\tsw %s, %d($fp) # %s\n", register_names[reg_num1],
          result->offset, operand2str(ir->result));

  freeRegister(reg_num1);
  freeRegister(reg_num2);
}

// generate MIPS32 code for GetAddr, e.g. x = y + z
static void genAdd(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_ADD);

  genArithmetic(ir, fout, IR_ADD);
}

// generate MIPS32 code for Sub, e.g. x = y - z
static void genSub(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_SUB);

  genArithmetic(ir, fout, IR_SUB);
}

// generate MIPS32 code for Mul, e.g. x = y * z
static void genMul(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_MUL);

  genArithmetic(ir, fout, IR_MUL);
}

// generate MIPS32 code for Div, e.g. x = y / z
static void genDiv(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_DIV);

  genArithmetic(ir, fout, IR_DIV);
}

// generate MIPS32 code for GetAddr, e.g. x = &y
static void genGetAddr(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_GET_ADDR);

  Variable* left = findVariable(ir->left);
  Variable* right = findVariable(ir->right);

  int reg_num = getRegister(left, fout);

  fprintf(fout, "\taddi %s, $fp, %d\n", register_names[reg_num], right->offset);
  fprintf(fout, "\tsw %s, %d($fp) # %s\n", register_names[reg_num],
          left->offset, operand2str(ir->left));

  freeRegister(reg_num);
}

// generate MIPS32 code for GetValue, e.g. x = *y
static void genGetValue(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_GET_VALUE);

  Variable* left = findVariable(ir->left);
  Variable* right = findVariable(ir->right);

  int reg_num = getRegister(right, fout);

  fprintf(fout, "\tlw %s, 0(%s)\n", register_names[reg_num],
          register_names[reg_num]);
  fprintf(fout, "\tsw %s, %d($fp) # %s\n", register_names[reg_num],
          left->offset, operand2str(ir->left));

  freeRegister(reg_num);
}

// generate MIPS32 code for SetValue, e.g. *x = y
static void genSetValue(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_SET_VALUE);

  Variable* left = findVariable(ir->left);
  Variable* right = findVariable(ir->right);

  int left_reg_num = getRegister(left, fout);
  int right_reg_num = getRegister(right, fout);

  fprintf(fout, "\tsw %s, 0(%s)\n", register_names[right_reg_num],
          register_names[left_reg_num]);

  freeRegister(left_reg_num);
  freeRegister(right_reg_num);
}

// generate MIPS32 code for Goto, e.g. goto l
static void genGoto(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_GOTO);

  fprintf(fout, "\tj %s\n", operand2str(ir->op));
}

// generate MIPS32 code for IfGoto, e.g. if x [relop] y goto l
static void genIfGoto(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_IF_GOTO);

  Variable* op_l = findVariable(ir->op_l);
  Variable* op_r = findVariable(ir->op_r);

  int reg_num1 = getRegister(op_l, fout);
  int reg_num2 = getRegister(op_r, fout);

  char* relop;
  switch (ir->relop[0]) {
    case '<':
      if (ir->relop[1] == '=') {
        relop = "ble";
      } else {
        relop = "blt";
      }
      break;
    case '>':
      if (ir->relop[1] == '=') {
        relop = "bge";
      } else {
        relop = "bgt";
      }
      break;
    case '=':
      relop = "beq";
      break;
    case '!':
      relop = "bne";
      break;
    default:
      assert(0);
      break;
  }

  fprintf(fout, "\t%s %s, %s, %s\n", relop, register_names[reg_num1],
          register_names[reg_num2], operand2str(ir->label));

  freeRegister(reg_num1);
  freeRegister(reg_num2);
}

// generate MIPS32 code for Return, e.g. return x
static void genReturn(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_RETURN);

  Variable* op = findVariable(ir->op);

  int reg_num = getRegister(op, fout);

  fprintf(fout, "\tmove $v0, %s\n", register_names[reg_num]);
  fprintf(fout, "\tmove $sp, $fp\n");
  fprintf(fout, "\tjr $ra\n");

  freeRegister(reg_num);
}

// generate MIPS32 code for Dec, e.g. dec x [size]
static void genDec(IRCode* ir, FILE* fout) { assert(ir && ir->kind == IR_DEC); }

// generate MIPS32 code for Arg, e.g. arg x
static void genArg(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_ARG);

  Variable* op = findVariable(ir->op);

  int reg_num = getRegister(op, fout);

  fprintf(fout, "\taddi $sp, $sp, -4\n");
  fprintf(fout, "\tsw %s, 0($sp)\n", register_names[reg_num]);

  freeRegister(reg_num);

  arg_num++;
}

// generate MIPS32 code for Call, e.g. x = call f
static void genCall(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_CALL);

  // save $ra and $fp
  SAVE_FP_RA();

  fprintf(fout, "\tjal %s\n", operand2str(ir->right));

  // restore $ra and $fp
  RESTORE_FP_RA();

  // save return value
  Variable* left = findVariable(ir->left);

  fprintf(fout, "\tsw $v0, %d($fp) # %s\n", left->offset,
          operand2str(ir->left));

  fprintf(fout, "\taddi $sp, $sp, %d\n", arg_num * BASIC_MEM_SIZE);
  arg_num = 0;
}

// generate MIPS32 code for Param, e.g. param x
static void genParam(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_PARAM);
}

// generate MIPS32 code for Read, e.g. read x
static void genRead(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_READ);

  Variable* op = findVariable(ir->op);

  SAVE_FP_RA();

  fprintf(fout, "\tjal read\n");

  RESTORE_FP_RA();

  fprintf(fout, "\tsw $v0, %d($fp) # %s\n", op->offset, operand2str(ir->op));
}

// generate MIPS32 code for Write, e.g. write x
static void genWrite(IRCode* ir, FILE* fout) {
  assert(ir && ir->kind == IR_WRITE);

  Variable* op = findVariable(ir->op);

  int reg_num = getRegister(op, fout);

  fprintf(fout, "\tmove $a0, %s\n", register_names[reg_num]);

  SAVE_FP_RA();

  fprintf(fout, "\tjal write\n");

  RESTORE_FP_RA();

  freeRegister(reg_num);
}