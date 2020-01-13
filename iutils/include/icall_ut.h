#ifndef ICALL_TEST_H
#define ICALL_TEST_H

void call_hello(void);
int call_t1(int a1);
int call_t5(char a1, int a2, char a3, int a4, int a5);
int call_t6(char a1, int a2, char a3, int a4, int a5, char * a6);
int call_t7(char a1, int a2, char a3, char * a4, int a5, char * a6, long a7);

void str_next_ut(void);
void icall_table_ut(void);
void icall_table_ut_init(void);
#endif