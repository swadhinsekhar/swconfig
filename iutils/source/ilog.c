#include "ilog.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <time.h>
#include "iutil.h"

const char * ilog_level_name(int level);
uint32_t ilog_outstream_puts(ilog_outstream_t * os, const char * buf);
uint32_t ilog_parser_handler(const char * str);


#define ILOG_BUF_MAX (2*MAX_LINE)			/* 输出缓冲区最大长度 */
ilog_context_t * ilog_cx = NULL;			/* 默认log上下文 */

static ilog_context_t cx_null = {
    "default_null",
    { true, true, true, true, true, true, true, },
    {
        { ILOG_OUTSTREAM_TYPE_NULL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_NULL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_NULL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_NULL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_NULL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_NULL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_NULL, "", NULL },
    },
};
static ilog_context_t cx_std = {
    "default_std",
    { true, true, true, true, true, true, true, },
    {
        { ILOG_OUTSTREAM_TYPE_STDOUT, "", NULL },
        { ILOG_OUTSTREAM_TYPE_STDOUT, "", NULL },
        { ILOG_OUTSTREAM_TYPE_STDOUT, "", NULL },
        { ILOG_OUTSTREAM_TYPE_STDOUT, "", NULL },
        { ILOG_OUTSTREAM_TYPE_STDOUT, "", NULL },
        { ILOG_OUTSTREAM_TYPE_STDOUT, "", NULL },
        { ILOG_OUTSTREAM_TYPE_STDOUT, "", NULL },
    },
};
static ilog_context_t cx_dcall = {
    "default_dcall",
    { true, true, true, true, true, true, true, },
    {
        { ILOG_OUTSTREAM_TYPE_DCALL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_DCALL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_DCALL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_DCALL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_DCALL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_DCALL, "", NULL },
        { ILOG_OUTSTREAM_TYPE_DCALL, "", NULL },
    },
};
static ilog_context_t cx_file = {
    "default_file",
    { true, true, true, true, true, true, true, },
    {
        { ILOG_OUTSTREAM_TYPE_FILE, "./ilog.txt", NULL },
        { ILOG_OUTSTREAM_TYPE_FILE, "./ilog.txt", NULL },
        { ILOG_OUTSTREAM_TYPE_FILE, "./ilog.txt", NULL },
        { ILOG_OUTSTREAM_TYPE_FILE, "./ilog.txt", NULL },
        { ILOG_OUTSTREAM_TYPE_FILE, "./ilog.txt", NULL },
        { ILOG_OUTSTREAM_TYPE_FILE, "./ilog.txt", NULL },
        { ILOG_OUTSTREAM_TYPE_FILE, "./ilog.txt", NULL },
    },
};
static ilog_context_t cx_socket_tcp = {
    "default_socket_tcp",
    { true, true, true, true, true, true, true, },
    {
        { ILOG_OUTSTREAM_TYPE_SOCKET_TCP, "127.0.0.1:9100", NULL },
        { ILOG_OUTSTREAM_TYPE_SOCKET_TCP, "127.0.0.1:9100", NULL },
        { ILOG_OUTSTREAM_TYPE_SOCKET_TCP, "127.0.0.1:9100", NULL },
        { ILOG_OUTSTREAM_TYPE_SOCKET_TCP, "127.0.0.1:9100", NULL },
        { ILOG_OUTSTREAM_TYPE_SOCKET_TCP, "127.0.0.1:9100", NULL },
        { ILOG_OUTSTREAM_TYPE_SOCKET_TCP, "127.0.0.1:9100", NULL },
        { ILOG_OUTSTREAM_TYPE_SOCKET_TCP, "127.0.0.1:9100", NULL },
    },
};


void ilog_init(void)
{
    ilog_default_cx_set(2);
}

bool ilog_context_init(ilog_context_t * cx)
{
    return true;
}


void ilog_default_cx_set(uint8_t cx_index)
{
    static bool is_inited = false;
    if (is_inited == false){
        ilog_context_init(&cx_null);
        ilog_context_init(&cx_std);
        ilog_context_init(&cx_dcall);
        ilog_context_init(&cx_file);
        ilog_context_init(&cx_socket_tcp);
        is_inited = true;
    }

    switch (cx_index)
    {
    case 0: ilog_cx = &cx_null; break;
    case 1: ilog_cx = &cx_std; break;
    case 2: ilog_cx = &cx_dcall; break;
    case 3: ilog_cx = &cx_file; break;
    case 4: ilog_cx = &cx_socket_tcp; break;
    default:
        break;
    }
}


