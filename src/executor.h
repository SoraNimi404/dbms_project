// executor.h
#ifndef EXECUTOR_H
#define EXECUTOR_H

void create_database(const char* name);
void use_database(const char* name);
void insert_into_table(const char* table_name, char** values, int value_count);
void show_databases(void);

const char* get_current_db();

typedef struct {
    char name[64];
    char type[64];  // "INT" or "CHAR(20)"
} Field;

void create_table(const char* table_name, Field* fields, int field_count);

typedef struct {
    char field[64];
    char op[4];       // "=", "!=", "<", ">"
    char value[64];   // 目标值字符串
    int enabled;      // 是否使用 WHERE
} WhereClause;

void select_from_table(const char* table, char** fields, int field_count, WhereClause* where);

void show_tables(void);

void delete_from_table(const char* table, WhereClause* where);
void update_table(const char* table, const char* set_field, const char* set_value, WhereClause* where);

void drop_table(const char* table_name);
void drop_database(const char* db_name);


#endif
