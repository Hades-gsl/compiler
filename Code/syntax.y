%{
#define YYSTYPE MBTreeNode*
#define YYDEBUG 1

#include <stdio.h>
#include "data.h"
#include "lex.yy.c"

extern int error_line;
extern MBTreeNode* root;
void yyerror(const char* msg);
%}

%locations
%define parse.error verbose

%token INT     
%token FLOAT   
%token ID      
%token SEMI    
%token COMMA   
%token ASSIGNOP
%token RELOP   
%token PLUS    
%token MINUS   
%token STAR    
%token DIV     
%token AND     
%token OR      
%token DOT     
%token NOT     
%token TYPE    
%token LP      
%token RP      
%token LB      
%token RB      
%token LC      
%token RC      
%token STRUCT  
%token RETURN  
%token IF      
%token ELSE    
%token WHILE   

%right ASSIGNOP
%left  OR
%left  AND
%left  RELOP
%left  PLUS MINUS
%left  STAR DIV
%right NOT
%left  DOT LP RP LB RB
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

Program : ExtDefList                            {$$ = newMBTreeNodeData(VAL_EMPTY, _Program, @$.first_line); addMBTreeNode($$, $1, NULL); root = $$;}
    ;
ExtDefList : ExtDef ExtDefList                  {$$ = newMBTreeNodeData(VAL_EMPTY, _ExtDefList, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    |                                           {$$ = newMBTreeNodeData(VAL_EMPTY, _ExtDefList, @$.first_line); addMBTreeNode($$, newMBTreeNodeData(VAL_EMPTY, _Empty, -1), NULL);}
    ;
ExtDef : Specifier ExtDecList SEMI              {$$ = newMBTreeNodeData(VAL_EMPTY, _ExtDef, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Specifier SEMI                            {$$ = newMBTreeNodeData(VAL_EMPTY, _ExtDef, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | Specifier FunDec CompSt                   {$$ = newMBTreeNodeData(VAL_EMPTY, _ExtDef, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | error SEMI
    | Specifier error SEMI
    | error Specifier SEMI
    ;
ExtDecList : VarDec                             {$$ = newMBTreeNodeData(VAL_EMPTY, _ExtDecList, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | VarDec COMMA ExtDecList                   {$$ = newMBTreeNodeData(VAL_EMPTY, _ExtDecList, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);};
    | VarDec error ExtDefList
    ;
Specifier : TYPE                                {$$ = newMBTreeNodeData(VAL_EMPTY, _Specifier, @$.first_line); addMBTreeNode($$, $1,NULL);}
    | StructSpecifier                           {$$ = newMBTreeNodeData(VAL_EMPTY, _Specifier, @$.first_line); addMBTreeNode($$, $1, NULL);}
    ;                 
StructSpecifier : STRUCT OptTag LC DefList RC   {$$ = newMBTreeNodeData(VAL_EMPTY, _StructSpecifier, @$.first_line); addMBTreeNode($$, $5, $4, $3, $2, $1, NULL);}
    | STRUCT Tag                                {$$ = newMBTreeNodeData(VAL_EMPTY, _StructSpecifier, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | STRUCT OptTag LC DefList error RC
    ;             
OptTag : ID                                     {$$ = newMBTreeNodeData(VAL_EMPTY, _OptTag, @$.first_line); addMBTreeNode($$, $1, NULL);}
    |                                           {$$ = newMBTreeNodeData(VAL_EMPTY, _OptTag, @$.first_line); addMBTreeNode($$, newMBTreeNodeData(VAL_EMPTY, _Empty, -1), NULL);}
    ;               
Tag : ID                                        {$$ = newMBTreeNodeData(VAL_EMPTY, _Tag, @$.first_line); addMBTreeNode($$, $1, NULL);}
    ;
VarDec : ID                                     {$$ = newMBTreeNodeData(VAL_EMPTY, _VarDec, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | VarDec LB INT RB                          {$$ = newMBTreeNodeData(VAL_EMPTY, _VarDec, @$.first_line); addMBTreeNode($$,$4,$3,$2,$1, NULL); }
    | VarDec LB error RB
    ;               
FunDec : ID LP VarList RP                       {$$ = newMBTreeNodeData(VAL_EMPTY, _FunDec, @$.first_line); addMBTreeNode($$, $4,$3,$2,$1, NULL);}
    | ID LP RP                                  {$$ = newMBTreeNodeData(VAL_EMPTY, _FunDec, @$.first_line); addMBTreeNode($$, $3,$2,$1, NULL);}
    | ID LP error RP
    | error LP VarList RP
    ;    
VarList : ParamDec COMMA VarList                {$$ = newMBTreeNodeData(VAL_EMPTY, _VarList, @$.first_line); addMBTreeNode($$, $3,$2, $1, NULL);}
    | ParamDec                                  {$$ = newMBTreeNodeData(VAL_EMPTY, _VarList, @$.first_line); addMBTreeNode($$, $1, NULL);}
    ;            
ParamDec : Specifier VarDec                     {$$ = newMBTreeNodeData(VAL_EMPTY, _ParamDec, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    ; 
CompSt : LC DefList StmtList RC                 {$$ = newMBTreeNodeData(VAL_EMPTY, _CompSt, @$.first_line); addMBTreeNode($$, $4, $3, $2, $1, NULL);}
    ;               
StmtList : Stmt StmtList                        {$$ = newMBTreeNodeData(VAL_EMPTY, _StmtList, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    |                                           {$$ = newMBTreeNodeData(VAL_EMPTY, _StmtList, @$.first_line); addMBTreeNode($$, newMBTreeNodeData(VAL_EMPTY, _Empty, -1), NULL);}
    ;               
Stmt : Exp SEMI                                 {$$ = newMBTreeNodeData(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | CompSt                                    {$$ = newMBTreeNodeData(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | RETURN Exp SEMI                           {$$ = newMBTreeNodeData(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $3, $2,$1, NULL);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE   {$$ = newMBTreeNodeData(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $5, $4, $3, $2, $1, NULL);}
    | IF LP Exp RP Stmt ELSE Stmt               {$$ = newMBTreeNodeData(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $7, $6, $5, $4, $3, $2, $1, NULL);}
    | WHILE LP Exp RP Stmt                      {$$ = newMBTreeNodeData(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $5, $4, $3, $2, $1, NULL);}
    | error SEMI
    | error Stmt
    | WHILE LP error RP Stmt
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE
    | IF LP error RP Stmt ELSE Stmt
    | Exp error
    ;               
DefList : Def DefList                           {$$ = newMBTreeNodeData(VAL_EMPTY, _DefList, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    |                                           {$$ = newMBTreeNodeData(VAL_EMPTY, _DefList, @$.first_line); addMBTreeNode($$, newMBTreeNodeData(VAL_EMPTY, _Empty, -1), NULL);}
    ;               
Def : Specifier DecList SEMI                    {$$ = newMBTreeNodeData(VAL_EMPTY, _Def, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Specifier error SEMI
    | Specifier DecList error
    ;             
DecList : Dec                                   {$$ = newMBTreeNodeData(VAL_EMPTY, _DecList, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | Dec COMMA DecList                         {$$ = newMBTreeNodeData(VAL_EMPTY, _DecList, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    ;          
Dec : VarDec                                    {$$ = newMBTreeNodeData(VAL_EMPTY, _Dec, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | VarDec ASSIGNOP Exp                       {$$ = newMBTreeNodeData(VAL_EMPTY, _Dec, @$.first_line); addMBTreeNode($$, $3,$2, $1, NULL);}
    ;
Exp : Exp ASSIGNOP Exp                          {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp AND Exp                               {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp OR Exp                                {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp RELOP Exp                             {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp PLUS Exp                              {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp MINUS Exp                             {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp STAR Exp                              {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp DIV Exp                               {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | LP Exp RP                                 {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | MINUS Exp                                 {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | NOT Exp                                   {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | ID LP Args RP                             {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $4, $3, $2, $1, NULL);}
    | ID LP RP                                  {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp LB Exp RB                             {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $4, $3, $2, $1, NULL);}
    | Exp DOT ID                                {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | ID                                        {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | INT                                       {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | FLOAT                                     {$$ = newMBTreeNodeData(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | Exp ASSIGNOP error
    | Exp PLUS error
    | Exp MINUS error
    | Exp STAR error
    | Exp DIV error
    | Exp RELOP error
    | Exp AND error
    | Exp OR error
    | NOT error
    | MINUS error
    | LP error RP
    | ID LP error RP
    ;               
Args : Exp COMMA Args                           {$$ = newMBTreeNodeData(VAL_EMPTY, _Args, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp                                       {$$ = newMBTreeNodeData(VAL_EMPTY, _Args, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | error COMMA Args
    ;               

%%

void yyerror(const char* msg) {
    if (error_line == yylineno) return;
    printf("Error type B at Line %d: %s, near \"%s\".\n",yylineno, msg, yytext);
    has_error = 1;
}