/*
 *	支持富文本格式化输出
 *	time_us			level	cx_name		tag	  file_path 	func	line	msg_len | msg
 *  xxxxxxxx
 */
uint32_t ilog_raw_format(char * raw_str, uint32_t buf_max, uint64_t time_us,
    ilog_level_t level, const char * cx_name, const char * tag, const char * file_path,
    const char * func, const uint32_t line, const char * log_str)
{
    int out_num = 0;
    if (raw_str == NULL){
        return 0;
    }

    raw_str[0] = '\0';
    out_num += snprintf(raw_str + out_num, buf_max - out_num, "%"PRIu64, time_us);
    out_num += snprintf(raw_str + out_num, buf_max - out_num, " %d", level);
    out_num += snprintf(raw_str + out_num, buf_max - out_num, " %s", cx_name);
    out_num += snprintf(raw_str + out_num, buf_max - out_num, " %s", tag);
    out_num += snprintf(raw_str + out_num, buf_max - out_num, " %s", file_path);
    out_num += snprintf(raw_str + out_num, buf_max - out_num, " %s", func);
    out_num += snprintf(raw_str + out_num, buf_max - out_num, " %d", line);
    out_num += snprintf(raw_str + out_num, buf_max - out_num, " %d", (int)strlen(log_str));
    out_num += snprintf(raw_str + out_num, buf_max - out_num, " | ");
    out_num += snprintf(raw_str + out_num, buf_max - out_num, "%s", log_str);
    return out_num;
}


uint32_t ilog_raw_extract(const char * raw_str, uint64_t * time_us, ilog_level_t * level, char * cx_name,
    char * tag, char * file_path, char * func, uint32_t * line, uint32_t * log_len, char * log_str)
{
    if (raw_str != NULL){
        int ilevel = 0;
        int elem = sscanf(raw_str, "%ld %d %s %s %s %s %d %d | ", time_us, &ilevel, cx_name,
            tag, file_path, func, line, log_len);
        if (elem == 8){
            char * msg_log = strstr(raw_str, " | ");
            if (msg_log != NULL){
                *level = ilevel;
                msg_log += 3;
                memcpy(log_str, msg_log, *log_len);
                log_str[*log_len] = '\0';
                return msg_log - raw_str + *log_len;
            }
        }
    }
    *log_len = 0;
    return 0;
}


uint32_t ilog_puts(ilog_context_t * cx, ilog_level_t level, const char * tag,
    const char * file_path, const char * func, const uint32_t line, const char * log_str)
{
    /* 未指定cx时，使用全局默认cx */
    if (cx == NULL){
        cx = ilog_cx;
    }

    if (cx->level_enable[level] == true){
        char * raw_str = (char *)malloc(ILOG_BUF_MAX + 1);
        if (raw_str != NULL){
            ilog_raw_format(raw_str, ILOG_BUF_MAX, iutil_time_us(), level, cx->name, tag, file_path, func, line, log_str);
            uint32_t out_num = ilog_outstream_puts(&(cx->level_os[level]), raw_str);
            free(raw_str);
            return out_num;
        }
    } 
        
    return 0;
}



uint32_t ilog_printf(ilog_context_t * cx, ilog_level_t level, const char * tag,
    const char * file_path, const char * func, const uint32_t line, const char * fmt, ...)
{
    char * log_str = (char *)malloc(ILOG_BUF_MAX + 1);
    if (log_str != NULL){
		va_list args;
		va_start(args, fmt);
		vsnprintf(log_str, ILOG_BUF_MAX, fmt, args);
		//printf(fmt, args);
		//printf("\n");
		va_end(args);
		//return 0;

        uint32_t out_num = ilog_puts(cx, level, tag, file_path, func, line, log_str);
        free(log_str);
        return out_num;
    }
    
    return 0;
}


uint32_t ilog_outstream_puts(ilog_outstream_t * os, const char * buf)
{
    uint32_t out_num = 0;
    if (os == NULL || buf == NULL){
        return 0;
    }

    switch (os->type)
    {
    case ILOG_OUTSTREAM_TYPE_NULL		  :
        break;
    case ILOG_OUTSTREAM_TYPE_STDOUT		  :
        out_num = printf("%s%s", buf, "\n");
        break;
    case ILOG_OUTSTREAM_TYPE_DCALL		  :
        out_num = ilog_parser_handler(buf);
        break;
    case ILOG_OUTSTREAM_TYPE_FILE		  :
        break;
    case ILOG_OUTSTREAM_TYPE_SOCKET_TCP	  :
        break;
    case ILOG_OUTSTREAM_TYPE_SOCKET_UDP	  :
        break;
    }

    return out_num;
}



