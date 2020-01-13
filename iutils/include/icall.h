#ifndef ICALL_H
#define ICALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>


struct icall_func {
        void * func;
        const char * name;
        const char * desc;
    };

    int icall_shell(char * path, char * prefix);
    void icall_set_table(struct icall_func * table, int num);
    bool icall_parse_run(char * cmd, long * retl);
    char * str_next(char * str, char * out, size_t count, bool * hasMark);
    typedef long(*ifunc)(long, long, long, long, long, long, long, long, long, long);

#ifdef __cplusplus
}
#endif


#endif

