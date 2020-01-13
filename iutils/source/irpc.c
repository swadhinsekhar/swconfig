#include "irpc.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "iutil.h"
#include "ihash.h"
#include "cJSON.h"

/* 内置数据定义  */
#define CV_INNER(st, sz) \
    CVar cvar_##st = { #st, CV_##st, sz, 0, NULL, };

CV_INNER(VOID, 0)
CV_INNER(BOOL, sizeof(bool))
CV_INNER(INT8, sizeof(int8_t))
CV_INNER(INT16, sizeof(int16_t))
CV_INNER(INT32, sizeof(int32_t))
CV_INNER(INT64, sizeof(int64_t))
CV_INNER(UINT8, sizeof(uint8_t))
CV_INNER(UINT16, sizeof(uint16_t))
CV_INNER(UINT32, sizeof(uint32_t))
CV_INNER(UINT64, sizeof(uint64_t))
CV_INNER(FLOAT, sizeof(float))
CV_INNER(DOUBLE, sizeof(double))
CV_INNER(STRING, 0)


int irpc_debug = 0;

int64_t CVar_Inner_toNum(CVar * s, void * data)
{
    if (s == NULL || data == NULL) {
        return 0;
    }

    switch (s->type) {
    case CV_BOOL:
        return *((bool *)data);
    case CV_INT8:
        return *((int8_t *)data);
    case CV_INT16:
        return *((int16_t *)data);
    case CV_INT32:
        return *((int32_t *)data);
    case CV_INT64:
        return *((int64_t *)data);
    case CV_UINT8:
        return *((uint8_t *)data);
    case CV_UINT16:
        return *((uint16_t *)data);
    case CV_UINT32:
        return *((uint32_t *)data);
    case CV_UINT64:
        return *((uint64_t *)data);
    }
    return 0;
}

double CVar_Inner_toFloat(CVar * s, void * data)
{
    if (s == NULL || data == NULL) {
        return 0;
    }

    switch (s->type) {
    case CV_FLOAT:
        return *((float *)data);
    case CV_DOUBLE:
        return *((double *)data);
    }
    return 0;
}

/* 结构体变量中，第i个field，数组长度 */
uint32_t CVar_field_array_n(CVar * s, int i, void * data)
{
    CField * field = &(s->fields[i]);
    CField * vlen_field = NULL;
    // 获取该field的重复个数
    int array_n = 1;
    switch (field->flags) {
    case CF_ARRAY:
        array_n = field->vlen;
        break;
    case CF_ARRAY_VAR:
        vlen_field = &(s->fields[field->vlen]);
        array_n = (int)CVar_Inner_toNum(vlen_field->cvar, data + vlen_field->offset);
        break;
    case CF_PTR_ARRAY:
        array_n = field->vlen;
        break;
    case CF_PTR_ARRAY_VAR:
        vlen_field = &(s->fields[field->vlen]);
        array_n = (int)CVar_Inner_toNum(vlen_field->cvar, data + vlen_field->offset);
        break;
    }

    return array_n;
}


/* 获取feild的值的地址， data为cvar的地址 */
size_t CVar_field_addr(CField * field, int j, size_t data)
{
    // 获取该filed的值的地址
	size_t addr = 0;
    switch (field->flags) {
    case CF_PTR:
    case CF_STR_PTR:
        addr = data + field->offset;
        addr = *(size_t *)addr;
        break;
    case CF_PTR_ARRAY:
    case CF_PTR_ARRAY_VAR:
        addr = data + field->offset;
        addr = *(size_t *)addr;
        addr += j*field->cvar->size;
        break;
    default:
        addr = data + field->offset;
        addr += j*field->cvar->size;
    }

    return addr;
}


void CVar_free(CVar * s, void * data)
{
    if (s == NULL || data == NULL) {
        return;
    }

    int i = 0;
    // 遍历所有fields
    for (i = 0; i < s->fields_n; i++) {
        CField * field = &(s->fields[i]);

        uint32_t array_n = CVar_field_array_n(s, i, data);

        size_t addr = 0;
        int j = 0;
        for (j = 0; j < array_n; j++) {
            addr = CVar_field_addr(field, j, (size_t)data);
            if (field->cvar->type == CV_STRUCT) {
                CVar_free(field->cvar, (void *)addr);
            }
            
        }

        if ((field->flags == CF_PTR) || (field->flags == CF_PTR_ARRAY)
            || (field->flags == CF_PTR_ARRAY_VAR) || (field->flags == CF_STR_PTR)) {
            addr = (size_t)(data + field->offset);
            free((void *)(*(size_t *)addr));
            *((size_t *)addr) = 0;
        }
    }
}