/*
* tarce 打印调用栈
* 打印函数名称需添加编译选项 链接添加 -rdynamic
*/
void ilog_trace(void)
{
    char * str = iutil_trace_format();
    ilog_info(str);
    if (str){
        free(str);
    }
}


/*
*	格式化输出数组
*/
void ilog_array(void * ds, int num)
{
    char * str = iutil_array_format(ds, num, 16);
    ilog_debug(str);
    if (str){
        free(str);
    }
}

































/* -------------------------[      ilog_parser      ]------------------------- */

#define TTY_COLOR_NORAML		"\033[0m"	/* noraml  关闭所有属性	*/
#define TTY_COLOR_BLACK			"\033[30m"	/* black	30 40 黑色	*/
#define TTY_COLOR_RED			"\033[31m"	/* red		31 41 红色	*/
#define TTY_COLOR_GREEN			"\033[32m"	/* green	32 42 绿色	*/
#define TTY_COLOR_YELLOW		"\033[33m"	/* yellow	33 43 黄色	*/
#define TTY_COLOR_BLUE			"\033[34m"	/* blue		34 44 蓝色	*/
#define TTY_COLOR_MAGENTA		"\033[35m"	/* magenta	35 45 洋红	*/
#define TTY_COLOR_CYAN			"\033[36m"	/* cyan		36 46 青色	*/
#define TTY_COLOR_GRAY			"\033[37m"	/* white	37 47 白色	*/


typedef struct ilog_msg_s {
    uint64_t time_us;
    ilog_level_t level;
    char cx_name[MAX_NAME];
    char tag[MAX_NAME];
    char file_path[MAX_PATH];
    char func_name[MAX_NAME];
    uint32_t line_num;
    uint32_t log_len;
    char log[MAX_LINE];
}ilog_msg_t;


/* 显示控制 */
typedef struct ilog_parser_vctrl_s {
    bool is_enable;
    bool is_show_color;
    int show_time_mode;			/* 0:关闭  1:时间精度微秒  2:asic时间精度us 3:asic时间精度ms 4:只有时间没有日期精度us */
    bool is_show_level;
    bool is_show_cx_name;
    bool is_show_tag;
    int  show_file_mode;		/* 0:关闭  1:文件路径  2:文件名称 */
    bool is_show_func;
    bool is_show_line;
}ilog_parser_vctrl_t;


ilog_parser_vctrl_t levels_vctrl[ILOG_LEVEL_NUM];
char * parser_cfg = "all_enable=1, all_color=1, all_time=0, all_level=0,\
                    all_cx=0, all_tag=0, all_file=0, all_func=0, all_line=0, \
                    error_time=0, error_level=1, error_tag=0, error_file=2, error_func=1, error_line=1, \
                    fatal_time=0, fatal_level=1, fatal_tag=0, fatal_file=2, fatal_func=1, fatal_line=1, ";


uint32_t ilog_parser_init(void)
{
    ilog_parser_config(parser_cfg);
	return 0;
}

bool ilog_parser_vctrl_config(ilog_parser_vctrl_t * vctrl, char * key, char * value)
{
    if (vctrl == NULL || key == NULL || value == NULL){
        return false;
    }

    int v = atoi(value);
    if (strcasecmp(key, "enable") == 0){
        vctrl->is_enable = v;
    } else if (strcasecmp(key, "color") == 0){
        vctrl->is_show_color = v;
    } else if (strcasecmp(key, "time") == 0){
        vctrl->show_time_mode = v;
    } else if (strcasecmp(key, "level") == 0){
        vctrl->is_show_level = v;
    } else if (strcasecmp(key, "cx") == 0){
        vctrl->is_show_cx_name = v;
    } else if (strcasecmp(key, "tag") == 0){
        vctrl->is_show_tag = v;
    } else if (strcasecmp(key, "file") == 0){
        vctrl->show_file_mode = v;
    } else if (strcasecmp(key, "func") == 0){
        vctrl->is_show_func = v;
    } else if (strcasecmp(key, "line") == 0){
        vctrl->is_show_line = v;
    } else {
        return false;
    }
    return true;
}

