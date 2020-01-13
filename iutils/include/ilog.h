#ifndef ILOG_H
#define ILOG_H

#ifdef __cplusplus
extern "C" {
#endif


#include "itypes.h"


/* log等级 */
typedef enum ilog_level_e
{
    ILOG_LEVEL_NULL  = 0,			/* log等级 NULL  */
    ILOG_LEVEL_DEBUG = 1,			/* log等级 DEBUG */
    ILOG_LEVEL_INFO  = 2,			/* log等级 INFO  */
    ILOG_LEVEL_WARN  = 3,			/* log等级 WARN  */
    ILOG_LEVEL_ERROR = 4,			/* log等级 ERROR */
    ILOG_LEVEL_FATAL = 5,			/* log等级 FATAL */
    ILOG_LEVEL_MASK  = 6,			/* log等级 MASK  */
    ILOG_LEVEL_NUM   = 7,			/* log等级 个数  */
}ilog_level_t;


/* 输出流类型 */
typedef enum ilog_outstream_type_e
{
    ILOG_OUTSTREAM_TYPE_NULL   = 0,		/* 输出流定向为空 */
    ILOG_OUTSTREAM_TYPE_STDOUT = 1,		/* 输出流定向到标准输出 */
    ILOG_OUTSTREAM_TYPE_DCALL  = 2,		/* 输出到直接调用函数 */
    ILOG_OUTSTREAM_TYPE_FILE   = 3,		/* 输出流定向到文件 */
    ILOG_OUTSTREAM_TYPE_SOCKET_TCP = 4,	/* 输出流定向到socket套接字 */
    ILOG_OUTSTREAM_TYPE_SOCKET_UDP = 5,	/* 输出流定向到socket套接字 */
    ILOG_OUTSTREAM_TYPE_NUM	   = 6,		/* 输出流类型个数 */
}ilog_outstream_type_t;


/* log输出流结构体 */
typedef struct ilog_outstream_s
{
    ilog_outstream_type_t type;			/* 输出流类型 */
    char config_cmd[MAX_NAME];			/* 输出流类型的配置信息 */
    void * config_data;					/* 输出流类型的配置数据指针 */
}ilog_outstream_t;


/* log上下文结构体 */
typedef struct ilog_context_s
{
    char name[MAX_NAME];						/* 上下文名称 */
    bool level_enable[ILOG_LEVEL_NUM];			/* 等级日志打印开关 */
    ilog_outstream_t level_os[ILOG_LEVEL_NUM];	/* 等级输出流 */
}ilog_context_t;




void ilog_init(void);
void ilog_default_cx_set(uint8_t cx_index);

uint32_t ilog_puts(ilog_context_t * cx, ilog_level_t level, 
    const char * tag, const char * file_path, const char * func,
    const uint32_t line, const char * log_str);

uint32_t ilog_printf(ilog_context_t * cx, ilog_level_t level, 
    const char * tag, const char * file_path, const char * func, 
    const uint32_t line, const char * fmt, ...);

#define ilogm(cx, level, ...)	ilog_printf(cx, level, NULL, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define ilog_debug(...)			ilogm(NULL, ILOG_LEVEL_DEBUG, __VA_ARGS__)
#define ilog_info(...)			ilogm(NULL, ILOG_LEVEL_INFO,  __VA_ARGS__)
#define ilog_warn(...)			ilogm(NULL, ILOG_LEVEL_WARN,  __VA_ARGS__)
#define ilog_error(...)			ilogm(NULL, ILOG_LEVEL_ERROR, __VA_ARGS__)
#define ilog_fatal(...)			ilogm(NULL, ILOG_LEVEL_FATAL, __VA_ARGS__)
#define ilog_mask(...)			ilogm(NULL, ILOG_LEVEL_MASK,  __VA_ARGS__)




void ilog_trace(void);
void ilog_array(void * ds, int num);


uint32_t ilog_parser_init(void);
bool ilog_parser_config(const char * kvs);



#ifdef __cplusplus
}
#endif

#endif