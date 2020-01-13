#ifndef IELF_H
#define IELF_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdio.h>

    typedef enum
    {
        ELF_SYMBOL_TYPE_UNKOWN=0,
        ELF_SYMBOL_TYPE_FUNC=1,
        ELF_SYMBOL_TYPE_OBJECT=2,
        ELF_SYMBOL_TYPE_FILE=3,
        ELF_SYMBOL_TYPE_NOTYPE=4,
    }elf_symbol_type_t;


typedef struct {
        char * name;
        uint64_t addr;
        uint64_t size;
        uint8_t  type;
    } elf_symbol_entry;


    elf_symbol_entry * ielf_parse_symbol(char * path, size_t * symbols_num);
    int ielf_find_symbol_table_num(FILE * fp, char * table_name);
    void ielf_symbol_print(char * buf, size_t n, elf_symbol_entry * symbol);
    elf_symbol_type_t ielf_symbol_type_to_enum(const char * type_name);
    char * ielf_symbol_type_to_string(const elf_symbol_type_t type);
    elf_symbol_entry * ielf_search_symbol(elf_symbol_entry * symbols, size_t n, char * name);


#ifdef __cplusplus
}
#endif

#endif