uint32_t ilog_parser_config_format(char * str, uint32_t len_max)
{
    if (str == NULL){
        return 0;
    }

    int i = 0;
    uint32_t out_num = 0;
    for (i = ILOG_LEVEL_NULL; i < ILOG_LEVEL_NUM; i++){
        const char * lv = ilog_level_name(i);
        ilog_parser_vctrl_t * vc = &levels_vctrl[i];
        out_num += snprintf(str + out_num, len_max - out_num, "%s_%s=%d, %s_%s=%d, %s_%s=%d, %s_%s=%d, %s_%s=%d, %s_%s=%d, %s_%s=%d, %s_%s=%d, %s_%s=%d\n",
            lv, "enable", vc->is_enable,
            lv, "color", vc->is_show_color,
            lv, "time", vc->show_time_mode,
            lv, "level", vc->is_show_level,
            lv, "cx", vc->is_show_cx_name,
            lv, "tag", vc->is_show_tag,
            lv, "file", vc->show_file_mode,
            lv, "func", vc->is_show_func,
            lv, "line", vc->is_show_line);
    }

    return out_num;
}

bool ilog_parser_config(const char * kvs)
{
    char key[MAX_NAME];
    char value[MAX_NAME];
    ilog_level_t level = ILOG_LEVEL_NULL;
    int i = 0;

    while ((kvs = iutil_kv_iterater(kvs, key, value)) != NULL){
        if (strncasecmp(key, "all", strlen("all")) == 0){
            for (i = ILOG_LEVEL_NULL; i < ILOG_LEVEL_NUM; i++){
                ilog_parser_vctrl_config(&levels_vctrl[i], key + strlen("all") + 1, value);
            }
            continue;
        } 
        
        for (i = ILOG_LEVEL_NULL; i < ILOG_LEVEL_NUM; i++){
            if (strncasecmp(key, ilog_level_name(i), strlen(ilog_level_name(i))) == 0){
                level = i;
                break;
            }
        }
        ilog_parser_vctrl_config(&levels_vctrl[level], key + strlen(ilog_level_name(i)) + 1, value);
    }

    char cfg[MAX_PATH];
    ilog_parser_config_format(cfg, MAX_PATH);
    //printf("%s", cfg);

    return true;
}



uint32_t ilog_parser_msg_from_raw(ilog_msg_t * msg, const char * str)
{
    if (msg == NULL || str == NULL){
        return false;
    }
    
    return ilog_raw_extract(str, &msg->time_us, &msg->level, msg->cx_name, msg->tag,
        msg->file_path, msg->func_name, &msg->line_num, &msg->log_len, msg->log);
}




/*
 *	获取等级所对应的等级名称字符串
 */
const char * ilog_level_name(int level)
{
    if (level < ILOG_LEVEL_NULL || level > ILOG_LEVEL_MASK){
        level = ILOG_LEVEL_NULL;
    }
    switch (level)
    {
    case ILOG_LEVEL_NULL:
        return "NULL";
    case ILOG_LEVEL_DEBUG:
        return "DEBUG";
    case ILOG_LEVEL_INFO:
        return "INFO ";
    case ILOG_LEVEL_WARN:
        return "WARN ";
    case ILOG_LEVEL_ERROR:
        return "ERROR";
    case ILOG_LEVEL_FATAL:
        return "FATAL";
    case ILOG_LEVEL_MASK:
        return "MASK ";;
    default:
        return "NULL ";
    }
}

/*
 *	获取等级所对应的起始颜色字符串
 */
const char * ilog_level_color(int level)
{
    if (level < ILOG_LEVEL_NULL || level > ILOG_LEVEL_MASK){
        level = ILOG_LEVEL_NULL;
    }
    switch (level)
    {
    case ILOG_LEVEL_NULL:
        return TTY_COLOR_NORAML;
    case ILOG_LEVEL_DEBUG:
        return TTY_COLOR_NORAML;
    case ILOG_LEVEL_INFO:
        return TTY_COLOR_GREEN;
    case ILOG_LEVEL_WARN:
        return TTY_COLOR_YELLOW;
    case ILOG_LEVEL_ERROR:
        return TTY_COLOR_RED;
    case ILOG_LEVEL_FATAL:
        return TTY_COLOR_RED;
    case ILOG_LEVEL_MASK:
        return TTY_COLOR_CYAN;
    default:
        return "";
    }
}