/* -------------------------[      字符串打印        ]------------------------- */

size_t CVar_Inner_toString(CVar * s, void * data, char * out, char * pre)
{
    if (s == NULL || data == NULL || out == NULL || pre == NULL) {
        return 0;
    }

    int rv = 0;
    switch (s->type) {
    case CV_VOID:
        return rv + sprintf(out + rv, "%s<%s> : %lx\n", pre, "void", *((uint64_t *)data));
    case CV_BOOL:
        return rv + sprintf(out + rv, "%s(%s) : %s\n",  pre, "bool", *((bool *)data) ? "True" : "False");
    case CV_INT8:
        return rv + sprintf(out + rv, "%s(%s) : %d\n",  pre, "int8", *((int8_t *)data));
    case CV_INT16:
        return rv + sprintf(out + rv, "%s(%s) : %d\n",  pre, "int16", *((int16_t *)data));
    case CV_INT32:
        return rv + sprintf(out + rv, "%s(%s) : %d\n",  pre, "int32", *((int32_t *)data));
    case CV_INT64:
        return rv + sprintf(out + rv, "%s(%s) : %ld\n", pre, "int64", *((int64_t *)data));
    case CV_UINT8:
        return rv + sprintf(out + rv, "%s(%s) : %u\n",  pre, "uint8", *((uint8_t *)data));
    case CV_UINT16:
        return rv + sprintf(out + rv, "%s(%s) : %u\n",  pre, "uint16", *((uint16_t *)data));
    case CV_UINT32:
        return rv + sprintf(out + rv, "%s(%s) : %u\n",  pre, "uint32", *((uint32_t *)data));
    case CV_UINT64:
        return rv + sprintf(out + rv, "%s(%s) : %lu\n", pre, "uint64", *((uint64_t *)data));
    case CV_FLOAT:
        return rv + sprintf(out + rv, "%s(%s) : %f\n",  pre, "float", *((float *)data));
    case CV_DOUBLE:
        return rv + sprintf(out + rv, "%s(%s) : %f\n",  pre, "float", *((double *)data));
    case CV_STRING:
        return rv + sprintf(out + rv, "%s(%s) : %s\n",  pre, "string", (char *)data);
    }

    return 0;
}

size_t CVar_toString(CVar * s, void * data, char * out, char * pre)
{
    if (s == NULL || data == NULL || out == NULL || pre == NULL) {
        return 0;
    }

    if (s->type != CV_STRUCT) {
        return CVar_Inner_toString(s, data, out, pre);
    }

    int pre_olen = strlen(pre);
    int rv = 0;
    int i = 0;
    // 遍历所有fields
    for (i = 0; i < s->fields_n; i++) {
        uint32_t array_n = CVar_field_array_n(s, i, data);

        // 遍历该field的重复个数
        CField * field = &(s->fields[i]);
        int j = 0;
        for (j = 0; j < array_n; j++) {
            size_t addr = CVar_field_addr(field, j, (size_t)data);

            pre[pre_olen] = '\0';
            int pre_rv = pre_olen;
            pre_rv += sprintf(pre + pre_rv, "%s", field->name);
            if (field->flags == CF_PTR_ARRAY || field->flags == CF_PTR_ARRAY_VAR
                || field->flags == CF_ARRAY || field->flags == CF_ARRAY_VAR) {
                pre_rv += sprintf(pre + pre_rv, "[%d]", j);
            }


            // 如果该field为结构体，则添加前缀
            if (field->cvar->type == CV_STRUCT) {
                if ((field->flags == CF_PTR) || (field->flags == CF_PTR_ARRAY)
                    || (field->flags == CF_PTR_ARRAY_VAR) || (field->flags == CF_STR_PTR)) {
                    pre_rv += sprintf(pre + pre_rv, "->");
                } else {
                    pre_rv += sprintf(pre + pre_rv, ".");
                }
            }

            // 打印该field
            rv += CVar_toString(field->cvar, (void *)addr, out + rv, pre);
        }
    }

    return rv;
}

