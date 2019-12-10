#ifndef IHASH_H
#define IHASH_H


#include <stdint.h>
#include <stdbool.h>

typedef struct ihash_entry_s{
    void * d;
    uint64_t count;
}ihash_entry_t;



typedef struct ihash_s {
    uint32_t key_start;
    uint32_t key_len;
    uint32_t entrys_size;
    uint64_t pro_find_count;
    uint64_t pro_find_cmp_count;

    ihash_entry_t * entrys;
    int(*entry_show)(void * d);
    int(*compare)(struct ihash_s * ha, void * d1, void * d2);
    uint32_t(*keyhash)(struct ihash_s * ha, void * d);
}ihash_t;


bool ihash_create(ihash_t * ha, uint32_t num);
bool ihash_destroy(ihash_t * ha);
bool ihash_key_set(ihash_t * ha, uint32_t key_start, uint32_t key_len);
bool ihash_add(ihash_t * ha, void * d);
void * ihash_del(ihash_t * ha, void * d);
void * ihash_find(ihash_t * ha, void * d);
void ihash_show(ihash_t * ha);

int ihash_compare_byte(ihash_t * ha, void * d1, void * d2);
uint32_t ihash_keyhash_js(ihash_t * ha, void * d);
uint32_t ihash_keyhash_djb(ihash_t * ha, void * d);


#endif