%{
#include <stdio.h>

#include "syntax.tab.h"
#include "mbtree.h"

#define YY_VAL(v, t)\
    yylval = newMBTreeNode(v, _##t, yylineno)
%}

%option yylineno

INT         0|([1-9][0-9]*)
FLOAT       [0-9]+\.[0-9]+
SEMI        ";"
COMMA       ","
ASSIGNOP    "="
RELOP       >|<|>=|<=|==|!=
PLUS        \+
MINUS       "-"
STAR        \*
DIV         "/"
AND         "&&"
OR          \|\|
DOT         \.
NOT         "!"
TYPE        int|float
LP          \(
RP          \)
LB          \[
RB          \]
LC          \{
RC          \}
STRUCT      "struct"
RETURN      "return"
IF          "if"
ELSE        "else"
WHILE       "while"
ID          ([a-zA-Z]|_)([a-zA-Z]|_|[0-9])*
WHITE       [\t\n ]

%%

{INT}       { YY_VAL(VAL_INT(yytext), INT); return INT; }
{FLOAT}     { YY_VAL(VAL_FLOAT(yytext),FLOAT); return FLOAT; }
{SEMI}      { YY_VAL(VAL_EMPTY, SEMI); return SEMI; }
{COMMA}     { YY_VAL(VAL_EMPTY, COMMA); return COMMA; }
{ASSIGNOP}  { YY_VAL(VAL_EMPTY, ASSIGNOP); return ASSIGNOP; }
{RELOP}     { return RELOP; }
{PLUS}      { YY_VAL(VAL_EMPTY, PLUS); return PLUS; }
{MINUS}     { YY_VAL(VAL_EMPTY, MINUS); return MINUS; }
{STAR}      { YY_VAL(VAL_EMPTY, STAR); return STAR; }
{DIV}       { YY_VAL(VAL_EMPTY, DIV); return DIV; }
{AND}       { YY_VAL(VAL_EMPTY, AND); return AND; }
{OR}        { YY_VAL(VAL_EMPTY, OR); return OR; }
{DOT}       { YY_VAL(VAL_EMPTY, DOT); return DOT; }
{NOT}       { YY_VAL(VAL_EMPTY, NOT); return NOT; }
{TYPE}      { YY_VAL(VAL_EMPTY, TYPE); return TYPE; }
{LP}        { YY_VAL(VAL_EMPTY, LP); return LP; }
{RP}        { YY_VAL(VAL_EMPTY, RP); return RP; }
{LB}        { YY_VAL(VAL_EMPTY, LB); return LB; }
{RB}        { YY_VAL(VAL_EMPTY, RB); return RB; }
{LC}        { YY_VAL(VAL_EMPTY, LC); return LC; }
{RC}        { YY_VAL(VAL_EMPTY, RC); return RC; }
{STRUCT}    { YY_VAL(VAL_EMPTY, STRUCT); return STRUCT; }
{RETURN}    { YY_VAL(VAL_EMPTY, RETURN); return RETURN; }
{IF}        { YY_VAL(VAL_EMPTY, IF); return IF; }
{ELSE}      { YY_VAL(VAL_EMPTY, ELSE); return ELSE; }
{WHILE}     { YY_VAL(VAL_EMPTY, WHILE); return WHILE; }
{ID}        { YY_VAL(VAL_STR(yytext),ID); return ID; }
{WHITE}     ;

%%
