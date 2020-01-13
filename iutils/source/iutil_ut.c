#include "ilog.h"
#include "iassert.h"
#include "ielf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpc_icall.h"
#include "iutil.h"




void ielf_ut()
{
    size_t n;
    size_t i;
    char line[MAX_LINE+1];

    elf_symbol_entry * symbols = ielf_parse_symbol("./data/main.sym", &n);
    for (i = 0; i < n; i++) {
        ielf_symbol_print(line, MAX_LINE, &symbols[i]);
        ilog_debug("%4ld: %s", i, line);
    }
}

void iutil_num()
{
    char str[256];
    iutil_num_format(1024*1024*256, str);
    ilog_info(str);

    iutil_num_format(100000000, str);
    ilog_info(str);

    int ret;
    ret = strcmp("aabb", "aabb");
    ilog_info("%d", ret);
    ret = strncmp("aabb", "aa", 2);
    ilog_info("%d", ret);

}


extern void ilog_ut();

void iutil_ut(int argc, char *argv[])
{
    system("pwd");
    ilog_init();
    ilog_parser_init();

    char * buf = iutil_trace_format();
    printf("%s", buf);
    if (buf != NULL){
        free(buf);
    }
    

    char arr[100] = "hexingshi";
    buf = iutil_array_format(arr, 100, 16);
    printf("%s", buf);
    if (buf != NULL){
        free(buf);
    }
    

    char * kvs = "ak=av,bk=bv ,ck=cv, dk=dv ";
    printf("%s\n", kvs);
    char key[MAX_NAME];
    char value[MAX_NAME];
    while ((kvs = iutil_kv_iterater(kvs, key, value)) != NULL){
        printf("%s=%s\n", key, value);
    }

    
    ilog_ut();
    //icall_table_ut_init();

    


    //icall_shell(argv[0], NULL);
}



int iaccess_ut()
{
    EX_INFO(2 > 1);
    EX_INFO(2 < 1);

    EX_INFO(2 > 1, "msg");
    EX_INFO(2 < 1, "msg");

    EX_INFO(2 > 1, "msg %d %s", 10, "aa");
    EX_INFO(2 < 1, "msg %d %s", 10, "aa");



    EX_CHECK(2 > 1);
    EX_CHECK(2 < 1);

    EX_CHECK(2 > 1, "msg");
    EX_CHECK(2 < 1, "msg");

    EX_CHECK(2 > 1, "msg %d %s", 10, "aa");
    EX_CHECK(2 < 1, "msg %d %s", 10, "aa");

    EX_ASSERT(2 > 1);
    EX_ASSERT(2 > 1, "msg");
    EX_ASSERT(2 > 1, "msg %d %s", 10, "aa");

    EX_RET(2 > 1, true);
    EX_RET(2 > 1, true, "msg");
    EX_RET(2 > 1, true, "msg %d %s", 10, "aa");

    //EX_ASSERT(2 < 1, "msg %d %s", 10, "aa");
    EX_RET(2 < 1, true, "msg %d %s", 10, "aa");

    return false;

}







