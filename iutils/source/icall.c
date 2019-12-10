#include "icall.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "ilog.h"
#include "itypes.h"
#include "ielf.h"
#include "iutil.h"


#define ICALL_STR_MAX	256
#define ICALL_PARAM_MAX 10
#define ICALL_PARAM_TYPE_NONE	0
#define ICALL_PARAM_TYPE_INT	1
#define ICALL_PARAM_TYPE_STRING	2

size_t symbols_num = 0;
elf_symbol_entry * symbols = NULL;
static char shell_prefix[MAX_NAME] = "iutil:/crun# ";
static struct icall_func * func_table = NULL;
static int func_table_num = 0;



int icall_hello(char * s)
{
    printf("[hello] %s\n", s);
    return strlen(s);
}

long icall_crun(long addr, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7, long arg8)
{
    ifunc fun_run = (ifunc)addr;
    return fun_run(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, 0);
}

long icall_cvalue(long addr, long arg0, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6, long arg7, long arg8)
{
    ilog_array((void *)addr, arg0);
    return arg0;
}



struct icall_func icall_inner_functions[] = {
    { icall_hello, "hello", "call icall_hello" },
    { icall_crun, "crun", "call crun to run address functions" },
    { icall_cvalue, "cvalue", "call cvalue to dump object" },
};

int d(long addr, int len)
{
    ilog_array((void *)addr, len);
    return len;
}

int m(long addr, int value)
{
    char * ps = NULL;
    if (addr == 0) {
        return -1;
    }
    ps = (char *)addr;
    *ps = (char)value;
    return *ps;
}

int m16(long addr, int value)
{
    uint16_t * ps = NULL;
    if (addr == 0) {
        return -1;
    }
    ps = (uint16_t *)addr;
    *ps = (uint16_t)value;
    return *ps;
}

int m32(long addr, int value)
{
    uint32_t * ps = NULL;
    if (addr == 0) {
        return -1;
    }
    ps = (uint32_t *)addr;
    *ps = (uint32_t)value;
    return *ps;
}



void icall_set_table(struct icall_func * table, int num)
{
    func_table = table;
    func_table_num = num;
}

bool icall_parse_run(char * cmd, long * ret_out)
{
    char name[ICALL_STR_MAX];
    char tempArg[ICALL_STR_MAX];
    char argsStr[ICALL_PARAM_MAX][ICALL_STR_MAX];
    long argsLong[ICALL_PARAM_MAX];
    int argsType[ICALL_PARAM_MAX];
    ifunc fun_run;
    int paramsNum = 0;
    bool hasMask = false;
    int i = 0;
    long ret = 0;

    if (!cmd) {
        return false;
    }
    char * cmd_orignal = cmd;
    cmd = str_next(cmd, name, ICALL_STR_MAX, NULL);
    if (name == NULL || *name == '\0') {
        return false;
    }

    for (i = 0; i < ICALL_PARAM_MAX; i++) {
        argsLong[i] = 0x00;
        argsStr[i][0] = '\0';
        argsType[i] = ICALL_PARAM_TYPE_NONE;
    }

    i = 0;
    while (cmd != NULL && *cmd != '\0' && i < ICALL_PARAM_MAX) {
        cmd = str_next(cmd, tempArg, ICALL_STR_MAX, &hasMask);
        if (*tempArg != '\0') {
            if (hasMask) {
                argsType[i] = ICALL_PARAM_TYPE_STRING;
                strcpy(argsStr[i], tempArg);
                argsLong[i] = (long)argsStr[i];
            } else {
                if (isdigit(tempArg[0])) {
                    argsType[i] = ICALL_PARAM_TYPE_INT;
                    argsLong[i] = strtol(tempArg, NULL, 0);
                } else {
                    elf_symbol_entry * entry = ielf_search_symbol(symbols, symbols_num, tempArg);
                    if (entry != NULL && entry->type == ELF_SYMBOL_TYPE_OBJECT) {
                        argsType[i] = ICALL_PARAM_TYPE_INT;
                        argsLong[i] = entry->addr;
                        // #TODO 地址转换为值
                    } else {
                        argsType[i] = ICALL_PARAM_TYPE_INT;
                        argsLong[i] = 0;
                    }
                }

            }
            i++;
        }
    }

    for (i = 0; i < ARRAY_SIZE(icall_inner_functions); i++) {
        if (strcmp(name, icall_inner_functions[i].name) == 0) {
            fun_run = icall_inner_functions[i].func;
            ret = fun_run(argsLong[0], argsLong[1], argsLong[2], argsLong[3], argsLong[4],
                          argsLong[5], argsLong[6], argsLong[7], argsLong[8], argsLong[9]);
            if (ret_out) {
                *ret_out = ret;
            }
            return true;
        }
    }

    for (i = 0; i < func_table_num; i++) {
        if (strcmp(name, func_table[i].name) == 0) {
            fun_run = func_table[i].func;
            ret = fun_run(argsLong[0], argsLong[1], argsLong[2], argsLong[3], argsLong[4],
                          argsLong[5], argsLong[6], argsLong[7], argsLong[8], argsLong[9]);
            if (ret_out) {
                *ret_out = ret;
            }
            return true;
        }
    }

    int s_ret = system(cmd_orignal);
    *ret_out = s_ret;
    if (s_ret == 0) {
        return true;
    } else {
        return false;
    }
    return true;
}



