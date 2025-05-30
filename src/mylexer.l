%{
#include "myparser.tab.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h> // 添加ctype.h用于大小写转换

int yywrap(void) {
    return 1;
}

// 辅助函数：将字符串转换为小写
void to_lower_case(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}
%}

digit   [0-9]
letter  [a-zA-Z_]
id      {letter}({letter}|{digit})*
number  {digit}+
string  \'[^']*\'

%%

{string} {
    // 提取引号内的内容（去掉首尾引号）
    yylval.str = strdup(yytext + 1); // 跳过开头的引号
    yylval.str[yyleng - 2] = '\0';   // 去掉结尾的引号
    return STRING;
}

(?i:create)    { return CREATE; }
(?i:use)       { return USE; }
(?i:table)     { return TABLE; }
(?i:show)      { return SHOW; }
(?i:tables)    { return TABLES; }
(?i:insert)    { return INSERT; }
(?i:into)      { return INTO; }
(?i:values)    { return VALUES; }
(?i:select)    { return SELECT; }
(?i:from)      { return FROM; }
(?i:where)     { return WHERE; }
(?i:update)    { return UPDATE; }
(?i:set)       { return SET; }
(?i:delete)    { return DELETE; }
(?i:drop)      { return DROP; }
(?i:exit)      { return EXIT; }
(?i:char)      { return CHAR; }
(?i:int)       { return INT; }
(?i:databases) { return DATABASES; }
(?i:and)       { return AND; }
(?i:or)        { return OR; }

"="     { return EQ; }
"!="    { return NE; }
"<"     { return LT; }
">"     { return GT; }

"*"     { return STAR; }

{id}    { 
    yylval.str = strdup(yytext); 
    to_lower_case(yylval.str); // 转换为小写
    return ID; 
}
{number} { yylval.str = strdup(yytext); return NUMBER; }

";"     { return ';'; }
","     { return ','; }
"("     { return '('; }
")"     { return ')'; }

[ \t\r\n]+      ; // 跳过空白字符
.       { printf("Unrecognized character: %s\n", yytext); }

%%