void CVar_show(CVar * s, void * data)
{
    char str[1024 * 10] = "";
    char pre[512] = "";
    sprintf(pre, "%s.", s->name);
    CVar_toString(s, data, str, pre);
    printf("%s", str);
}


/* -------------------------[      JSON序列化        ]------------------------- */

cJSON * CVar_Inner_toJson(CVar * s, void * data)
{
    if (s == NULL || data == NULL) {
        return NULL;
    }

    switch (s->type) {
    case CV_BOOL:
        return cJSON_CreateBool(*((bool *)data));
    case CV_INT8:
    case CV_INT16:
    case CV_INT32:
    case CV_INT64:
    case CV_UINT8:
    case CV_UINT16:
    case CV_UINT32:
    case CV_UINT64:
        return cJSON_CreateNumber(CVar_Inner_toNum(s, data));
    case CV_FLOAT:
    case CV_DOUBLE:
        return cJSON_CreateNumber(CVar_Inner_toFloat(s, data));
    case CV_STRING:
        return cJSON_CreateString(data);
    }
    return NULL;
}



cJSON * CVar_toJson(CVar * s, void * data)
{
    if (s == NULL || data == NULL) {
        return NULL;
    }

    if (s->type != CV_STRUCT) {
        return CVar_Inner_toJson(s, data);
    }

    cJSON * root = cJSON_CreateObject();
    int i = 0;
    // 遍历所有fields
    for (i = 0; i < s->fields_n; i++) {
        uint32_t array_n = CVar_field_array_n(s, i, data);

        // 遍历该field的重复个数
        CField * field = &(s->fields[i]);

        cJSON * froot = root;
        if (field->flags == CF_PTR_ARRAY || field->flags == CF_PTR_ARRAY_VAR
            || field->flags == CF_ARRAY || field->flags == CF_ARRAY_VAR) {
            froot = cJSON_CreateArray();
            cJSON_AddItemToObject(root, field->name, froot);
        }

        int j = 0;
        for (j = 0; j < array_n; j++) {
            size_t addr = CVar_field_addr(field, j, (size_t)data);
            cJSON_AddItemToObject(froot, field->name, CVar_toJson(field->cvar, (void *)addr));
        }
    }

    return root;
}


size_t CVar_Inner_fromJson(CVar * s, void * data, cJSON * root)
{
    if (s == NULL || data == NULL || root == NULL) {
        return 0;
    }

    switch (s->type) {
    case CV_BOOL:
        if (*((bool *)data) != root->valueint) {
            *((bool *)data) = root->valueint;
        }
        break;
    case CV_INT8:
        if (*((int8_t *)data) != root->valueint) {
            *((int8_t *)data) = root->valueint;
        }
        break;
    case CV_INT16:
        if (*((int16_t *)data) != root->valueint) {
            *((int16_t *)data) = root->valueint;
        }
        break;
    case CV_INT32:
        if (*((int32_t *)data) != root->valueint) {
            *((int32_t *)data) = root->valueint;
        }
        break;
    case CV_INT64:
        if (*((int64_t *)data) != root->valueint) {
            *((int64_t *)data) = root->valueint;
        }
        break;
    case CV_UINT8:
        if (*((uint8_t *)data) != root->valueint) {
            *((uint8_t *)data) = root->valueint;
        }
        break;
    case CV_UINT16:
        if (*((uint16_t *)data) != root->valueint) {
            *((uint16_t *)data) = root->valueint;
        }
        break;
    case CV_UINT32:
        if (*((uint32_t *)data) != root->valueint) {
            *((uint32_t *)data) = root->valueint;
        }
        break;
    case CV_UINT64:
        if (*((uint64_t *)data) != root->valueint) {
            *((uint64_t *)data) = root->valueint;
        }
        break;
    case CV_FLOAT:
        if (*((float *)data) != root->valuedouble) {
            *((float *)data) = root->valuedouble;
        }
        break;
    case CV_DOUBLE:
        if (*((double *)data) != root->valuedouble) {
            *((double *)data) = root->valuedouble;
        }
        break;
    case CV_STRING:
        if (strcmp(data, root->valuestring)!=0) {
            strcpy(data, root->valuestring);
        }
        break;
    }
    return 0;
}


