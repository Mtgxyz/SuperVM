%{
#include "tokens.h"
#define RETURN(x,y) /* fprintf(stderr, "{" #x "}" y ); */ return x
%}
comment			;[^\n]*
whitespace	[ \t]
newline     [\n]
digit				[0-9]
hexdigit    [0-9a-fA-F]
intnum			{digit}+
hexnum			"0x"{hexdigit}+
mnemonic    [a-zA-Z]+
labelname   [a-zA-Z0-9_]+
label       {labelname}":"
mod         \[[^\]]+\]
character   "'"."'"
ref         @{labelname}
%%
{comment}				;
{hexnum} 				RETURN(TOK_HEX,);
{intnum} 				RETURN(TOK_INT,);
{whitespace}		;
{mnemonic}      RETURN(TOK_MNEMONIC,);
{label}         RETURN(TOK_LABEL,); 
{mod}           RETURN(TOK_MOD,);
{character}     RETURN(TOK_CHAR, );
{ref}           RETURN(TOK_REFERENCE, );
{newline}       RETURN(TOK_NEWLINE, "\n");
.               RETURN(TOK_INVALID,);
%%
// This is the end of the file
int yywrap() { return 1; }