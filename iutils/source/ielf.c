#include "ielf.h"
#include "itypes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


elf_symbol_entry * ielf_parse_symbol(char * path, size_t * symbols_num)
{
    FILE * fp=NULL;
    char line[MAX_LINE];
    int num=0;
    elf_symbol_entry * symbols;
    int symbols_index=0;
    int i=0;
    int sy_num;
    uint64_t sy_value;
    uint64_t sy_size;
    char sy_type[MAX_NAME];
    char sy_bind[MAX_NAME];
    char sy_vis[MAX_NAME];
    char sy_ndx[MAX_NAME];
    char sy_name[MAX_NAME];

    if(path == NULL || symbols_num == NULL) {
        return NULL;
    }

    fp=fopen(path, "r");
    if(fp == NULL) {
        fprintf(stderr, "[Fail] open symbol file, path: %s\n", path);
        return NULL;
    }

    num=ielf_find_symbol_table_num(fp, ".symtab");
    if(num > 0) {
        symbols=(elf_symbol_entry *)malloc(sizeof(elf_symbol_entry)* num);
        if(symbols == NULL) {
            fprintf(stderr, "[Fail] malloc symbols, symbol_num: %d\n", num);
            fclose(fp);
            return NULL;
        }
        fgets(line, MAX_LINE, fp);
        for(i=0; i < num; i++) {
            if(fgets(line, MAX_LINE, fp) == NULL) {
                fprintf(stderr, "[Fail] symbol text line can read over, read: %d all: %d\n", i, num);
                *symbols_num=symbols_index;
                return symbols;
            } else {
                if(sscanf(line, "%d: %lx %ld %s %s %s %s %s", &sy_num, &sy_value, &sy_size, sy_type, sy_bind, sy_vis, sy_ndx, sy_name) == 8) {
                    int name_len=strlen(sy_name);
                    if(name_len>0) {
                        symbols[symbols_index].name=malloc(name_len + 1);
						strcpy(symbols[symbols_index].name, sy_name);
                        symbols[symbols_index].addr=sy_value;
                        symbols[symbols_index].size=sy_size;
                        symbols[symbols_index].type=ielf_symbol_type_to_enum(sy_type);
                        symbols_index++;
                    }
                }
            }
        }

        *symbols_num=symbols_index;
        return symbols;
    }

    return NULL;
}

static char * elf_symbol_type_strs[]={
    "UNKOWN",
    "FUNC",
    "OBJECT",
    "FILE",
    "NOTYPE",
};


elf_symbol_type_t ielf_symbol_type_to_enum(const char * type_name)
{
    int i=0;
    if(type_name == NULL) {
        return ELF_SYMBOL_TYPE_UNKOWN;
    }

    for(i=0; i < ARRAY_SIZE(elf_symbol_type_strs); i++) {
        if(strcmp(type_name, elf_symbol_type_strs[i]) == 0) {
            return i;
        }
    }

    return ELF_SYMBOL_TYPE_UNKOWN;
}

char * ielf_symbol_type_to_string(const elf_symbol_type_t type)
{
    if(type < 0 || type >= ARRAY_SIZE(elf_symbol_type_strs)) {
        return NULL;
    }
    return elf_symbol_type_strs[type];
}

int ielf_find_symbol_table_num(FILE * fp, char * table_name)
{
    char line[MAX_LINE + 1];
    char table_line[MAX_LINE + 1];
    int table_line_len=0;
    int symbol_num=-1;

    if(fp == NULL || table_name == NULL) {
        return -1;
    }

    snprintf(table_line, MAX_LINE, "Symbol table '%s' contains", table_name);
    table_line_len=strlen(table_line);

    while(fgets(line, MAX_LINE, fp) != NULL) {
        if(strncmp(line, table_line, table_line_len) == 0) {
            sscanf(line + table_line_len, " %d entries:", &symbol_num);
            return symbol_num;
        }
    }
    return -1;
}

void ielf_symbol_print(char * buf, size_t n, elf_symbol_entry * symbol)
{
    snprintf(buf, n, "0x%016lx %8ld   %8s   %s", symbol->addr, symbol->size, ielf_symbol_type_to_string(symbol->type), symbol->name);
}


elf_symbol_entry * ielf_search_symbol(elf_symbol_entry * symbols, size_t n, char * name)
{
    if(name == NULL || symbols == NULL || n <= 0) {
        return NULL;
    }

    size_t i=0;
    for(i=0; i < n; i++) {
        if(strcmp(symbols[i].name, name) == 0) {
            return &symbols[i];
        }
    }

    return NULL;
}

