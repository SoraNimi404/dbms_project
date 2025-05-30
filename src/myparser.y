%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "executor.h"

Field parsed_fields[64];
int field_count = 0;

char* insert_values[64];
int insert_value_count = 0;

char* select_fields[64];
int select_field_count = 0;


void yyerror(const char *s);
int yylex(void);

WhereClause select_where;

%}




%union {
    char* str;
}

%type <str> value

%token <str> ID NUMBER
%token CREATE DATABASE USE TABLE SHOW TABLES INSERT INTO VALUES
%token SELECT FROM WHERE UPDATE SET DELETE DROP EXIT
%token CHAR INT
%token EQ NE LT GT

%%

input:
    statements
    ;

statements:
    statements statement
  | statement
  ;

statement:
    create_database_stmt ';'
  | use_database_stmt ';'
  | create_table_stmt ';'
  | show_tables_stmt ';'
  | insert_stmt ';'
  | select_stmt ';'
  | update_stmt ';'
  | delete_stmt ';'
  | drop_table_stmt ';'
  | drop_database_stmt ';'
  | EXIT ';' {
        printf("Exit command received.\n");
        exit(0);
    }
  ;

create_database_stmt:
    CREATE DATABASE ID {
        create_database($3);
        free($3);
    }
  ;

use_database_stmt:
    USE DATABASE ID {
        use_database($3);
        free($3);
    }
  ;

create_table_stmt:
    CREATE TABLE ID '(' create_field_list ')' {
        create_table($3, parsed_fields, field_count);
        field_count = 0;
        free($3);
    }
  ;

create_field_list:
    create_field
  | create_field_list ',' create_field
  ;

create_field:
    ID CHAR '(' NUMBER ')' {
        snprintf(parsed_fields[field_count].name, sizeof(parsed_fields[field_count].name), "%s", $1);
        snprintf(parsed_fields[field_count].type, sizeof(parsed_fields[field_count].type), "CHAR(%s)", $4);
        field_count++;
        free($1); free($4);
    }
  | ID INT {
        snprintf(parsed_fields[field_count].name, sizeof(parsed_fields[field_count].name), "%s", $1);
        snprintf(parsed_fields[field_count].type, sizeof(parsed_fields[field_count].type), "INT");
        field_count++;
        free($1);
    }
  ;

show_tables_stmt:
    SHOW TABLES {
        show_tables();
    }
  ;

insert_stmt:
     INSERT INTO ID VALUES '(' value_list ')' {
        insert_into_table($3, insert_values, insert_value_count);
        // 清空缓存
        for (int i = 0; i < insert_value_count; ++i) {
            free(insert_values[i]);
        }
        insert_value_count = 0;
        free($3);
    }
  ;

value_list:
    value
  | value_list ',' value
  ;

value:
    NUMBER {
        insert_values[insert_value_count++] = strdup($1);
        free($1);
    }
  | ID {
        insert_values[insert_value_count++] = strdup($1);
        free($1);
    }
  ;

select_stmt:
     SELECT field_list FROM ID opt_where_clause {
        select_from_table($4, select_fields, select_field_count, &select_where);
        for (int i = 0; i < select_field_count; ++i) free(select_fields[i]);
        select_field_count = 0;
        free($4);
    }
  ;

field_list:
    ID {
        select_fields[select_field_count++] = strdup($1);
        free($1);
    }
  | field_list ',' ID {
        select_fields[select_field_count++] = strdup($3);
        free($3);
    }
  ;

opt_where_clause:
   /* empty */ { select_where.enabled = 0; }
  | WHERE condition_expr { select_where.enabled = 1; }
  ;

condition_expr:
 ID EQ NUMBER {
        snprintf(select_where.field, sizeof(select_where.field), "%s", $1);
        snprintf(select_where.op, sizeof(select_where.op), "=");
        snprintf(select_where.value, sizeof(select_where.value), "%s", $3);
        free($1); free($3);
    }
  | ID NE NUMBER {
        snprintf(select_where.field, sizeof(select_where.field), "%s", $1);
        snprintf(select_where.op, sizeof(select_where.op), "!=");
        snprintf(select_where.value, sizeof(select_where.value), "%s", $3);
        free($1); free($3);
    }
  | ID LT NUMBER {
        snprintf(select_where.field, sizeof(select_where.field), "%s", $1);
        snprintf(select_where.op, sizeof(select_where.op), "<");
        snprintf(select_where.value, sizeof(select_where.value), "%s", $3);
        free($1); free($3);
    }
  | ID GT NUMBER {
        snprintf(select_where.field, sizeof(select_where.field), "%s", $1);
        snprintf(select_where.op, sizeof(select_where.op), ">");
        snprintf(select_where.value, sizeof(select_where.value), "%s", $3);
        free($1); free($3);
    }
  ;

update_stmt:
    UPDATE ID SET ID EQ value opt_where_clause {
         update_table($2, $4, $6, &select_where);
        free($2); free($4); free($6);
    }
  ;

delete_stmt:
    DELETE FROM ID opt_where_clause {
        delete_from_table($3, &select_where);
        free($3);
    }
  ;

drop_table_stmt:
    DROP TABLE ID {
         drop_table($3);
        free($3);
    }
  ;

drop_database_stmt:
    DROP DATABASE ID {
        drop_database($3);
        free($3);
    }
  ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax error: %s\n", s);
}
