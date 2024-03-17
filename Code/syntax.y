%{
#define YYSTYPE MBTreeNode*

#include <stdio.h>
#include "mbtree.h"
#include "lex.yy.c"

extern MBTreeNode* root;
extern void yyerror(const char *s);
%}

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

%%

Program : ExtDefList                            {$$ = newMBTreeNode(VAL_EMPTY, _Program, $1->data->lineno); addMBTreeNode($$, $1, NULL); root = $$;}
    ;
ExtDefList : ExtDef ExtDefList                  {//如果$2不为空，直接使用作为$2添加$1作为子结点
                                                 if($2 != NULL){
                                                    $$ = $2;
                                                    addMBTreeNode($$, $1, NULL);
                                                 }
                                                 else{
                                                    $$ = newMBTreeNode(VAL_EMPTY, _ExtDefList, $1->data->lineno);
                                                    addMBTreeNode($$, $1, NULL);
                                                 }
                                                 }
    |                                           
    ;
ExtDef : Specifier ExtDecList SEMI              {$$ = newMBTreeNode(VAL_EMPTY, _ExtDef, $1->data->lineno); addMBTreeNode($$, $3,$2, $1, NULL);}
    | Specifier SEMI                            {$$ = newMBTreeNode(VAL_EMPTY, _ExtDef, $1->data->lineno); addMBTreeNode($$, $2,$1, NULL);}
    | Specifier FunDec CompSt                   {$$ = newMBTreeNode(VAL_EMPTY, _ExtDef, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    ;
ExtDecList : VarDec                             {$$ = newMBTreeNode(VAL_EMPTY, _ExtDecList, $1->data->lineno); addMBTreeNode($$, $1, NULL);}
    | VarDec COMMA ExtDecList                   {$$ = $3;addMBTreeNode($$, $2, $1, NULL);};
    ;
    
Specifier : TYPE                                {$$ = newMBTreeNode(VAL_STR(yytext), _Specifier, yylineno);}
    | StructSpecifier                           {$$ = $1;}
    ;                 
StructSpecifier : STRUCT OptTag LC DefList RC   {$$ = newMBTreeNode(VAL_EMPTY, _StructSpecifier, yylineno); 
                                                addMBTreeNode($$, $5, NULL); 
                                                addMBTreeNode($$, $4, NULL); 
                                                addMBTreeNode($$, $3, NULL); 
                                                if ($2 != NULL) {
                                                    addMBTreeNode($$, $2, NULL); 
                                                }
                                                addMBTreeNode($$, $1, NULL); 
                                                }
    | STRUCT Tag                                {$$ = newMBTreeNode(VAL_EMPTY, _StructSpecifier, $2->data->lineno); addMBTreeNode($$, $2,$1, NULL);}
    ;             
OptTag : ID                                     {$$ = newMBTreeNode(VAL_STR(yytext), _OptTag, yylineno);}
    |                                           {$$ = NULL;}
    ;               
Tag : ID                                        {$$ = newMBTreeNode(VAL_STR(yytext), _Tag, yylineno);}
    ;

VarDec : ID                                     {$$ = newMBTreeNode(VAL_STR(yytext), _VarDec, yylineno);}
    | VarDec LB INT RB                          {$$ = newMBTreeNode(VAL_EMPTY, _VarDec, $3->data->lineno); addMBTreeNode($$,$4,$3,$2,$1, NULL); }
    ;               
FunDec : ID LP VarList RP                       {$$ = newMBTreeNode(VAL_STR(yytext), _FunDec, $1->data->lineno); addMBTreeNode($$, $4,$3,$2,$1, NULL);}
    | ID LP RP                                  {$$ = newMBTreeNode(VAL_STR(yytext), _FunDec, $1->data->lineno);addMBTreeNode($$, $3,$2,$1, NULL);}
    ;    

VarList : ParamDec COMMA VarList                {$$ = newMBTreeNode(VAL_EMPTY, _VarList, $1->data->lineno);addMBTreeNode($$, $3,$2, $1, NULL);}
    | ParamDec                                  {$$ = newMBTreeNode(VAL_EMPTY, _VarList, $1->data->lineno);addMBTreeNode($$, $1, NULL);}
    ;            
ParamDec : Specifier VarDec                     {$$ = newMBTreeNode(VAL_EMPTY, _ParamDec, $1->data->lineno);addMBTreeNode($$, $2, $1, NULL);}
    ; 

CompSt : LC DefList StmtList RC                 {$$ = newMBTreeNode(VAL_EMPTY, _CompSt, yylineno);
                                                addMBTreeNode($$, $4,NULL);
                                                if ($3 != NULL) { 
                                                addMBTreeNode($$, $3, NULL); 
                                                }
                                                if ($2 != NULL) { 
                                                    addMBTreeNode($$, $2, NULL); 
                                                }
                                                addMBTreeNode($$, $1, NULL);
                                                }
    ;               
StmtList : Stmt StmtList                        {if ($2 != NULL) {
                                                    $$ = $2;
                                                    addMBTreeNode($$, $1, NULL);
                                                } else {
                                                    $$ = newMBTreeNode(VAL_EMPTY, _StmtList, $1->data->lineno);
                                                    addMBTreeNode($$, $1, NULL);
                                                } }
    |               
    ;               
Stmt : Exp SEMI                                 {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, $1->data->lineno); addMBTreeNode($$,$2, $1, NULL);}
    | CompSt                                    {$$ = $1;}
    | RETURN Exp SEMI                           {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, $2->data->lineno); addMBTreeNode($$,$3, $2,$1, NULL);}
    | IF LP Exp RP Stmt                         {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, $3->data->lineno); addMBTreeNode($$, $5, $4, $3, $2, $1, NULL);}
    | IF LP Exp RP Stmt ELSE Stmt               {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, $3->data->lineno); addMBTreeNode($$, $7, $6, $5, $4, $3, $2, $1, NULL);}
    | WHILE LP Exp RP Stmt                      {$$ = newMBTreeNode(VAL_EMPTY, _Stmt, $3->data->lineno); addMBTreeNode($$, $5, $4, $3, $2, $1, NULL);}
    ;               
