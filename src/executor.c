// executor.c
#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <ctype.h> // 添加这个头文件以支持 isspace()
#include <ctype.h>
#include <string.h>
#include <windows.h>
static char current_db[256] = "";

void create_database(const char* name) {
    char path[300];
    snprintf(path, sizeof(path), "data/%s", name);
    int result = _mkdir(path);
    if (result == 0) {
        printf("Database '%s' created.\n", name);
    } else {
        perror("Failed to create database");
    }
}

void use_database(const char* name) {
    char path[300];
    snprintf(path, sizeof(path), "data/%s", name);
    if (_access(path, 0) == 0) {
        strncpy(current_db, name, sizeof(current_db));
        printf("Now using database '%s'.\n", current_db);
    } else {
        printf("Database '%s' does not exist.\n", name);
    }
}

const char* get_current_db() {
    return current_db;
}

void create_table(const char* table_name, Field* fields, int field_count) {
    if (strlen(current_db) == 0) {
        printf("No database selected. Use `USE DATABASE xxx;` first.\n");
        return;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "data/%s/%s.tab", current_db, table_name);

    FILE* fp = fopen(filepath, "w");
    if (!fp) {
        perror("Failed to create table file");
        return;
    }

    for (int i = 0; i < field_count; ++i) {
        fprintf(fp, "%s %s", fields[i].name, fields[i].type);
        if (i < field_count - 1) fprintf(fp, ",");
    }
    fprintf(fp, "\n");

    fclose(fp);
    printf("Table '%s' created in database '%s'.\n", table_name, current_db);
}

void insert_into_table(const char* table_name, char** values, int value_count) {
    if (strlen(current_db) == 0) {
        printf("No database selected.\n");
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "data/%s/%s.tab", current_db, table_name);

    FILE* fp = fopen(path, "a"); // 以追加模式打开
    if (!fp) {
        perror("Failed to open table file");
        return;
    }

    for (int i = 0; i < value_count; ++i) {
        fprintf(fp, "%s", values[i]);
        if (i < value_count - 1)
            fprintf(fp, ",");
    }
    fprintf(fp, "\n");

    fclose(fp);
    printf("Inserted 1 row into table '%s'.\n", table_name);
}

static void trim_newline(char* str) {
    char* p = str + strlen(str) - 1;
    while (p >= str && (*p == '\n' || *p == '\r' || isspace(*p))) {
        *p-- = '\0';
    }
}

static int evaluate_condition(const char* left, const char* op, const char* right) {
    int cmp = strcmp(left, right);
    if (strcmp(op, "=") == 0) return cmp == 0;
    if (strcmp(op, "!=") == 0) return cmp != 0;
    if (strcmp(op, "<") == 0) return atoi(left) < atoi(right);
    if (strcmp(op, ">") == 0) return atoi(left) > atoi(right);
    return 0;  // 默认不匹配
}

void select_from_table(const char* table_name, char** fields, int field_count, WhereClause* where) {
    if (strlen(current_db) == 0) {
        printf("No database selected.\n");
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "data/%s/%s.tab", current_db, table_name);
    FILE* fp = fopen(path, "r");
    if (!fp) {
        perror("Failed to open table");
        return;
    }

    char line[1024];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        printf("Empty table.\n");
        return;
    }

    trim_newline(line);
    char* headers[64];
    int col_count = 0;

    // 解析表头
    char* token = strtok(line, ",");
    while (token && col_count < 64) {
        headers[col_count++] = strdup(token);
        token = strtok(NULL, ",");
    }

    // 输出字段行（可过滤字段）
    printf("Rows:\n");

    while (fgets(line, sizeof(line), fp)) {
        trim_newline(line);
        char* row_fields[64];
        int i = 0;
        char* tok = strtok(line, ",");
        while (tok && i < col_count) {
            row_fields[i++] = tok;
            tok = strtok(NULL, ",");
        }

        // WHERE 条件判断
        if (where && where->enabled) {
            int where_index = -1;
            for (int j = 0; j < col_count; ++j) {
                if (strncmp(headers[j], where->field, strlen(where->field)) == 0) {
                    where_index = j;
                    break;
                }
            }
            if (where_index == -1) continue;
            if (!evaluate_condition(row_fields[where_index], where->op, where->value)) {
                continue; // 不匹配则跳过
            }
        }

        // 输出行
        for (int f = 0; f < col_count; ++f) {
            printf("%s", row_fields[f]);
            if (f < col_count - 1) printf(",");
        }
        printf("\n");
    }

    // 清理 header 内存
    for (int i = 0; i < col_count; ++i) {
        free(headers[i]);
    }

    fclose(fp);
}

void show_tables(void) {
    if (strlen(current_db) == 0) {
        printf("No database selected.\n");
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "data/%s", current_db);

    WIN32_FIND_DATA find_data;
    char pattern[512];
    snprintf(pattern, sizeof(pattern), "%s\\*.tab", path);

    HANDLE hFind = FindFirstFile(pattern, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No tables found in database '%s'.\n", current_db);
        return;
    }

    printf("Tables in database '%s':\n", current_db);
    do {
        char* dot = strrchr(find_data.cFileName, '.');
        if (dot && strcmp(dot, ".tab") == 0) {
            *dot = '\0';  // remove .tab extension
            printf("- %s\n", find_data.cFileName);
        }
    } while (FindNextFile(hFind, &find_data));

    FindClose(hFind);
}

