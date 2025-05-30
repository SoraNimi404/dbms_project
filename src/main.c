// main.c
#include <stdio.h>
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include "executor.h"

int yyparse(void);

int main() {
    SetConsoleOutputCP(CP_UTF8);
    

    printf("Welcome to the mini DBMS (UTF-8 supported).\n");

    while (1) {
        printf("\nSQL> ");
        fflush(stdout);
        yyparse();  // 分析并解释执行
    }

    return 0;
}
