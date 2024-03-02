%{

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



%%

