#include "icall_ut.h"
#include "icall.h"
#include <stdbool.h>
#include "ilog.h"


struct icall_func table_t[]={
    {call_hello, "call_hello", "icall func call hello"},
    {call_hello, "hello", NULL},
    {call_t1, "call_t1", "icall func call t1"},
    {call_t5, "call_t5", "icall func call t5"},
    {call_t6, "call_t6", "icall func call t6"},
    {call_t7, "call_t7", "icall func call t7"},
};

int table_num_t=sizeof(table_t) / sizeof(struct icall_func);



int call_t1(int a1)
{
    ilog_debug("call_t1 %d", a1);
    return 1;
}

int call_t5(char a1, int a2, char a3, int a4, int a5)
{
    ilog_debug("call_t5 %d %d %d %d %d", a1, a2, a3, a4, a5);
    return 5;
}

int call_t6(char a1, int a2, char a3, int a4, int a5, char * a6)
{
    ilog_debug("call_t6 %d %d %d %d %d %s", a1, a2, a3, a4, a5, a6);
    return 6;
}

int call_t7(char a1, int a2, char a3, char * a4, int a5, char * a6, long a7)
{
    ilog_debug("call_t7 %d %d %d %s %d %s %ld", a1, a2, a3, a4, a5, a6, a7);
    return 7;
}

void call_hello(void)
{
    ilog_debug("call hello");
}

void str_next_ut(void)
{
    char * cmd=" ddd \"call_hello  \" asdf asd \"asd\" sdaf ";
    ilog_debug("cmd:%s\n", cmd);
    bool hasMark;
    char sbuf[64];
    char * str=cmd;
    while(*str != '\0') {
        str=str_next(str, sbuf, 64, &hasMark);
        ilog_debug(sbuf);
        ilog_debug(":%s\n", (hasMark ? "true" : "false"));
    }
}

void icall_table_ut_init(void)
{
    icall_set_table(table_t, table_num_t);
}

void icall_table_ut(void)
{
    long ret;
    icall_set_table(table_t, table_num_t);
    icall_parse_run("call_hello", &ret);
    ilog_debug("%ld ", ret);
    icall_parse_run("call_t1 100", &ret);
    ilog_debug("%ld ", ret);
    icall_parse_run("call_t5 101 102 58 10000 105", &ret);
    ilog_debug("%ld ", ret);
    icall_parse_run("call_t6 101 102 58 10000 105 \"hexignshi\" ", &ret);
    ilog_debug("%ld ", ret);
    icall_parse_run("call_t7 101 102 58 \"hexignshi\" 100000 \"100\" 107", &ret);
    ilog_debug("%ld ", ret);
}