/*
 *	获取str中的下一个字符子串
 *	使用引号"分割的为一个子串
 *	hasMark true表示用"分割， false表示不用"分割
 */
char * str_next(char * str, char * out, size_t count, bool * hasMark)
{
    if (!str || !out) {
        return NULL;
    }

    while (*str == ' ' || *str == ',' || *str == '\t' || *str == '\n') {
        str++;
    }

    if (*str == '\"') {
        if (hasMark) {
            *hasMark = true;
        }
        str++;
        while (count > 0 && *str != '\"' && *str != '\n' && *str != '\0') {
            *out++ = *str++;
        }
        if (*str == '\"') {
            str++;
        }
    } else {
        if (hasMark) {
            *hasMark = false;
        }
        while (count > 0 && *str != ' ' && *str != ',' && *str != '\n' && *str != '\t' && *str != '\0') {
            *out++ = *str++;
        }
    }

    if (count > 0) {
        *out = '\0';
    }

    return str;
}


#define IS_SUBSTR_EQUAL(s1, s2) strncmp(s1, s2, min(strlen(s1), strlen(s2)))==0?true:false


char* dupstr(char *s)
{
    char *r;

    r = malloc(strlen(s) + 1);
    strcpy(r, s);
    return (r);
}

/*
* Generator function for command completion. STATE lets us know whether
* to start from scratch; without any state (i.e. STATE == 0), then we
* start at the top of the list.
*/
char* command_generator(const char *text, int state)
{
    static int list_index, len;
    char *name;

    /*
    * If this is a new word to complete, initialize now. This includes
    * saving the length of TEXT for efficiency, and initializing the index
    * variable to 0.
    */
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    /* Return the next name which partially matches from the command list. */
    while (list_index < symbols_num - 1) {
        list_index++;
        if (strstr(symbols[list_index].name, text) != NULL) {
            return dupstr(symbols[list_index].name);
        }
    }

    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}

char** fileman_completion(const char *text, int start, int end)
{
    char **matches;

    matches = (char **)NULL;

    if (start == 0)
        matches = rl_completion_matches(text, command_generator);

    return (matches);
}

int icall_shell(char * path, char * prefix)
{
    //char line[MAX_LINE+1];
    char * line = NULL;
    char cmd_tmp[MAX_LINE + 1];


    ilog_info("ExePath: %s", path);

    char cmd_readelf[MAX_LINE];
    char * sym_path = "/tmp/iutil.sym";
    snprintf(cmd_readelf, MAX_LINE, "readelf -s %s > %s", path, sym_path);
    system("mkdir -p /tmp");
    system(cmd_readelf);

    symbols = ielf_parse_symbol(sym_path, &symbols_num);
    ilog_info("Symbols Num: %d", symbols_num);
    if (symbols == NULL) {
        ilog_error("[Fail] load symbol");
        return 0;
    }

    rl_attempted_completion_function = fileman_completion;

    do {
        if (prefix == NULL) {
            line = readline(shell_prefix);
        } else {
            line = readline(prefix);
        }

        if (line == NULL) {
            continue;
        }

        if (strlen(line) == 0) {
            free(line);
            continue;
        }

        if (IS_SUBSTR_EQUAL(line, "quit") || IS_SUBSTR_EQUAL(line, "exit")) {
            free(line);
            break;
        } else {
            if (IS_SUBSTR_EQUAL(line, "\n")) {
                free(line);
                continue;
            }

            char name[MAX_NAME];
            char * cmd = line;
            bool bret = false;
            long call_ret = 0;

            //ilogm(ILOG_LEVEL_MASK, "InputCmd: %s", line);
            uint64_t time_s;
            uint64_t time_e;
            cmd = str_next(cmd, name, ICALL_STR_MAX, NULL);
            elf_symbol_entry * entry = ielf_search_symbol(symbols, symbols_num, name);
            if (entry != NULL) {
                ielf_symbol_print(cmd_tmp, MAX_LINE, entry);
                ilog_mask("%s", cmd_tmp);
                if (entry->type == ELF_SYMBOL_TYPE_FUNC) {
                    snprintf(cmd_tmp, MAX_LINE, "crun 0x%lx %s", entry->addr, cmd);
                } else if (entry->type == ELF_SYMBOL_TYPE_OBJECT) {
                    snprintf(cmd_tmp, MAX_LINE, "cvalue 0x%lx 0x%lx", entry->addr, entry->size);
                }
                time_s = iutil_time_ns();
                bret = icall_parse_run(cmd_tmp, &call_ret);
            } else {
                time_s = iutil_time_ns();
                bret = icall_parse_run(line, &call_ret);
            }
            time_e = iutil_time_ns();
            //ilog_mask("[%s] value = %lld(0x%lx) T=%lldms %lldns", (bret) ? " OK " : "False", call_ret, call_ret, (time_e - time_s) / 1000000, (time_e - time_s) % 1000000);
            ilog_mask("value = %lld(0x%lx) T=%lldms %lldns",  call_ret, call_ret, (time_e - time_s) / 1000000, (time_e - time_s) % 1000000);
            add_history(line);
        }

        if (line != NULL) {
            free(line);
        }


    } while (true);

}

