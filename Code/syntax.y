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
ExtDefList : ExtDef ExtDefList                  {}
    | 
    ;
ExtDef : Specifier ExtDecList SEMI              {}
    | Specifier SEMI                            {}
    | Specifier FunDec CompSt                   {}
    ;
ExtDecList : VarDec                             {}
    | VarDec COMMA ExtDecList                   {}
    ;
Specifier : TYPE                                {}
    | StructSpecifier                           {}
    ;             
StructSpecifier : STRUCT OptTag LC DefList RC   {}
    | STRUCT Tag                                {}
    ;             
OptTag : ID                                     {}
    |               
    ;               
Tag : ID                                        {}
VarDec : ID                                     {}
    | VarDec LB INT RB                          {}
    ;               
FunDec : ID LP VarList RP                       {}
    | ID LP RP                                  {}
    ;               
VarList : ParamDec COMMA VarList                {}
    | ParamDec                                  {}
    ;            
ParamDec : Specifier VarDec                     {}
    ;               
CompSt : LC DefList StmtList RC                 {}
    ;               
StmtList : Stmt StmtList                        {}
    |               
    ;               
Stmt : Exp SEMI                                 {}
    | CompSt                                    {}
    | RETURN Exp SEMI                           {}
    | IF LP Exp RP Stmt                         {}
    | IF LP Exp RP Stmt ELSE Stmt               {}
    | WHILE LP Exp RP Stmt                      {}
    ;               
DefList : Def DefList                           {}
    |               
    ;               
Def : Specifier DecList SEMI                    {}
    ;             
DecList : Dec                                   {}
    | Dec COMMA DecList                         {}
    ;          
Dec : VarDec                                    {}
    | VarDec ASSIGNOP Exp                       {}
    ;           
Exp : Exp ASSIGNOP Exp                          {}
    | Exp AND Exp                               {}
    | Exp OR Exp                                {}
    | Exp RELOP Exp                             {}
    | Exp PLUS Exp                              {}
    | Exp MINUS Exp                             {}
    | Exp STAR Exp                              {}
    | Exp DIV Exp                               {}
    | LP Exp RP                                 {}
    | MINUS Exp                                 {}
    | NOT Exp                                   {}
    | ID LP Args RP                             {}
    | ID LP RP                                  {}
    | Exp LB Exp RB                             {}
    | Exp DOT ID                                {}
    | ID                                        {}
    | INT                                       {}
    | FLOAT                                     {}
    ;               
Args : Exp COMMA Args                           {}
    | Exp                                       {}
    ;               

%%