size_t CVar_fromJson(CVar * s, void * data, cJSON * root)
{
    if (s == NULL || data == NULL || root == NULL) {
        return 0;
    }

    if (root->type != cJSON_Object) {
        return CVar_Inner_fromJson(s, data, root);
    }


    int i, j;
    for (i = 0; i < s->fields_n; i++) {
        CField * field = &(s->fields[i]);

        cJSON * item = cJSON_GetObjectItem(root, field->name);
        if (item == NULL) {
            continue;
        }

        int array_n = 0;
        size_t addr;
        cJSON * item_n = NULL;
        switch (field->flags) {
        case CF_PTR:
            addr = (size_t)(data + field->offset);
            if (*(size_t *)addr == 0) {	// 为NULL时，才分配内存
                *(size_t *)addr = (size_t)malloc(field->cvar->size);
                memset((void *)(*(size_t *)addr), 0x00, field->cvar->size);
            }
            CVar_fromJson(field->cvar, (void *)(*(size_t *)addr), item);
            break;
        case CF_STR_PTR:
            addr = (size_t)(data + field->offset);
            if (*(size_t *)addr == 0) {	// 为NULL时，才分配内存
                *(size_t *)addr = (size_t)malloc(field->vlen);
                memset((void *)(*(size_t *)addr), 0x00, field->vlen);
            }
            CVar_fromJson(field->cvar, (void *)(*(size_t *)addr), item);
            break;
        case CF_PTR_ARRAY:
        case CF_PTR_ARRAY_VAR:
            array_n = cJSON_GetArraySize(item);
            addr = (size_t)(data + field->offset);
            if (*(size_t *)addr == 0) {	// 为NULL时，才分配内存
                *(size_t *)addr = (size_t)malloc(field->cvar->size * array_n);
                memset((void *)(*(size_t *)addr), 0x00, field->cvar->size * array_n);
            }

            for (j = 0; j < array_n; j++) {
                item_n = cJSON_GetArrayItem(item, j);
                CVar_fromJson(field->cvar, (void *)(*(size_t *)addr + field->cvar->size*j), item_n);
            }
            break;

        case CF_ARRAY:
        case CF_ARRAY_VAR:
            array_n = cJSON_GetArraySize(item);
            addr = (size_t)(data + field->offset);
            for (j = 0; j < array_n; j++) {
                item_n = cJSON_GetArrayItem(item, j);
                CVar_fromJson(field->cvar, (void *)(addr + field->cvar->size*j), item_n);
            }
            break;
        case CF_STR:
        default:
            addr = (size_t)(data + field->offset);
            CVar_fromJson(field->cvar, (void *)addr, item);
        }
    }

    return 0;
}

/* -------------------------[      内存模式序列化        ]------------------------- */

#pragma pack(1) 
typedef struct  {
    uint16_t type;
    uint32_t len;
    uint8_t	value[0];
}dm_tlv;
#pragma pack()

size_t CVar_toDMem(CVar * s, void * data, uint8_t * dm)
{
    if (s == NULL || dm == NULL) {
        return 0;
    }
    uint32_t rv = 0;
    dm_tlv * tlv = (dm_tlv *)dm;

    if (data != NULL){
        if (s->type != CV_STRUCT) {
            if (s->type == CV_STRING) {
                uint32_t str_len = strlen(data) + 1;
                memmove(tlv->value + rv, data, str_len);
                rv += str_len;
            } else {
                memmove(tlv->value + rv, data, s->size);
                rv += s->size;
            }
        }

        int i = 0;
        // 遍历所有fields
        for (i = 0; i < s->fields_n; i++) {
            uint32_t array_n = CVar_field_array_n(s, i, data);

            // 遍历该field的重复个数
            CField * field = &(s->fields[i]);

            int j = 0;
            for (j = 0; j < array_n; j++) {
                size_t addr = CVar_field_addr(field, j, (size_t)data);
                rv += CVar_toDMem(field->cvar, (void *)addr, tlv->value + rv);
            }
        }
    }
    
    tlv->type = s->type;
    tlv->len = rv + 6;
    return tlv->len;

}


