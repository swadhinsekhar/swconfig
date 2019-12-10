#ifndef ILOG_H
#define ILOG_H

#ifdef __cplusplus
extern "C" {
#endif


#include "itypes.h"


/* log�ȼ� */
typedef enum ilog_level_e
{
    ILOG_LEVEL_NULL  = 0,			/* log�ȼ� NULL  */
    ILOG_LEVEL_DEBUG = 1,			/* log�ȼ� DEBUG */
    ILOG_LEVEL_INFO  = 2,			/* log�ȼ� INFO  */
    ILOG_LEVEL_WARN  = 3,			/* log�ȼ� WARN  */
    ILOG_LEVEL_ERROR = 4,			/* log�ȼ� ERROR */
    ILOG_LEVEL_FATAL = 5,			/* log�ȼ� FATAL */
    ILOG_LEVEL_MASK  = 6,			/* log�ȼ� MASK  */
    ILOG_LEVEL_NUM   = 7,			/* log�ȼ� ����  */
}ilog_level_t;


/* ��������� */
typedef enum ilog_outstream_type_e
{
    ILOG_OUTSTREAM_TYPE_NULL   = 0,		/* ���������Ϊ�� */
    ILOG_OUTSTREAM_TYPE_STDOUT = 1,		/* ��������򵽱�׼��� */
    ILOG_OUTSTREAM_TYPE_DCALL  = 2,		/* �����ֱ�ӵ��ú��� */
    ILOG_OUTSTREAM_TYPE_FILE   = 3,		/* ����������ļ� */
    ILOG_OUTSTREAM_TYPE_SOCKET_TCP = 4,	/* ���������socket�׽��� */
    ILOG_OUTSTREAM_TYPE_SOCKET_UDP = 5,	/* ���������socket�׽��� */
    ILOG_OUTSTREAM_TYPE_NUM	   = 6,		/* ��������͸��� */
}ilog_outstream_type_t;


/* log������ṹ�� */
typedef struct ilog_outstream_s
{
    ilog_outstream_type_t type;			/* ��������� */
    char config_cmd[MAX_NAME];			/* ��������͵�������Ϣ */
    void * config_data;					/* ��������͵���������ָ�� */
}ilog_outstream_t;


/* log�����Ľṹ�� */
typedef struct ilog_context_s
{
    char name[MAX_NAME];						/* ���������� */
    bool level_enable[ILOG_LEVEL_NUM];			/* �ȼ���־��ӡ���� */
    ilog_outstream_t level_os[ILOG_LEVEL_NUM];	/* �ȼ������ */
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