#include "ihash.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool ihash_create(ihash_t * ha, uint32_t num)
{
    if ( ha == NULL){
        return false;
    }

    ha->entrys = (ihash_entry_t *)malloc(sizeof(ihash_entry_t)* num);
    if (ha->entrys == NULL) {
        return false;
    }
    memset(ha->entrys, 0x00, sizeof(ihash_entry_t)*num);
    ha->entrys_size = num;
    ha->key_start = 0;
    ha->key_len = 0;
    ha->compare = ihash_compare_byte;
    ha->keyhash = ihash_keyhash_djb;
    ha->entry_show = NULL;
    ha->pro_find_cmp_count = 0;
    ha->pro_find_count = 0;

    return true;
}

bool ihash_destroy(ihash_t * ha)
{
    if (ha == NULL){
        return false;
    }

    if (ha->entrys != NULL){
        free(ha->entrys);
    }

    memset(ha, 0x00, sizeof(ihash_t));
    return true;
}

bool ihash_key_set(ihash_t * ha, uint32_t key_start, uint32_t key_len)
{
    if (ha == NULL){
        return false;
    }

    ha->key_start = key_start;
    ha->key_len = key_len;

    return true;
}

/**************************************************************************
* 函数名称: ihash_add
* 功能描述: 添加entry， 需先用find查找到相同的key，并删除
* 输入参数: ihash_t * ha
* 输入参数: void * d
* 输出参数: 
* 返 回 值: int
* 其它说明: 
* ------------------------------------------------------------------------
* 修改日期			版本号     修改人          修改内容
* 2016年12月22日		v1.0      何兴诗          创建
**************************************************************************/
bool ihash_add(ihash_t * ha, void * d)
{
    if (ha == NULL || d == NULL){
        return false;
    }

    uint32_t hash = ha->keyhash(ha, d) % ha->entrys_size;
    uint32_t i = hash;
    do {
        if (ha->entrys[i].d == NULL) {
            ha->entrys[i].d = d;
            ha->entrys[i].count = 0;

            return true;
        }
        i++;
        i = (i) % (ha->entrys_size);
    } while (i != hash);

    return false;
}


void * ihash_del(ihash_t * ha, void * d)
{
    if (ha == NULL || d == NULL){
        return NULL;
    }
    void * ind = NULL;
    uint32_t hash = ha->keyhash(ha, d) % ha->entrys_size;
    uint32_t i = hash;
    do {
        if (ha->entrys[i].d != NULL){
            if (ha->compare(ha, ha->entrys[i].d, d) == 0) {
                ind = ha->entrys[i].d;
                ha->entrys[i].d = NULL;
                ha->entrys[i].count = 0;
                return ind;
            }
        }
        i++;
        i = (i) % (ha->entrys_size);
    } while (i != hash);
    return NULL;
}


void * ihash_find(ihash_t * ha, void * d)
{
    if (ha == NULL || d == NULL){
        return NULL;
    }

    ha->pro_find_count++;
    uint32_t hash = ha->keyhash(ha, d)%ha->entrys_size;
    uint32_t i = hash;
    do {
        ha->pro_find_cmp_count++;
        if (ha->entrys[i].d != NULL){
            if (ha->compare(ha, ha->entrys[i].d, d) == 0) {
                (ha->entrys[i].count)++;
                return ha->entrys[i].d;
            }
        }
        i++;
        i = (i) % (ha->entrys_size);
    } while (i != hash);
    return NULL;
}

void ihash_show(ihash_t * ha)
{
    if (ha == NULL){
        return ;
    }

    uint32_t i = 0;
    printf("size=%d key_start=%d key_len=%d\n", ha->entrys_size, ha->key_start, ha->key_len);
    printf("pro: find[cmp]=%ld[%ld] %0.4lf\n", ha->pro_find_count, ha->pro_find_cmp_count, ha->pro_find_cmp_count *100 / (double)ha->pro_find_count);
    printf("%-8s %-8s %-8s %-18s | %s\n", "id", "key", "count", "ptr", "info");
    printf("-----------------------------------------------------------------------------\n");
    for (i = 0; i < ha->entrys_size; i++){
        if (ha->entrys[i].d != NULL){
            printf("%-8d %-8d %-8ld 0x%016lx | ", i, ha->keyhash(ha, ha->entrys[i].d) % ha->entrys_size, ha->entrys[i].count,(uint64_t)ha->entrys[i].d);
            if (ha->entry_show != NULL){
                ha->entry_show(ha->entrys[i].d);
            }
            printf("\n");
        }
    }
}

int ihash_compare_byte(ihash_t * ha, void * d1, void * d2)
{
    if (ha == NULL || d1 == NULL || d2 == NULL){
        return 0;
    }
    return memcmp((char *)d1 + ha->key_start, (char *)d2 + ha->key_start, ha->key_len);
}

uint32_t ihash_keyhash_js(ihash_t * ha, void * d)
{
    if (ha == NULL || d == NULL){
        return 0;
    }
    uint8_t * key = (uint8_t *)d + ha->key_start;
    uint32_t i = 0;
    uint32_t hash = 1315423911;
    for (i = 0; i < ha->key_len; i++){
        hash ^= ((hash << 5) + key[i] + (hash >> 2));
    }
    return hash;
}

uint32_t ihash_keyhash_djb(ihash_t * ha, void * d)
{
    if (ha == NULL || d == NULL){
        return 0;
    }
    uint8_t * key = (uint8_t *)d + ha->key_start;
    uint32_t i = 0;
    uint32_t hash = 5381;
    
    for (i = 0; i < ha->key_len; i++){
        //hash = ((hash << 5) + hash) + key[i];
        hash += key[i];
    }
    return hash;

}







/* -------------------------[      单元测试        ]------------------------- */

typedef struct student_s {
    char info2[64];
    char name[64];
    uint32_t id;
    char info[128];
}student_t;

int ihash_student_show(void * d)
{
    if (d == NULL){
        printf("NULL");
        return 0;
    }

    student_t * s = d;
    printf("info2=%s name=%s id=%d info=%s", s->info2, s->name, s->id, s->info);
    return 0;
}

void ihash_ut()
{
    student_t s1 = { "aaa1", "a1", 100, "aa1xx" };
    student_t s2 = { "aaa2", "a2", 101, "aa2xx" };
    student_t s3 = { "aaa3", "a3", 102, "aa3xx" };
    student_t s4 = { "aaa4", "a4", 103, "aa4xx" };
    student_t * s = NULL;

    uint32_t key_start = offsetof(student_t, name);
    uint32_t key_len = offsetof(student_t, id) + sizeof(uint32_t)-offsetof(student_t, name);

    ihash_t ha;
    ihash_create(&ha, 512);
    ihash_key_set(&ha, key_start, key_len);
    ha.entry_show = ihash_student_show;

    ihash_show(&ha);

    ihash_add(&ha, &s1);
    ihash_add(&ha, &s2);
    ihash_add(&ha, &s3);
    ihash_add(&ha, &s4);

    ihash_find(&ha, &s1);
    ihash_find(&ha, &s1);
    ihash_find(&ha, &s1);
    ihash_find(&ha, &s1);
    ihash_find(&ha, &s2);
    ihash_find(&ha, &s2);


    ihash_show(&ha);



    ihash_del(&ha, &s1);
    ihash_del(&ha, &s2);
    ihash_show(&ha);

    s = ihash_find(&ha, &s3);
    ihash_student_show(s);
    printf("\n");

    s = ihash_find(&ha, &s1);
    ihash_student_show(s);
    printf("\n");
}

