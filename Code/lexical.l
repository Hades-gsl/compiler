%{
#include <ctype.h>
#include <stdio.h>

#include "syntax.tab.h"
#include "data.h"

extern int fileno(FILE *);

int error_line = -1;

extern int has_error;
void print_error(const char *s);

int yycolumn = 1;
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

#ifdef DEBUG_LEXICAL
#define YY_VAL(v, t)\
    yylval = newMBTreeNodeData(v, _##t, yylineno);\
    printf("Line %d: %s\n", yylineno, yytext);\
    displayMBTreeNode(yylval, 4);
#else
#define YY_VAL(v, t) yylval = newMBTreeNodeData(v, _##t, yylineno);
#endif
%}
%option yylineno

SEMI        ;
COMMA       ,
ASSIGNOP    =
RELOP       >|<|>=|<=|==|!=
PLUS        \+
MINUS       -
STAR        \*
DIV         \/
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

COMMENT     ("//".*)|([/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]) 

INT         0|([1-9][0-9]*)
FLOAT       [0-9]+\.[0-9]+

OCTAL       0[0-7]*
HEX         0[xX][0-9a-fA-F]+
EXPONENT    ({FLOAT}|(\.[0-9]+)|([0-9]+\.))[eE][+-]?[0-9]+

ID          ([a-zA-Z_][a-zA-Z_0-9]*)|([a-zA-Z_0-9]*)
WHITE       [\t\f\r\v ]

%%

{COMMENT}   ;
[/][*]      { print_error("Comment not closed"); }
{SEMI}      { YY_VAL(VAL_EMPTY, SEMI); return SEMI; }
{COMMA}     { YY_VAL(VAL_EMPTY, COMMA); return COMMA; }
{ASSIGNOP}  { YY_VAL(VAL_EMPTY, ASSIGNOP); return ASSIGNOP; }
{RELOP}     { YY_VAL(val_str(yytext), RELOP);return RELOP; }
{PLUS}      { YY_VAL(VAL_EMPTY, PLUS); return PLUS; }
{MINUS}     { YY_VAL(VAL_EMPTY, MINUS); return MINUS; }
{STAR}      { YY_VAL(VAL_EMPTY, STAR); return STAR; }
{DIV}       { YY_VAL(VAL_EMPTY, DIV); return DIV; }
{AND}       { YY_VAL(VAL_EMPTY, AND); return AND; }
{OR}        { YY_VAL(VAL_EMPTY, OR); return OR; }
{DOT}       { YY_VAL(VAL_EMPTY, DOT); return DOT; }
{NOT}       { YY_VAL(VAL_EMPTY, NOT); return NOT; }
{TYPE}      { YY_VAL(val_str(yytext), TYPE); return TYPE; }
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
{INT}       { YY_VAL(VAL_INT(yytext), INT); return INT; }
{FLOAT}     { YY_VAL(VAL_FLOAT(yytext), FLOAT); return FLOAT; }
{OCTAL}     { YY_VAL(VAL_INT(yytext), INT); print_error("Octal number is not supported"); return INT; }
{HEX}       { YY_VAL(VAL_INT(yytext), INT); print_error("Hexadecimal number is not supported"); return INT; }
{EXPONENT}  { YY_VAL(VAL_FLOAT(yytext), FLOAT); print_error("Exponential number is not supported"); return FLOAT; }
{ID}        {   
                if (isdigit(yytext[0])) {
                    print_error("Identifier cannot start with a digit");
                }
                YY_VAL(val_str(yytext), ID); 
                return ID; 
            }
\n          { yycolumn = 1; }
{WHITE}     ;
.           { print_error("Unknown character"); }

%%

void print_error(const char *s) {
    printf("Error type A at Line %d: %s: %s\n", yylineno, s, yytext);
    has_error = 1;
    error_line = yylineno;
}