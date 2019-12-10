#ifndef IRPC_H
#define IRPC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cJSON.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
#endif


/* -------------------------[      序列化        ]------------------------- */

typedef struct CField_t CField;		/* 成员 */
typedef struct CVar_t CVar;			/* 变量 */

typedef enum CVar_type_t{
    CV_VOID,
    CV_BOOL,
    CV_INT8,    
    CV_INT16,
    CV_INT32,
    CV_INT64,
    CV_UINT8,
    CV_UINT16,
    CV_UINT32,
    CV_UINT64,
    CV_FLOAT,
    CV_DOUBLE,
    CV_STRUCT,
    CV_STRING,
} CVar_type;

#define CF_PTR				1		/* 指针					vlen:无意义 */
#define CF_PTR_ARRAY		2		/* 指向固定长度数组的指针	vlen:数组长度 */
#define CF_PTR_ARRAY_VAR	3		/* 指向可变长度数组的指针	vlen:表示数组长度的field的序号 */
#define CF_ARRAY			4		/* 固定长度数组			vlen:数组长度 */
#define CF_ARRAY_VAR		5		/* 可变长度数组			vlen:表示数组长度的field的序号 */
#define CF_STR				6		/* 字符串				vlen:字符串长度 */
#define CF_STR_PTR			7		/* 字符串指针				vlen:分配的字符串内存长度 */

struct CField_t {
    char	 *	name;
    CVar	 *  cvar;
    uint32_t	offset;		/* 在struct中的偏移 */
    uint32_t	vlen;		
    uint64_t	flags;
};

struct CVar_t {
    char	*	name;
    CVar_type	type;
    uint32_t	size;
    uint32_t	fields_n;
    CField *	fields;
};


/* 内置数据 */
extern CVar cvar_VOID;
extern CVar cvar_BOOL;
extern CVar cvar_INT8;
extern CVar cvar_INT16;
extern CVar cvar_INT32;
extern CVar cvar_INT64;
extern CVar cvar_UINT8;
extern CVar cvar_UINT16;
extern CVar cvar_UINT32;
extern CVar cvar_UINT64;
extern CVar cvar_FLOAT;
extern CVar cvar_DOUBLE;
extern CVar cvar_STRING;


/* 对外函数 */
void CVar_free(CVar * s, void * data);

void CVar_show(CVar * s, void * data);
size_t CVar_toString(CVar * s, void * data, char * out, char * pre);
size_t CField_toString(CField * f, void* data, char * out, char * pre);

cJSON * CVar_toJson(CVar * s, void * data);
size_t  CVar_fromJson(CVar * s, void * data, cJSON * root);

size_t CVar_toDMem(CVar * s, void * data, uint8_t * dm);
size_t CVar_FromDMem(CVar * s, void * data, uint8_t * dm);


#define CVAR_STRUCT_DF(t) \
    CVar cvar_##t = { #t, CV_STRUCT, ARRAY_SIZE(fields_##t) * PLEN, ARRAY_SIZE(fields_##t), fields_##t, }
#define IRPC_FUNC_DF(t) \
    irpc_func irpc_func__##t = { #t, irpc_srv__##t, &cvar_##t }

/* -------------------------[      RPC        ]------------------------- */

#define PLEN  sizeof(void *)			// 指针占字节数
#define IRPC_PARAM_MAX				32	// RPC支持的最大参数个数
typedef void (*irpc_func_handle)(void * pvs[]);

typedef struct irpc_func_t{
    char name[64];
    irpc_func_handle func;
    CVar * cvar;
}irpc_func;

bool irpc_init();
void irpc_config(int port_json, int port_dmem, char * client_ip);
bool irpc_server_func_add(irpc_func * rpc);
void irpc_clt_call_json(CVar *cvar, void * pvs[]);
void irpc_clt_call_dmem(CVar *cvar, void * pvs[]);
#endif