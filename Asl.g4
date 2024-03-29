grammar Asl;

//////////////////////////////////////////////////
/// Parser Rules
//////////////////////////////////////////////////

// A program is a list of functions
program : function+ EOF
        ;

// A function has a name, a list of parameters and a list of statements
function
        : (FUNC ID '(' ( |parameter_decl) ')' (|return_type) declarations statements ENDFUNC)
        //: FUNC ID '(' ')' declarations statements ENDFUNC
        ;

return_type
        : ':' type
        ;

parameter_decl
        :  pdObj (',' pdObj)*
        ;

pdObj
        : ID ':' type
        ;


declarations
        : (variable_decl)*
        ;

variable_decl
        : VAR ID (',' ID)* ':' type
        ;

type    : vect
        | INT
        | BOOL
        | FLOAT
        | CHAR
        ;

vect    : ARRAY '[' INTVAL ']' OF type
        ;

basic_type: (INT|BOOL|FLOAT|CHAR)
        ;

statements
        : (statement)*
        ;

// The different types of instructions
statement
          // Assignment Not Logical
        : left_expr ASSIGN expr ';'                                 # assignStmt
          //crida void function
        //| ID '(' ( |expr (',' expr)*) ')' ';'                     # func_Stmt
          // Return
        | RETURN (|expr) ';'                                        # return
          //WHILE
        | WHILE expr DO statements ENDWHILE                         # whileStmt
          // if-then-else statement (else is optional)
        | IF expr THEN statements (ELSE statements)? ENDIF          # ifStmt
          // A function/procedure call has a list of arguments in parenthesis (possibly empty)
        | ident '(' ( |expr (',' expr)*) ')' ';'                    # procCall
          // Read a variable
        | READ left_expr ';'                                        # readStmt
          // Write an expression
        | WRITE expr ';'                                            # writeExpr
          // Write a string
        | WRITE STRING ';'                                          # writeString
        ;


// Grammar for left expressions (l-values in C++)
left_expr
        : ident ( |'[' expr ']')
        ;

// Grammar for expressions with boolean, relational and aritmetic operators
expr    :   '(' expr ')'                                 # par
        | ident '[' expr ']'                             # array_read
        //| ident '.' (FIRST | SECOND)                     # pair_read
        | ident '(' ( |expr (',' expr)*) ')'             # return_func
        | op=(NOT|PLUS|MINUS) expr                       # notplusminus
        | expr op=(MUL|DIV|MOD) expr                     # arithmetic
        | expr op=(PLUS|MINUS) expr                      # arithmetic
        | expr op=(EQUAL|NE|GT|GE|LE|LT)  expr           # relational
	      | expr op=AND  expr		                           # logic
	      | expr op=OR   expr		                           # logic
//      | (INTVAL|FLOATVAL|CHARVAL)                      # value
        | INTVAL			                                   # value
      	| FLOATVAL			                                 # value
       	| CHARVAL			                                   # value
        | BOOLVAL                                        # value
        | ident                                          # exprIdent
        ;



ident   : ID
        ;

//////////////////////////////////////////////////
/// Lexer Rules
//////////////////////////////////////////////////

ASSIGN    : '=' ;
NOT	  : 'not';
EQUAL     : '==';
LT        : '<';
GT        : '>';
NE        : '!=';
LE        : '<=';
GE        : '>=';

AND	  : 'and';
OR        : 'or' ;

PLUS      : '+' ;
MUL       : '*';
MOD       : '%';
MINUS     : '-';
DIV       : '/';
LPAR      : '(';
RPAR      : ')';
COMETA	  : '\'';

VAR       : 'var';
INT       : 'int';
BOOL      : 'bool';
FLOAT     : 'float';
CHAR      : 'char';
ARRAY     : 'array';
OF        : 'of';

IF        : 'if' ;
THEN      : 'then' ;
ELSE      : 'else' ;
ENDIF     : 'endif' ;
WHILE     : 'while';
DO        : 'do' ;
ENDWHILE  : 'endwhile';

RETURN    : 'return';
FUNC      : 'func' ;
ENDFUNC   : 'endfunc' ;
READ      : 'read' ;
WRITE     : 'write' ;
BOOLVAL   : ('true'|'false');
INTVAL    : ('0'..'9')+ ;
FLOATVAL  : ('0'..'9')+ ( | ('.' ('0'..'9')+) );
//fragment NEWLINE : '\r' '\n' | '\n' | '\r';
CHARVAL   : ('\'' ((~('\\'|'\'')) | ESC_SEQ) '\'');
ID        : ('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'_'|'0'..'9')* ;


// Strings (in quotes) with escape sequences
STRING    : '"' ( ESC_SEQ | ~('\\'|'"') )* '"' ;

fragment
ESC_SEQ   : '\\' ('b'|'t'|'n'|'f'|'r'|'"'|'\''|'\\') ;

// Comments (inline C++-style)
COMMENT   : '//' ~('\n'|'\r')* '\r'? '\n' -> skip ;

// White spaces
WS        : (' '|'\t'|'\r'|'\n')+ -> skip ;
// Alternative description
// WS        : [ \t\r\n]+ -> skip ;