DefList : Def DefList                           {if ($2 != NULL) {
                                                    $$ = $2;
                                                    addMBTreeNode($$, $1, NULL);
                                                } else {
                                                    $$ = newMBTreeNode(VAL_EMPTY, _DefList, $1->data->lineno);
                                                    addMBTreeNode($$, $1, NULL);
                                                } }
    |               
    ;               
Def : Specifier DecList SEMI                    {$$ = newMBTreeNode(VAL_EMPTY, _Def, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    ;             
DecList : Dec                                   {$$ = newMBTreeNode(VAL_EMPTY, _DecList, $1->data->lineno); addMBTreeNode($$, $1, NULL);}
    | Dec COMMA DecList                         {$$ = newMBTreeNode(VAL_EMPTY, _DecList, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    ;          
Dec : VarDec                                    {$$ = $1;}
    | VarDec ASSIGNOP Exp                       {$$ = newMBTreeNode(VAL_EMPTY, _Dec, $1->data->lineno); addMBTreeNode($$, $3,$2, $1, NULL);}
    ;

Exp : Exp ASSIGNOP Exp                          {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp AND Exp                               {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp OR Exp                                {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp RELOP Exp                             {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp PLUS Exp                              {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp MINUS Exp                             {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp STAR Exp                              {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp DIV Exp                               {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | LP Exp RP                                 {$$ = $2;addMBTreeNode($$, $3, $2, $1, NULL);}
    | MINUS Exp                                 {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $2->data->lineno); addMBTreeNode($$, $2, $1, NULL);}
    | NOT Exp                                   {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $2->data->lineno); addMBTreeNode($$, $2, $1, NULL);}
    | ID LP Args RP                             {$$ = newMBTreeNode(VAL_STR(yytext), _Exp, $1->data->lineno); addMBTreeNode($$, $4, $3, $2, $1, NULL);}
    | ID LP RP                                  {$$ = newMBTreeNode(VAL_STR(yytext), _Exp, $1->data->lineno);addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp LB Exp RB                             {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $4, $3, $2, $1, NULL);}
    | Exp DOT ID                                {$$ = newMBTreeNode(VAL_EMPTY, _Exp, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | ID                                        {$$ = newMBTreeNode(VAL_STR(yytext), _Exp, $1->data->lineno);}
    | INT                                       {$$ = newMBTreeNode(VAL_INT(yytext), _Exp, $1->data->lineno);}
    | FLOAT                                     {$$ = newMBTreeNode(VAL_FLOAT(yytext), _Exp, $1->data->lineno);}
    ;               
Args : Exp COMMA Args                           {$$ = newMBTreeNode(VAL_EMPTY, _Args, $1->data->lineno); addMBTreeNode($$, $3, $2, $1, NULL);}
    | Exp                                       {$$ = newMBTreeNode(VAL_EMPTY, _Args, $1->data->lineno); addMBTreeNode($$, $1, NULL);}
    ;               

%%