size_t CVar_FromDMem(CVar * s, void * data, uint8_t * dm)
{
    if (s == NULL  || dm == NULL) {
        return 0;
    }

    uint32_t rv = 0;
    dm_tlv * tlv = (dm_tlv *)dm;

    if (tlv->len <= 6){
        return rv + tlv->len;
    }

    if (s->type != CV_STRUCT) {
        if (memcmp(data, tlv->value + rv, tlv->len - 6)!=0) {
            memmove(data, tlv->value + rv, tlv->len - 6);
        }
        rv += tlv->len - 6;
    }


    int i, j;
    for (i = 0; i < s->fields_n; i++) {
        CField * field = &(s->fields[i]);
        dm_tlv * tlv_field = (dm_tlv *)(tlv->value + rv);
        int array_n = 0;
        size_t addr;
        switch (field->flags) {
        case CF_PTR:
            addr = (size_t)(data + field->offset);
            if (*(size_t *)addr == 0  && tlv_field->len > 6) {	// 为NULL时，才分配内存
                *(size_t *)addr = (size_t)malloc(field->cvar->size);
                memset((void *)(*(size_t *)addr), 0x00, field->cvar->size);
            }
            rv += CVar_FromDMem(field->cvar, (void *)(*(size_t *)addr), tlv->value + rv);
            break;
        case CF_STR_PTR:
            addr = (size_t)(data + field->offset);
            if (*(size_t *)addr == 0 && tlv_field->len > 6) {	// 为NULL时，才分配内存
                *(size_t *)addr = (size_t)malloc(field->vlen);
                memset((void *)(*(size_t *)addr), 0x00, field->vlen);
            }
            rv += CVar_FromDMem(field->cvar, (void *)(*(size_t *)addr), tlv->value + rv);
            break;
        case CF_PTR_ARRAY:
        case CF_PTR_ARRAY_VAR:
            array_n = CVar_field_array_n(s, i, data);
            addr = (size_t)(data + field->offset);
            if (*(size_t *)addr == 0 && tlv_field->len > 6) {	// 为NULL时，才分配内存
                *(size_t *)addr = (size_t)malloc(field->cvar->size * array_n);
                memset((void *)(*(size_t *)addr), 0x00, field->cvar->size * array_n);
            }

            for (j = 0; j < array_n; j++) {
                rv += CVar_FromDMem(field->cvar, (void *)(*(size_t *)addr + field->cvar->size*j), tlv->value + rv);
            }
            break;

        case CF_ARRAY:
        case CF_ARRAY_VAR:
            array_n = CVar_field_array_n(s, i, data);
            addr = (size_t)(data + field->offset);
            for (j = 0; j < array_n; j++) {
                rv += CVar_FromDMem(field->cvar, (void *)(addr + field->cvar->size*j), tlv->value + rv);
            }
            break;
        case CF_STR:
        default:
            addr = (size_t)(data + field->offset);
            rv += CVar_FromDMem(field->cvar, (void *)addr, tlv->value + rv);
        }
    }

    return rv+6;
}





/* -------------------------[      RPC        ]------------------------- */
ihash_t irpc_funcs_hash;

int irpc_func_show(void * d);
void * irpc_srv_run_dmem(void * arg);
void * irpc_srv_run_json(void * arg);
int irpc_srv_socket_cb_json(struct socket_recv_s * recv_data);
int irpc_srv_socket_cb_dmem(struct socket_recv_s * recv_data);

int irpc_port_json = 9101;
int irpc_port_dmem = 9102;
char irpc_client_ip[64] = "127.0.0.1";


void irpc_config(int port_json, int port_dmem, char * client_ip)
{
    irpc_port_json = port_json;
    irpc_port_dmem = port_dmem;
    strcpy(irpc_client_ip, client_ip);

    ilog_mask("port_json=%d port_dmem=%d  client_ip=%s", port_json, port_dmem, client_ip);
}


bool irpc_init()
{
    int i = 0;
    int ret = 0;

    // 创建hash
    ihash_create(&irpc_funcs_hash, 10240);
    ihash_key_set(&irpc_funcs_hash, 0, offsetof(irpc_func, func));
    irpc_funcs_hash.entry_show = irpc_func_show;

    pthread_t recv_tid;
    ret = pthread_create(&recv_tid, NULL, irpc_srv_run_dmem, NULL);
    pthread_detach(recv_tid);
    ret = pthread_create(&recv_tid, NULL, irpc_srv_run_json, NULL);
    pthread_detach(recv_tid);
    EX_INFO(ret == 0, "irpc_srv_run thread create");
    usleep(1000 * 100);
    return true;
}

