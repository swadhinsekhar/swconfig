#include "person.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  
#include <string.h>
#include "irpc.h"
#include "ilog.h"
#include "iassert.h"
#include "iutil.h"
#include "cJSON.h"



/* -------------------------[      person测试用例        ]------------------------- */

person * person_create(void)
{
    person * p = malloc(sizeof(person));
    memset(p, 0x00, sizeof(person));
    p->age = 27;
    strcpy(p->name, "hexingshi");
    p->ids[0] = 0;
    p->ids[1] = 1;
    p->ids[2] = 2;
    p->ids[3] = 3;
    p->ids[4] = 4;

    p->info = NULL;
    //p->info = malloc(64);
    //strcpy(p->info, "person tinfo");
    strcpy(p->addr.city, "shen zheng");
    p->addr.desc = malloc(64);
    strcpy(p->addr.desc, "home zzz");
    p->addr.code = 55555;

    //p->addrs2_n = 3;
    strcpy(p->addrs2[0].city, "dd0 shen zheng");
    p->addrs2[0].desc = malloc(64);
    strcpy(p->addrs2[0].desc, "home zzz");
    p->addrs2[0].code = 55550;
    strcpy(p->addrs2[1].city, "dd1 shen zheng");
    p->addrs2[1].desc = malloc(64);
    strcpy(p->addrs2[1].desc, "home zzz");
    p->addrs2[1].code = 55551;
    strcpy(p->addrs2[2].city, "dd2 shen zheng");
    p->addrs2[2].desc = malloc(64);
    strcpy(p->addrs2[2].desc, "home zzz");
    p->addrs2[2].code = 55552;

    p->addr_p = NULL;
    //p->addr_p = malloc(sizeof(address));
    //strcpy(p->addr_p->city, "pp2 shen zheng");
    //p->addr_p->desc = malloc(64);
    //strcpy(p->addr_p->desc, "home zzz");
    //p->addr_p->code = 55552;

    p->addr_arr_n = 2;
    p->addr_arr = malloc(sizeof(address)*2);
    strcpy(p->addr_arr[0].city, "aa0 shen zheng");
    p->addr_arr[0].desc = malloc(64);
    strcpy(p->addr_arr[0].desc, "home zzz");
    p->addr_arr[0].code = 55560;
    strcpy(p->addr_arr[1].city, "aa1 shen zheng");
    p->addr_arr[1].desc = malloc(64);
    strcpy(p->addr_arr[1].desc, "home zzz");
    p->addr_arr[1].code = 55561;
    return p;
}

int32_t person_do(person * p, uint32_t action, char * des, person * p_out)
{
    //ilog_mask("person_do function exec");
    //printf("action : %d\n", action);
    //printf("des : %s\n", des);
    p_out->age = p->age + 20;
    strcpy(p_out->name, p->name);
    //sprintf(p_out->info, "%s serv_done", p->info);
    return 200;
}

/* -------------------------[      数据        ]------------------------- */


CField fields_address[] = {
    { "city", &cvar_STRING, offsetof(address, city), 32, CF_STR },
    { "desc", &cvar_STRING, offsetof(address, desc), 128, CF_STR_PTR },
    { "code", &cvar_UINT64, offsetof(address, code), 0, 0},
};
CVar cvar_address = {"address", CV_STRUCT, sizeof(address), ARRAY_SIZE(fields_address), fields_address,};
CField fields_person[] = {
    { "age", &cvar_UINT8, offsetof(person, age), 0, 0 },
    { "name", &cvar_STRING, offsetof(person, name), 64, CF_STR },
    { "ids", &cvar_UINT32, offsetof(person, ids), 10, CF_ARRAY },
    { "info", &cvar_STRING, offsetof(person, info), 128, CF_STR_PTR },
    { "addr", &cvar_address, offsetof(person, addr), 0, 0},
    { "addrs2", &cvar_address, offsetof(person, addrs2), 5, CF_ARRAY },
    { "addr_p", &cvar_address, offsetof(person, addr_p), 0, CF_PTR },
    { "addr_arr_n", &cvar_UINT32, offsetof(person, addr_arr_n), 0, 0 },
    { "addr_arr", &cvar_address, offsetof(person, addr_arr), 7, CF_PTR_ARRAY_VAR },
};
CVar cvar_person = {"person", CV_STRUCT, sizeof(person), ARRAY_SIZE(fields_person), fields_person,};