void delete_from_table(const char* table_name, WhereClause* where) {
    if (strlen(current_db) == 0) {
        printf("No database selected.\n");
        return;
    }

    char path[512], temp_path[512];
    snprintf(path, sizeof(path), "data/%s/%s.tab", current_db, table_name);
    snprintf(temp_path, sizeof(temp_path), "data/%s/%s.tmp", current_db, table_name);

    FILE* in = fopen(path, "r");
    FILE* out = fopen(temp_path, "w");
    if (!in || !out) {
        perror("File error");
        return;
    }

    char line[1024];
    if (!fgets(line, sizeof(line), in)) {
        fclose(in); fclose(out);
        return;
    }

    // 写入表头
    fputs(line, out);
    trim_newline(line);

    char* headers[64];
    int col_count = 0;
    char* tok = strtok(line, ",");
    while (tok) {
        headers[col_count++] = strdup(tok);
        tok = strtok(NULL, ",");
    }

    while (fgets(line, sizeof(line), in)) {
        trim_newline(line);
        char* row_fields[64];
        int i = 0;
        tok = strtok(line, ",");
        while (tok) {
            row_fields[i++] = tok;
            tok = strtok(NULL, ",");
        }

        int match = 0;
        if (where && where->enabled) {
            int idx = -1;
            for (int j = 0; j < col_count; ++j) {
                if (strncmp(headers[j], where->field, strlen(where->field)) == 0) {
                    idx = j;
                    break;
                }
            }
            if (idx != -1 && evaluate_condition(row_fields[idx], where->op, where->value)) {
                match = 1;
            }
        }

        if (!match) {  // 不匹配条件则保留行
            for (int k = 0; k < col_count; ++k) {
                fprintf(out, "%s", row_fields[k]);
                if (k < col_count - 1) fprintf(out, ",");
            }
            fprintf(out, "\n");
        }
    }

    fclose(in);
    fclose(out);
    remove(path);
    rename(temp_path, path);
    printf("DELETE completed on table '%s'.\n", table_name);
}

void update_table(const char* table_name, const char* set_field, const char* set_value, WhereClause* where) {
    if (strlen(current_db) == 0) {
        printf("No database selected.\n");
        return;
    }

    char path[512], temp_path[512];
    snprintf(path, sizeof(path), "data/%s/%s.tab", current_db, table_name);
    snprintf(temp_path, sizeof(temp_path), "data/%s/%s.tmp", current_db, table_name);

    FILE* in = fopen(path, "r");
    FILE* out = fopen(temp_path, "w");
    if (!in || !out) {
        perror("File error");
        return;
    }

    char line[1024];
    if (!fgets(line, sizeof(line), in)) {
        fclose(in); fclose(out);
        return;
    }

    // 写入表头
    fputs(line, out);
    trim_newline(line);

    char* headers[64];
    int col_count = 0;
    char* tok = strtok(line, ",");
    while (tok) {
        headers[col_count++] = strdup(tok);
        tok = strtok(NULL, ",");
    }

    int set_index = -1;
    for (int i = 0; i < col_count; ++i) {
        if (strncmp(headers[i], set_field, strlen(set_field)) == 0) {
            set_index = i;
            break;
        }
    }

    while (fgets(line, sizeof(line), in)) {
        trim_newline(line);
        char* row_fields[64];
        int i = 0;
        tok = strtok(line, ",");
        while (tok) {
            row_fields[i++] = tok;
            tok = strtok(NULL, ",");
        }

        int match = 0;
        if (where && where->enabled) {
            int idx = -1;
            for (int j = 0; j < col_count; ++j) {
                if (strncmp(headers[j], where->field, strlen(where->field)) == 0) {
                    idx = j;
                    break;
                }
            }
            if (idx != -1 && evaluate_condition(row_fields[idx], where->op, where->value)) {
                match = 1;
            }
        }

        if (match && set_index != -1) {
            row_fields[set_index] = (char*)set_value;
        }

        for (int k = 0; k < col_count; ++k) {
            fprintf(out, "%s", row_fields[k]);
            if (k < col_count - 1) fprintf(out, ",");
        }
        fprintf(out, "\n");
    }

    fclose(in);
    fclose(out);
    remove(path);
    rename(temp_path, path);
    printf("UPDATE completed on table '%s'.\n", table_name);
}

void drop_table(const char* table_name) {
    if (strlen(current_db) == 0) {
        printf("No database selected.\n");
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "data/%s/%s.tab", current_db, table_name);

    if (remove(path) == 0) {
        printf("Table '%s' dropped from database '%s'.\n", table_name, current_db);
    } else {
        perror("Failed to delete table");
    }
}

void drop_database(const char* db_name) {
    char path[512];
    snprintf(path, sizeof(path), "data/%s", db_name);

    char search_path[512];
    snprintf(search_path, sizeof(search_path), "%s\\*.*", path);

    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Database '%s' not found.\n", db_name);
        return;
    }

    do {
        if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0) {
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s\\%s", path, fd.cFileName);
            remove(full_path);
        }
    } while (FindNextFile(hFind, &fd));
    FindClose(hFind);

    if (RemoveDirectory(path)) {
        printf("Database '%s' dropped.\n", db_name);
    } else {
        perror("Failed to delete database directory");
    }

    // 如果当前正在使用该数据库，则清空
    if (strcmp(current_db, db_name) == 0) {
        current_db[0] = '\0';
    }
}
void show_databases(void) {
    const char* path = "data";
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    char search_path[256];
    snprintf(search_path, sizeof(search_path), "%s\\*", path);

    hFind = FindFirstFile(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No databases found.\n");
        return;
    }

    printf("Databases:\n");

    do {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            strcmp(fd.cFileName, ".") != 0 &&
            strcmp(fd.cFileName, "..") != 0) {
            printf("- %s\n", fd.cFileName);
        }
    } while (FindNextFile(hFind, &fd));

    FindClose(hFind);
}