int irpc_func_show(void * d)
{
    if (d == NULL) {
        printf("NULL");
        return 0;
    }

    irpc_func * s = d;
    return printf("%s 0x%lX", s->name, (size_t)(s->func));
}



bool irpc_server_func_add(irpc_func * rpc)
{
    if (rpc == NULL) {
        return false;
    }

    ilog_info("irpc add func %s", rpc->name);
    ihash_add(&irpc_funcs_hash, rpc);
    return true;
}



void * irpc_srv_run_json(void * arg)
{
    socket_tcp_server(irpc_port_json, NULL, irpc_srv_socket_cb_json);
    return NULL;
}

int irpc_srv_socket_cb_json(struct socket_recv_s * recv_data)
{
    if (recv_data == NULL) {
        return 0;
    }

    int sockfd = recv_data->client_fd;
    FILE * fp = fdopen(sockfd, "rw+");

    char data_r[1024 * 10] = "";
    size_t data_r_n = 0;
    irpc_func rpc_r;
    int ret = 0;
    int ret2 = 0;

    while (1) {
        memset(&rpc_r, 0x00, sizeof(irpc_func));
        ret = fscanf(fp, "%s\n %lu\nend", rpc_r.name, &data_r_n);
        if (ret < 0){
            break;
        }
        ret = fread(data_r, data_r_n, 1, fp);
        if (ret < 0) {
            break;
        }
        data_r[data_r_n] = '\0';

        irpc_func * rpc = ihash_find(&irpc_funcs_hash, &rpc_r);
        if (rpc == NULL) {
            ilog_error("function %s not register");
            continue;
        }

        cJSON * root_r = cJSON_Parse(data_r);
        void *pvs_r[IRPC_PARAM_MAX] = { 0 };
        CVar_fromJson(rpc->cvar, pvs_r, root_r);
        cJSON_Delete(root_r);
        root_r = NULL;
        rpc->func(pvs_r);

        cJSON * root_s = CVar_toJson(rpc->cvar, pvs_r);
        char * data_s = cJSON_Print(root_s);
        cJSON_Delete(root_s);
        root_s = NULL;
        ret = fprintf(fp, "%s\n %lu\nend", rpc->cvar->name, strlen(data_s));
        ret2 = fputs(data_s, fp);
        free(data_s);
        data_s = NULL;
        CVar_free(rpc->cvar, pvs_r);	//释放反序列化的内存
        if (ret < 0 || ret2<0) {
            break;
        }
    }

    fclose(fp);
    close(sockfd);

    return 0;
}




void irpc_clt_call_json(CVar *cvar, void * pvs[])
{
    static FILE * fp = NULL;
    static int sockfd = 0;
    pthread_mutex_t  mutex;
    if (fp == NULL) {
        sockfd = socket_tcp_client(irpc_client_ip, irpc_port_dmem);
        fp = fdopen(sockfd, "rw+");
        if (fp == NULL) {
            ilog_error("fdopen fail");
            return;
        }
        pthread_mutex_init(&mutex, NULL);
    }
    int ret = 0;
    int ret2 = 0;

    pthread_mutex_lock(&mutex);
    // 发送数据
    cJSON * root_s = CVar_toJson(cvar, pvs);
    char * data_s = cJSON_Print(root_s);
    cJSON_Delete(root_s);
    ret = fprintf(fp, "%s\n %lu\nend", cvar->name, strlen(data_s));
    ret2 = fputs(data_s, fp);
    free(data_s);
    if (ret < 0 || ret2 < 0) {
        return;
    }

    // 接受数据
    char rpc_name[64];
    char data_r[1024 * 10];
    size_t data_r_n = 0;
    ret = fscanf(fp, "%s\n %lu\nend", rpc_name, &data_r_n);
    ret2 = fread(data_r, data_r_n, 1, fp);
    if (ret < 0 || ret2 < 0) {
        return;
    }

    data_r[data_r_n] = '\0';
    cJSON * root_r = cJSON_Parse(data_r);
    CVar_fromJson(cvar, pvs, root_r);
    cJSON_Delete(root_r);

    pthread_mutex_unlock(&mutex);
    //按需复制

    //释放反序列化的内存
}