/* -------------------------[      序列化        ]------------------------- */

void person_serialize()
{
    person * p = person_create();

    CVar_show(&cvar_person, p);
    ilog_info("-------------------");

    
    cJSON * root = CVar_toJson(&cvar_person, p);
    char * str = cJSON_Print(root);
    printf("%s\n", str);
    free(str);
    ilog_info("-------------------");
    
    person p2 = { 0 };
    CVar_fromJson(&cvar_person, &p2, root);
    CVar_show(&cvar_person, &p2);
    ilog_info("-------------------");
    cJSON_Delete(root);
    CVar_free(&cvar_person, &p2);


    size_t len = 0;
    uint8_t buf[1024 * 100];
    memset(buf, 0x00, sizeof(buf));

    len = CVar_toDMem(&cvar_person, p, buf);
    ilog_array(buf, len);
    ilog_info("-------------------");

    person p3 = { 0 };
    CVar_FromDMem(&cvar_person, &p3, buf);
    CVar_show(&cvar_person, &p3);
    ilog_info("-------------------");

}


/* -------------------------[      RPC_UT        ]------------------------- */


CField fields_person_do[] = {
    { "rpc_name", &cvar_STRING, 0 * PLEN, 128, CF_PTR },
    { "ret", &cvar_INT32, 1 * PLEN, 0, CF_PTR },
    { "p", &cvar_person, 2 * PLEN, 0, CF_PTR },
    { "action", &cvar_UINT32, 3 * PLEN, 0, CF_PTR },
    { "des", &cvar_STRING, 4 * PLEN, 128, CF_PTR },
    { "p_out", &cvar_person, 5 * PLEN, 0, CF_PTR },
};
CVar cvar_person_do = {"person_do", CV_STRUCT,  ARRAY_SIZE(fields_person_do) * PLEN, ARRAY_SIZE(fields_person_do), fields_person_do,};

int32_t irpc_clt__person_do(person * p, uint32_t action, char * des, person * p_out)
{
    int32_t ret;
    void * pvs[IRPC_PARAM_MAX] = { "person_do", &ret, p, &action, des, p_out,  };
    irpc_clt_call_dmem(&cvar_person_do, pvs);
    return ret;
}

void irpc_srv__person_do(void * pvs[])
{
    *(int32_t *)pvs[1] =  person_do((person *)pvs[2], *(uint32_t *)pvs[3], (char *)pvs[4], (person *)pvs[5]);
}
irpc_func irpc_func__person_do = { "person_do", irpc_srv__person_do, &cvar_person_do };

person * p = NULL;
person * p2 = NULL;

void irpc_ut_pre()
{
    p = person_create();
    p2 = person_create();

    irpc_server_func_add(&irpc_func__person_do);
}

void irpc_ut()
{

    int32_t ret = 0;


    ret = irpc_clt__person_do(p, 100, "good man", p2);
    EX_INFO(ret == 200 && p2->age == 47, "[RPC] person_do");
    //printf("ret : %d\n", ret);
    //CVar_show(&cvar_person, p2);
    ilog_info("-------------------");
}



void irpc_ut2()
{
    int32_t ret = 0;
    char des[64] = "good man";

    int i = 0;
    uint64_t start_us = iutil_time_us();
    for (i = 0; i < 100000; i++) {
        ret = irpc_clt__person_do(p, 100, des, p2);
    }
    uint64_t end_us = iutil_time_us();

    ilog_mask("[Time]%d %f\n", end_us - start_us, 100000/((end_us - start_us)/1000000.0));
    ilog_info("-------------------");
}
