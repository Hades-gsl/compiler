%{
#include "syntax.tab.h"
%}

%option yylineno

INT         0|([1-9][0-9]*)
FLOAT       [0-9]+\.[0-9]+
ID          ([a-zA-Z]|_)([a-zA-Z]|_|[0-9])*
SEMI        ;
COMMA       ,
ASSIGNOP    =
RELOP       >|<|>=|<=|==|!=
PLUS        \+
MINUS       -
STAR        \*
DIV         /
AND         &&
OR          \|\|
DOT         \.
NOT         !
TYPE        int|float
LP          \(
RP          \)
LB          \[
RB          \]
LC          \{
RC          \}
STRUCT      struct
RETURN      return
IF          if
ELSE        else
WHILE       while

%%

INT       { return INT     ; }
FLOAT     { return FLOAT   ; }
ID        { return ID      ; }
SEMI      { return SEMI    ; }
COMMA     { return COMMA   ; }
ASSIGNOP  { return ASSIGNOP; }
RELOP     { return RELOP   ; }
PLUS      { return PLUS    ; }
MINUS     { return MINUS   ; }
STAR      { return STAR    ; }
DIV       { return DIV     ; }
AND       { return AND     ; }
OR        { return OR      ; }
DOT       { return DOT     ; }
NOT       { return NOT     ; }
TYPE      { return TYPE    ; }
LP        { return LP      ; }
RP        { return RP      ; }
LB        { return LB      ; }
RB        { return RB      ; }
LC        { return LC      ; }
RC        { return RC      ; }
STRUCT    { return STRUCT  ; }
RETURN    { return RETURN  ; }
IF        { return IF      ; }
ELSE      { return ELSE    ; }
WHILE     { return WHILE   ; }

%%