void * irpc_srv_run_dmem(void * arg)
{
    socket_tcp_server(irpc_port_dmem, NULL, irpc_srv_socket_cb_dmem);
    return NULL;
}


int irpc_srv_socket_cb_dmem(struct socket_recv_s * recv_data)
{
    if (recv_data == NULL) {
        return 0;
    }

    int sockfd = recv_data->client_fd;
    FILE * fp = fdopen(sockfd, "rw+");
    if (fp == NULL){
        close(sockfd);
        return 0;
    }

    char data_r[1024 * 10] = "";
    char data_s[1024 * 10] = "";
    size_t data_r_n = 0;
    size_t data_s_n = 0;
    irpc_func rpc_r;

    int ret = 0;
    int ret2 = 0;

    while (1) {
        memset(&rpc_r, 0x00, sizeof(irpc_func));
        ret = fscanf(fp, "%s\n %lu\nend", rpc_r.name, &data_r_n);
        ret2 = fread(data_r, data_r_n, 1, fp);
        if (ret < 0 || ret2 < 0) {
            break;
        }

        irpc_func * rpc = ihash_find(&irpc_funcs_hash, &rpc_r);
        if (rpc == NULL) {
            ilog_error("function %s not register");
            continue;
        }


        void *pvs_r[IRPC_PARAM_MAX] = { 0 };
        CVar_FromDMem(rpc->cvar, pvs_r, data_r);
        if (irpc_debug == 1) {
            CVar_show(rpc->cvar, pvs_r);
        }

        rpc->func(pvs_r);

        data_s_n = CVar_toDMem(rpc->cvar, pvs_r, data_s);
        if (irpc_debug == 1) {
            CVar_show(rpc->cvar, pvs_r);
        }
        ret = fprintf(fp, "%s\n %lu\nend", rpc->cvar->name, data_s_n);
        ret2 = fwrite(data_s, data_s_n, 1, fp);
        CVar_free(rpc->cvar, pvs_r);	//释放反序列化的内存
        if (ret < 0 || ret2 < 0) {
            break;
        }
    }

    fclose(fp);
    close(sockfd);

    return 0;
}




void irpc_clt_call_dmem(CVar *cvar, void * pvs[])
{
    static FILE * fp = NULL;
    static int sockfd = 0;
    static pthread_mutex_t  mutex;
    if (fp == NULL) {
        sockfd = socket_tcp_client(irpc_client_ip, irpc_port_dmem);
        fp = fdopen(sockfd, "rw+");
        if (fp == NULL){
            ilog_error("fdopen fail");
            return;
        }
        pthread_mutex_init(&mutex, NULL);
    }

    pthread_mutex_lock(&mutex);

    char data_r[1024 * 10] = "";
    char data_s[1024 * 10] = "";
    size_t data_r_n = 0;
    size_t data_s_n = 0;

    int ret = 0;
    int ret2 = 0;

    // 发送数据
    data_s_n = CVar_toDMem(cvar, pvs, data_s);
    if (irpc_debug == 1) {
        CVar_show(cvar, pvs);
    }
    ret = fprintf(fp, "%s\n %lu\nend", cvar->name, data_s_n);
    ret2 = fwrite(data_s, data_s_n, 1, fp);
    if (ret < 0 || ret2 < 0) {
        pthread_mutex_unlock(&mutex);
        ilog_error("send data fail");
        fclose(fp);
        close(sockfd);
        fp = NULL;
        return;
    }
    fflush(fp);
    
    // 接受数据
    char rpc_name[64];
    ret = fscanf(fp, "%s\n %lu\nend", rpc_name, &data_r_n);
    ret2 = fread(data_r, data_r_n, 1, fp);
    if (ret < 0 || ret2 < 0) {
        pthread_mutex_unlock(&mutex);
        ilog_error("send data fail");
        fclose(fp);
        close(sockfd);
        fp = NULL;
        return;
    }


    CVar_FromDMem(cvar, pvs, data_r);
    if (irpc_debug == 1) {
        CVar_show(cvar, pvs);
    }
    pthread_mutex_unlock(&mutex);
    //按需复制

    //释放反序列化的内存
    //fclose(fp);
    //fp = NULL;
    //close(sockfd);
    //sockfd = 0;
    return;
}


void irpc_server_info()
{
    ihash_show(&irpc_funcs_hash);
}