uint32_t ilog_parser_msg_vctrl_show(ilog_msg_t * msg)
{
    char * buf = NULL;
    uint32_t out_num = 0;
    ilog_parser_vctrl_t * vctrl = NULL;


    if(msg == NULL) {
        return 0;
    }

    vctrl = &levels_vctrl[msg->level];
    if (vctrl->is_enable==false){
        return 0;
    }

    
    buf = (char *)malloc(ILOG_BUF_MAX + 1);
    if (buf == NULL){
        return 0;
    }

    buf[0] = '\0';
    if (vctrl->is_show_color == true){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "%s", ilog_level_color(msg->level));
    }

    time_t tts;
    struct tm *p;
    if (vctrl->show_time_mode == 1){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "[%ld] ", msg->time_us);
    } else if (vctrl->show_time_mode == 2){
        tts = msg->time_us / 1000000;
        p = gmtime(&tts);
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "[%04d-%02d-%02d %02d:%02d:%02d:%06ld] ", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, msg->time_us % 1000000);
    } else if (vctrl->show_time_mode == 3){
        tts = msg->time_us / 1000000;
        p = gmtime(&tts);
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "[%04d-%02d-%02d %02d:%02d:%02d:%03ld] ", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, msg->time_us/1000 % 1000);
    } else if (vctrl->show_time_mode == 4){
        tts = msg->time_us / 1000000;
        p = gmtime(&tts);
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "[%02d:%02d:%02d:%06ld] ", p->tm_hour, p->tm_min, p->tm_sec, msg->time_us % 1000000);
    } 

    if (vctrl->is_show_level == true){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "[%s] ", ilog_level_name(msg->level));
    }

    if (vctrl->is_show_cx_name == true){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "[%s] ", msg->cx_name);
    }
    
    if (vctrl->is_show_tag == true){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "[%s] ", msg->tag);
    }


    /* [文件路径:行号 函数] */
    if (vctrl->show_file_mode > 0 || vctrl->is_show_func == true || vctrl->is_show_line == true){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "[");
    }
    
    if (vctrl->show_file_mode == 1){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "%s:", msg->file_path);
    }else if (vctrl->show_file_mode == 2){
        const char * file_name = strrchr(msg->file_path, '/');
        file_name = (file_name == NULL) ? msg->file_path : (file_name + 1);
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "%s:", file_name);
    }

    if (vctrl->is_show_line == true) {
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "%d ", msg->line_num);
    }

    if (vctrl->is_show_func == true){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "%s", msg->func_name);
    }
    
    
    if (vctrl->show_file_mode > 0 || vctrl->is_show_func == true || vctrl->is_show_line == true){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "] ");
    }

    out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "%s", msg->log);

    if (vctrl->is_show_color == true){
        out_num += snprintf(buf + out_num, ILOG_BUF_MAX - out_num, "%s", TTY_COLOR_NORAML);
    }

    printf("%s\n", buf);
    free(buf);

    return out_num;
}



uint32_t ilog_parser_handler(const char * str)
{
    ilog_msg_t msg;
    
    ilog_parser_msg_from_raw(&msg, str);
    return ilog_parser_msg_vctrl_show(&msg);
}




/* -------------------------[      测试用例        ]------------------------- */




void ilog_ut_level(void)
{
    ilog_debug("debug %s %d", "aaa", 100);
    ilog_info ("info  %s %d", "aaa", 100);
    ilog_warn ("warn  %s %d", "aaa", 100);
    ilog_error("error %s %d", "aaa", 100);
    ilog_fatal("fatal %s %d", "aaa", 100);
    ilog_mask ("mask  %s %d", "aaa", 100);
}


void ilog_ut_color(void)
{
    printf("colors\n");
    printf(TTY_COLOR_NORAML		"0123456789 TTY_COLOR_NORAML	\n"TTY_COLOR_NORAML);
    printf(TTY_COLOR_BLACK		"0123456789 TTY_COLOR_BLACK		\n"TTY_COLOR_NORAML);
    printf(TTY_COLOR_RED		"0123456789 TTY_COLOR_RED		\n"TTY_COLOR_NORAML);
    printf(TTY_COLOR_GREEN		"0123456789 TTY_COLOR_GREEN		\n"TTY_COLOR_NORAML);
    printf(TTY_COLOR_YELLOW		"0123456789 TTY_COLOR_YELLOW	\n"TTY_COLOR_NORAML);
    printf(TTY_COLOR_BLUE		"0123456789 TTY_COLOR_BLUE		\n"TTY_COLOR_NORAML);
    printf(TTY_COLOR_MAGENTA	"0123456789 TTY_COLOR_MAGENTA	\n"TTY_COLOR_NORAML);
    printf(TTY_COLOR_CYAN		"0123456789 TTY_COLOR_CYAN		\n"TTY_COLOR_NORAML);
    printf(TTY_COLOR_GRAY		"0123456789 TTY_COLOR_GRAY		\n"TTY_COLOR_NORAML);
}


void ilog_ut()
{
    ilog_ut_color();
    ilog_ut_level();



}