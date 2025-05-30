%{
#include "myparser.tab.h"
#include <string.h>
#include <stdlib.h>

int yywrap(void) {
    return 1;
}
%}

digit   [0-9]
letter  [a-zA-Z_]
id      {letter}({letter}|{digit})*
number  {digit}+

%%

"CREATE"        { return CREATE; }

"USE"           { return USE; }
"TABLE"         { return TABLE; }
"SHOW"          { return SHOW; }
"TABLES"        { return TABLES; }
"INSERT"        { return INSERT; }
"INTO"          { return INTO; }
"VALUES"        { return VALUES; }
"SELECT"        { return SELECT; }
"FROM"          { return FROM; }
"WHERE"         { return WHERE; }
"UPDATE"        { return UPDATE; }
"SET"           { return SET; }
"DELETE"        { return DELETE; }
"DROP"          { return DROP; }
"EXIT"          { return EXIT; }
"CHAR"          { return CHAR; }
"INT"           { return INT; }
"DATABASES"     { return DATABASES; }
"="             { return EQ; }
"!="            { return NE; }
"<"             { return LT; }
">"             { return GT; }

{id}            {
    yylval.str = strdup(yytext);
    return ID;
}

{number}        {
    yylval.str = strdup(yytext);
    return NUMBER;
}

";"             { return ';'; }
","             { return ','; }
"("             { return '('; }
")"             { return ')'; }

[ \t\r\n]+      ; // skip whitespace
.               {
    printf("Unrecognized character: %s\n", yytext);
}

%%
