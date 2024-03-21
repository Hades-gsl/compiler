%{
#define YYSTYPE MBTreeNode*

#include <stdio.h>
#include "mbtree.h"
#include "lex.yy.c"

extern MBTreeNode* root;
extern void yyerror(const char *s);
extern MBTreeNode* empty;
%}

%union {
    MBTreeNode* node;
}

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

%type<node> Program, ExtDefList, ExtDef, ExtDecList, Specifier, StructSpecifier, OptTag, Tag, VarDec, FunDec, VarList, ParamDec, CompSt, StmtList, Stmt, DefList, Def, DecList, Dec, Exp, Args

%%

Program : ExtDefList                            {$$ = newMBTreeNode(VAL_EMPTY, _Program, @$.first_line); addMBTreeNode($$, $1, NULL); root = $$;}
    ;
ExtDefList : ExtDef ExtDefList                  {$$ = newMBTreeNode(VAL_EMPTY, _ExtDefList, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    |                                           {$$ = newMBTreeNode(VAL_EMPTY, _ExtDefList, @$.first_line); addMBTreeNode($$, empty, NULL);}
    ;
ExtDef : Specifier ExtDecList SEMI              {$$ = newMBTreeNode(VAL_EMPTY, _ExtDef, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Specifier SEMI                            {$$ = newMBTreeNode(VAL_EMPTY, _ExtDef, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | Specifier FunDec CompSt                   {$$ = newMBTreeNode(VAL_EMPTY, _ExtDef, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    ;
ExtDecList : VarDec                             {$$ = newMBTreeNode(VAL_EMPTY, _ExtDecList, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | VarDec COMMA ExtDecList                   {$$ = newMBTreeNode(VAL_EMPTY, _ExtDecList, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);};
    ;
Specifier : TYPE                                {$$ = newMBTreeNode(VAL_EMPTY, _Specifier, @$.first_line); addMBTreeNode($$, $1,NULL);}
    | StructSpecifier                           {$$ = newMBTreeNode(VAL_EMPTY, _StructSpecifier, @$.first_line); addMBTreeNode($$, $1, NULL);}
    ;                 
StructSpecifier : STRUCT OptTag LC DefList RC   {$$ = newMBTreeNode(VAL_EMPTY, _StructSpecifier, @$.first_line); addMBTreeNode($$, $5, $4, $3, $2, $1, NULL);}
    | STRUCT Tag                                {$$ = newMBTreeNode(VAL_EMPTY, _StructSpecifier, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    ;             
OptTag : ID                                     {$$ = newMBTreeNode(VAL_EMPTY, _OptTag, @$.first_line); addMBTreeNode($$, $1, NULL);}
    |                                           {$$ = newMBTreeNode(VAL_EMPTY, _OptTag, @$.first_line); addMBTreeNode($$, empty, NULL);}
    ;               
Tag : ID                                        {$$ = newMBTreeNode(VAL_EMPTY, _Tag, @$.first_line); addMBTreeNode($$, $1, NULL);}
    ;
VarDec : ID                                     {$$ = newMBTreeNode(VAL_EMPTY, _VarDec, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | VarDec LB INT RB                          {$$ = newMBTreeNode(VAL_EMPTY, _VarDec, @$.first_line); addMBTreeNode($$,$4,$3,$2,$1, NULL); }
    ;               
FunDec : ID LP VarList RP                       {$$ = newMBTreeNode(VAL_EMPTY, _FunDec, @$.first_line); addMBTreeNode($$, $4,$3,$2,$1, NULL);}
    | ID LP RP                                  {$$ = newMBTreeNode(VAL_EMPTY, _FunDec, @$.first_line); addMBTreeNode($$, $3,$2,$1, NULL);}
    ;    
VarList : ParamDec COMMA VarList                {$$ = newMBTreeNode(VAL_EMPTY, _VarList, @$.first_line); addMBTreeNode($$, $3,$2, $1, NULL);}
    | ParamDec                                  {$$ = newMBTreeNode(VAL_EMPTY, _VarList, @$.first_line); addMBTreeNode($$, $1, NULL);}
    ;            
ParamDec : Specifier VarDec                     {$$ = newMBTreeNode(VAL_EMPTY, _ParamDec, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    ; 
CompSt : LC DefList StmtList RC                 {$$ = newMBTreeNode(VAL_EMPTY, _CompSt, @$.first_line); addMBTreeNode($$, $4, $3, $2, $1, NULL);}
    ;               
StmtList : Stmt StmtList                        {$$ = newMBTreeNode(VAL_EMPTY, _StmtList, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    |                                           {$$ = newMBTreeNode(VAL_EMPTY, _StmtList, @$.first_line); addMBTreeNode($$, empty, NULL);}
    ;               
Stmt : Exp SEMI                                 {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | CompSt                                    {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | RETURN Exp SEMI                           {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $3, $2,$1, NULL);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE   {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $5, $4, $3, $2, $1, NULL);}
    | IF LP Exp RP Stmt ELSE Stmt               {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $7, $6, $5, $4, $3, $2, $1, NULL);}
    | WHILE LP Exp RP Stmt                      {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, @$.first_line); addMBTreeNode($$, $5, $4, $3, $2, $1, NULL);}
    ;               
DefList : Def DefList                           {$$ = newMBTreeNode(VAL_EMPTY, _DefList, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    |                                           {$$ = newMBTreeNode(VAL_EMPTY, _DefList, @$.first_line); addMBTreeNode($$, empty, NULL);}
    ;               
Def : Specifier DecList SEMI                    {$$ = newMBTreeNode(VAL_EMPTY, _Def, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    ;             
DecList : Dec                                   {$$ = newMBTreeNode(VAL_EMPTY, _DecList, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | Dec COMMA DecList                         {$$ = newMBTreeNode(VAL_EMPTY, _DecList, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    ;          
Dec : VarDec                                    {$$ = newMBTreeNode(VAL_EMPTY, _VarDec, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | VarDec ASSIGNOP Exp                       {$$ = newMBTreeNode(VAL_EMPTY, _Dec, @$.first_line); addMBTreeNode($$, $3,$2, $1, NULL);}
    ;

Exp : Exp ASSIGNOP Exp                          {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp AND Exp                               {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp OR Exp                                {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp RELOP Exp                             {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp PLUS Exp                              {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp MINUS Exp                             {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp STAR Exp                              {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp DIV Exp                               {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | LP Exp RP                                 {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | MINUS Exp                                 {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | NOT Exp                                   {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $2, $1, NULL);}
    | ID LP Args RP                             {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $4, $3, $2, $1, NULL);}
    | ID LP RP                                  {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp LB Exp RB                             {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $4, $3, $2, $1, NULL);}
    | Exp DOT ID                                {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | ID                                        {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | INT                                       {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $1, NULL);}
    | FLOAT                                     {$$ = newMBTreeNode(VAL_EMPTY, _Exp, @$.first_line); addMBTreeNode($$, $1, NULL);}
    ;               
Args : Exp COMMA Args                           {$$ = newMBTreeNode(VAL_EMPTY, _Args, @$.first_line); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp                                       {$$ = newMBTreeNode(VAL_EMPTY, _Args, @$.first_line); addMBTreeNode($$, $1, NULL);}
    ;               

%%
