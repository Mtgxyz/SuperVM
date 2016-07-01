#pragma once

#include <stdio.h>

#define TOK_INVALID  -1
#define TOK_EOF       0
#define TOK_INT       1
#define TOK_HEX       2
#define TOK_LABEL     3
#define TOK_MNEMONIC  4
#define TOK_MOD       5
#define TOK_NEWLINE   6
#define TOK_CHAR      7
#define TOK_REFERENCE 8
#define TOK_BIN       9


extern int yylineno;
extern char *yytext;

void yyrestart (FILE *input_file  );

/** Get the current line number.
 * 
 */
int yyget_lineno  (void);

/** Get the input stream.
 * 
 */
FILE *yyget_in  (void);

/** Get the output stream.
 * 
 */
FILE *yyget_out  (void);

/** Get the current token.
 * 
 */
char *yyget_text  (void);

/** Set the current line number.
 * @param _line_number line number
 * 
 */
void yyset_lineno (int  _line_number );

/** Set the input stream. This does not discard the current
 * input buffer.
 * @param _in_str A readable stream.
 * 
 * @see yy_switch_to_buffer
 */
void yyset_in (FILE *  _in_str );

void yyset_out (FILE *  _out_str );

int yyget_debug  (void);

void yyset_debug (int  _bdebug );

int yylex();